#ifndef EMBEDDING_ENGINE_H
#define EMBEDDING_ENGINE_H

#include <QObject>
#include <QString>
#include <vector>
#include <atomic>

struct llama_model;
struct llama_context;

class EmbeddingEngine : public QObject {
    Q_OBJECT

public:
    explicit EmbeddingEngine(QObject *parent = nullptr);
    ~EmbeddingEngine();

    /**
     * Load an embedding model from disk.
     */
    bool loadModel(const QString &modelPath, int nGpuLayers = 0, int nThreads = 0);
    
    /**
     * Unload current model.
     */
    void unloadModel();
    
    /**
     * Returns true if an embedding model is loaded.
     */
    bool isModelLoaded() const;

    /**
     * Compute the embeddings for a given text using the loaded model.
     * Returns an empty vector on failure.
     */
    std::vector<float> computeEmbedding(const QString &text);

private:
    llama_model *m_model = nullptr;
    llama_context *m_ctx = nullptr;
};

#endif // EMBEDDING_ENGINE_H
