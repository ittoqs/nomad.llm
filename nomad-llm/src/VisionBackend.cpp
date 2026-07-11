#include "VisionBackend.h"
#include <llama.h>
#include <QDebug>
#include <QFile>
#include <stdexcept>

// Assuming llama.cpp has clip.h for vision examples (llava architecture compatible).
#ifdef HAS_LLAVA
#include "clip.h"
#endif

VisionBackend::VisionBackend() {}

VisionBackend::~VisionBackend() {
    unloadModel();
}

bool VisionBackend::loadModel(const QString &modelPath, int nCtx, int nGpuLayers, int nThreads,
                             std::function<void(float)> progressCallback) {
    // Normal LLM load without projector
    return false; // VisionBackend requires projector
}

bool VisionBackend::loadVisionModel(const QString &modelPath, const QString &mmprojPath, int nCtx, int nGpuLayers, int nThreads) {
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = nGpuLayers;

    m_model = llama_load_model_from_file(modelPath.toStdString().c_str(), model_params);
    if (!m_model) {
        qWarning() << "Failed to load vision model";
        return false;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = nCtx;
    ctx_params.n_threads = nThreads == 0 ? std::max(1, (int)std::thread::hardware_concurrency() / 2) : nThreads;

    m_ctx = llama_new_context_with_model(m_model, ctx_params);
    if (!m_ctx) {
        qWarning() << "Failed to create context";
        unloadModel();
        return false;
    }
    m_nCtx = nCtx;

#ifdef HAS_LLAVA
    m_clip_ctx = clip_model_load(mmprojPath.toStdString().c_str(), 0);
    if (!m_clip_ctx) {
        qWarning() << "Failed to load clip projector";
        unloadModel();
        return false;
    }
#endif

    return true;
}

void VisionBackend::unloadModel() {
    if (m_ctx) {
        llama_free(m_ctx);
        m_ctx = nullptr;
    }
    if (m_model) {
        llama_free_model(m_model);
        m_model = nullptr;
    }
#ifdef HAS_LLAVA
    if (m_clip_ctx) {
        clip_free(m_clip_ctx);
        m_clip_ctx = nullptr;
    }
#endif
}

bool VisionBackend::isModelLoaded() const {
    return m_model != nullptr && m_ctx != nullptr;
}

std::string VisionBackend::buildPrompt(const QVariantList &messages) {
    // For vision models, prompt includes image tokens
    std::string prompt = "A chat between a curious human and an artificial intelligence assistant.\n";
    for (const auto &msg : messages) {
        QVariantMap map = msg.toMap();
        QString role = map["role"].toString();
        QString content = map["content"].toString();
        
        if (role == "user") {
            prompt += "USER: " + content.toStdString() + "\n";
        } else if (role == "assistant") {
            prompt += "ASSISTANT: " + content.toStdString() + "\n";
        }
    }
    prompt += "ASSISTANT:";
    return prompt;
}

std::string VisionBackend::tokenToString(int token) {
    if (!m_model) return "";
    std::vector<char> result(8, 0);
    const int n_tokens = llama_token_to_piece(m_model, token, result.data(), result.size(), 0, true);
    if (n_tokens < 0) {
        result.resize(-n_tokens);
        int check = llama_token_to_piece(m_model, token, result.data(), result.size(), 0, true);
        Q_ASSERT(check == -n_tokens);
    } else {
        result.resize(n_tokens);
    }
    return std::string(result.data(), result.size());
}

bool VisionBackend::evalImage(const QString &imagePath) {
#ifdef HAS_LLAVA
    if (!m_clip_ctx) return false;
    // clip_image_u8 img;
    // if (!clip_image_load_from_file(imagePath.toStdString().c_str(), &img)) return false;
    // struct clip_image_f32 * img_res = clip_image_preprocess(m_clip_ctx, &img);
    // ... evaluate image ...
    return true;
#else
    return false;
#endif
}

void VisionBackend::generate(const QVariantList &messages, int maxTokens, double temperature, double topP,
                            std::function<void (const std::string &, int, int, double)> onToken,
                            std::function<void (const std::string &)> onError,
                            std::function<void (const std::string &)> onFinished) {
    m_stop = false;
    onFinished("Vision stub generation finished.");
}
