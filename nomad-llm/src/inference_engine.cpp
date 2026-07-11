#include "inference_engine.h"
#include <QDebug>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QMetaObject>
#include <functional>

// llama.cpp headers
#include "llama.h"
#include "common.h"
#include "LlamaBackend.h"
#include "LlamaBackend.h"

InferenceEngine::InferenceEngine(QObject *parent)
    : QObject(parent)
{
    // Initialize llama.cpp backend
    m_backend = new LlamaBackend();
}

InferenceEngine::~InferenceEngine()
{
    stopGeneration();
    unloadModel();
    delete m_backend;
}

bool InferenceEngine::isModelLoaded() const
{
    return m_backend && m_backend->isModelLoaded();
}

bool InferenceEngine::isGenerating() const
{
    return m_generating.load();
}

bool InferenceEngine::isBackgroundGenerating() const
{
    return m_backgroundGenerating.load();
}

double InferenceEngine::tokensPerSecond() const
{
    return m_tokensPerSecond;
}

int InferenceEngine::contextUsed() const
{
    return m_contextUsed;
}

int InferenceEngine::contextTotal() const
{
    return m_backend ? m_backend->contextTotal() : m_nCtx;
}

QString InferenceEngine::loadedModelName() const
{
    return m_modelName;
}

void InferenceEngine::loadModel(const QString &modelPath, int nCtx, int nGpuLayers, int nThreads, const QString &mmprojPath)
{
    Q_UNUSED(mmprojPath) // TODO: multimodal support in future

    if (isGenerating()) {
        emit modelLoadFinished(false, "Cannot load model while generating");
        return;
    }

    m_modelName = QFileInfo(modelPath).baseName();
    emit modelLoadStarted(m_modelName);

    if (m_isModelLoaded) {
        unloadModel();
    }

    m_isModelLoaded = false;
    emit modelLoadedChanged();
    emit modelLoadProgress(0.0);

    QString path = modelPath;
    QFuture<void> future = QtConcurrent::run([this, path, nCtx, nGpuLayers, nThreads]() {
        doLoadModel(path, nCtx, nGpuLayers, nThreads);
    });
}

void InferenceEngine::doLoadModel(const QString &modelPath, int nCtx, int nGpuLayers, int nThreads)
{
    QMutexLocker locker(&m_modelMutex);
    bool success = false;
    if (m_backend) {
        auto progressCb = [this](float progress) {
            QMetaObject::invokeMethod(this, [this, progress]() {
                emit modelLoadProgress(progress);
            }, Qt::QueuedConnection);
        };
        success = m_backend->loadModel(modelPath, nCtx, nGpuLayers, nThreads, progressCb);
    }

    if (!success) {
        QMetaObject::invokeMethod(this, [this]() {
            m_modelName.clear();
            emit modelLoadFinished(false, "Failed to load model file");
            emit modelLoadedChanged();
        }, Qt::QueuedConnection);
        return;
    }

    m_modelPath = modelPath;
    m_nCtx = ((LlamaBackend*)m_backend)->contextTotal();

    QMetaObject::invokeMethod(this, [this]() {
        emit modelLoadFinished(true, "");
        emit modelLoadedChanged();
    }, Qt::QueuedConnection);
}

void InferenceEngine::unloadModel()
{
    QMutexLocker locker(&m_modelMutex);

    if (m_backend) {
        m_backend->unloadModel();
    }
    m_modelPath.clear();
    m_modelName.clear();
    m_tokensPerSecond = 0.0;
    m_contextUsed = 0;

    emit modelLoadedChanged();
    emit statsUpdated();
}





void InferenceEngine::generate(const QVariantList &messages, int maxTokens,
                                double temperature, double topP)
{
    if (!isModelLoaded()) {
        emit generationError("No model loaded");
        return;
    }

    if (isGenerating()) {
        emit generationError("Generation already in progress");
        return;
    }

    if (isBackgroundGenerating()) {
        // Cancel background task to prioritize user request
        stopGeneration();
        // Wait briefly for background to stop (simple spin for demonstration, should be improved)
        while(m_backgroundGenerating.load()) {
            QThread::msleep(10);
        }
    }

    m_stopRequested.store(false);
    m_generating.store(true);
    emit generatingChanged();

    QVariantList msgsCopy = messages;
    QFuture<void> future = QtConcurrent::run([this, msgsCopy, maxTokens, temperature, topP]() {
        doGenerate(msgsCopy, maxTokens, temperature, topP, false);
    });
}

void InferenceEngine::generateBackground(const QVariantList &messages, int maxTokens,
                                         double temperature, double topP)
{
    if (!isModelLoaded()) {
        emit backgroundGenerationError("No model loaded");
        return;
    }

    if (isGenerating() || isBackgroundGenerating()) {
        emit backgroundGenerationError("Generation already in progress");
        return;
    }

    m_stopRequested.store(false);
    m_backgroundGenerating.store(true);

    QVariantList msgsCopy = messages;
    QFuture<void> future = QtConcurrent::run([this, msgsCopy, maxTokens, temperature, topP]() {
        doGenerate(msgsCopy, maxTokens, temperature, topP, true);
    });
}

void InferenceEngine::doGenerate(QVariantList messages, int maxTokens,
                                  double temperature, double topP, bool isBackground)
{
    QMutexLocker locker(&m_modelMutex);

    if (!m_backend || !m_backend->isModelLoaded()) {
        QMetaObject::invokeMethod(this, [this, isBackground]() {
            if (isBackground) {
                m_backgroundGenerating.store(false);
                emit backgroundGenerationError("Model not loaded");
            } else {
                m_generating.store(false);
                emit generationError("Model not loaded");
                emit generatingChanged();
            }
        }, Qt::QueuedConnection);
        return;
    }

    // Fully delegated generation to backend
    m_backend->generate(messages, maxTokens, temperature, topP,
        [this, isBackground](const std::string& token, int generated, int contextUsed, double tps) {
            m_tokensPerSecond = tps;
            m_contextUsed = contextUsed;
            QString qPiece = QString::fromStdString(token);
            if(m_stopRequested.load()) m_backend->stopGeneration();
            QMetaObject::invokeMethod(this, [this, qPiece, isBackground]() {
                if (!isBackground) {
                    emit tokenGenerated(qPiece);
                }
                emit statsUpdated();
            }, Qt::QueuedConnection);
        },
        [this, isBackground](const std::string& error) {
            QMetaObject::invokeMethod(this, [this, error, isBackground]() {
                if (isBackground) {
                    m_backgroundGenerating.store(false);
                    emit backgroundGenerationError(QString::fromStdString(error));
                } else {
                    m_generating.store(false);
                    emit generationError(QString::fromStdString(error));
                    emit generatingChanged();
                }
            }, Qt::QueuedConnection);
        },
        [this, isBackground](const std::string& fullResponse) {
            QString qFullResponse = QString::fromStdString(fullResponse);
            QMetaObject::invokeMethod(this, [this, qFullResponse, isBackground]() {
                if (isBackground) {
                    m_backgroundGenerating.store(false);
                    emit backgroundGenerationFinished(qFullResponse);
                } else {
                    m_generating.store(false);
                    emit generationFinished(qFullResponse);
                    emit generatingChanged();
                }
                emit statsUpdated();
            }, Qt::QueuedConnection);
        }
    );
}

void InferenceEngine::stopGeneration()
{
    m_stopRequested.store(true);
}
