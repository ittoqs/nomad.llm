#ifndef VISIONBACKEND_H
#define VISIONBACKEND_H

#include "ILlmBackend.h"
#include <atomic>
#include <vector>
#include <string>

struct llama_model;
struct llama_context;
struct clip_ctx;

class VisionBackend : public ILlmBackend {
public:
    VisionBackend();
    ~VisionBackend() override;

    bool loadModel(const QString &modelPath, int nCtx, int nGpuLayers, int nThreads = 0,
                   std::function<void(float)> progressCallback = nullptr) override;
    
    // Extended load for vision models (Llava, Qwen-VL, Moondream, etc.)
    bool loadVisionModel(const QString &modelPath, const QString &mmprojPath, int nCtx, int nGpuLayers, int nThreads = 0);
    
    void unloadModel() override;
    bool isModelLoaded() const override;

    std::string buildPrompt(const QVariantList &messages) override;
    std::string tokenToString(int token) override;

    llama_model* model() const { return m_model; }
    int contextTotal() const override { return m_nCtx; }
    void stopGeneration() override { m_stop = true; }
    
    void generate(const QVariantList &messages, int maxTokens, double temperature, double topP,
                  std::function<void(const std::string&, int, int, double)> onToken,
                  std::function<void(const std::string&)> onError,
                  std::function<void(const std::string&)> onFinished) override;

private:
    bool evalImage(const QString &imagePath);

    std::atomic<bool> m_stop{false};
    std::vector<int32_t> m_prev_tokens;
    llama_model *m_model = nullptr;
    llama_context *m_ctx = nullptr;
    clip_ctx *m_clip_ctx = nullptr;
    int m_nCtx = 4096;
};

#endif // VISIONBACKEND_H
