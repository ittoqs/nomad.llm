#include "sd_engine.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QRegularExpression>

SDEngine::SDEngine(QObject *parent)
    : QObject(parent), m_process(new QProcess(this)), m_isGenerating(false)
{
    connect(m_process, &QProcess::readyReadStandardOutput, this, &SDEngine::onProcessReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &SDEngine::onProcessReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SDEngine::onProcessFinished);
}

SDEngine::~SDEngine() {
    stopGeneration();
}

void SDEngine::generateImage(const QString &prompt, const QString &modelPath, const QString &outputPath) {
    if (m_isGenerating) {
        emit generationError("SDEngine is already generating an image.");
        return;
    }

    m_currentOutputPath = outputPath;
    m_isGenerating = true;
    emit generatingChanged();

    QString sdExecutable = QCoreApplication::applicationDirPath() + "/sd";
#ifdef Q_OS_WIN
    sdExecutable += ".exe";
#endif

    QStringList args;
    args << "-m" << modelPath;
    args << "-p" << prompt;
    args << "-o" << outputPath;
    args << "-v"; // verbose for progress

    qDebug() << "Starting stable-diffusion:" << sdExecutable << args;
    m_process->start(sdExecutable, args);
}

void SDEngine::stopGeneration() {
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

bool SDEngine::isGenerating() const {
    return m_isGenerating;
}

void SDEngine::onProcessReadyReadStandardOutput() {
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    // Parse progress if stable-diffusion.cpp outputs it (e.g., "Step 10/20")
    QRegularExpression re("Step\\s+(\\d+)/(\\d+)");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        emit generationProgress(match.captured(1).toInt(), match.captured(2).toInt());
    }
}

void SDEngine::onProcessReadyReadStandardError() {
    QString error = QString::fromUtf8(m_process->readAllStandardError());
    qDebug() << "[SD Error]" << error;
}

void SDEngine::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_isGenerating = false;
    emit generatingChanged();

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        if (QFile::exists(m_currentOutputPath)) {
            emit imageGenerated(m_currentOutputPath);
        } else {
            emit generationError("Image generated but output file not found.");
        }
    } else {
        emit generationError("Stable Diffusion process failed with code " + QString::number(exitCode));
    }
}
