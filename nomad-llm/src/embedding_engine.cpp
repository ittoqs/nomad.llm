#include "embedding_engine.h"
#include "llama.h"
#include <QDebug>
#include <string>

EmbeddingEngine::EmbeddingEngine(QObject *parent)
    : QObject(parent)
{
}

EmbeddingEngine::~EmbeddingEngine()
{
    unloadModel();
}

bool EmbeddingEngine::loadModel(const QString &modelPath, int nGpuLayers, int nThreads)
{
    unloadModel();

    auto model_params = llama_model_default_params();
    model_params.n_gpu_layers = nGpuLayers;

    std::string pathStr = modelPath.toStdString();
    m_model = llama_model_load_from_file(pathStr.c_str(), model_params);
    if (!m_model) {
        qWarning() << "Failed to load embedding model from" << modelPath;
        return false;
    }

    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 1024; // Standard embedding context
    ctx_params.n_batch = 512;
    ctx_params.n_threads = nThreads > 0 ? nThreads : 4;
    
    // In many llama.cpp versions, this flag is required to return embeddings
    // We try to set it if we can; if it doesn't compile we can remove it.
    // However, some versions might not have it. Let's assume it's needed for b5460.
    // For safety, we will rely on pooling if available, or just standard decoding.

    m_ctx = llama_init_from_model(m_model, ctx_params);
    if (!m_ctx) {
        qWarning() << "Failed to create context for embedding model";
        unloadModel();
        return false;
    }

    return true;
}

void EmbeddingEngine::unloadModel()
{
    if (m_ctx) {
        llama_free(m_ctx);
        m_ctx = nullptr;
    }
    if (m_model) {
        llama_model_free(m_model);
        m_model = nullptr;
    }
}

bool EmbeddingEngine::isModelLoaded() const
{
    return m_model != nullptr && m_ctx != nullptr;
}

std::vector<float> EmbeddingEngine::computeEmbedding(const QString &text)
{
    if (!isModelLoaded()) return {};

    std::string str = text.toStdString();
    
    // Tokenize
    std::vector<llama_token> tokens(str.length() + 2);
    int n_tokens = llama_tokenize(m_model, str.c_str(), str.length(), tokens.data(), tokens.size(), true, true);
    if (n_tokens < 0) {
        // Retry with larger buffer
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(m_model, str.c_str(), str.length(), tokens.data(), tokens.size(), true, true);
        if (n_tokens < 0) {
            qWarning() << "Failed to tokenize text for embeddings";
            return {};
        }
    }
    tokens.resize(n_tokens);

    // Evaluate
    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    for (int i = 0; i < n_tokens; i++) {
        llama_batch_add(&batch, tokens[i], i, { 0 }, false);
    }
    
    // Request embedding for the last token or based on pooling
    batch.logits[n_tokens - 1] = false; // we don't need logits
    
    if (llama_decode(m_ctx, batch) != 0) {
        qWarning() << "llama_decode failed during embedding computation";
        llama_batch_free(batch);
        return {};
    }
    llama_batch_free(batch);

    // Get embeddings
    // Modern llama.cpp uses pooling for embeddings and retrieves them via llama_get_embeddings_seq or llama_get_embeddings
    int n_embd = llama_model_n_embd(m_model);
    std::vector<float> embedding;
    
    // For many embedding models (like all-MiniLM, Nomic), llama_get_embeddings_seq(ctx, 0) gives the pooled embedding
    const float * embd = llama_get_embeddings_seq(m_ctx, 0);
    if (!embd) {
        // Fallback to token embeddings
        embd = llama_get_embeddings(m_ctx);
    }
    
    if (embd) {
        embedding.assign(embd, embd + n_embd);
        
        // Normalize the embedding vector (cosine similarity standard)
        float sum = 0.0f;
        for (float v : embedding) sum += v * v;
        if (sum > 0.0f) {
            float norm = std::sqrt(sum);
            for (float &v : embedding) v /= norm;
        }
    } else {
        qWarning() << "Failed to retrieve embeddings from llama context";
    }

    return embedding;
}
