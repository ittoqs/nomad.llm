#ifndef SD_ENGINE_H
#define SD_ENGINE_H

#include <QObject>
#include <QProcess>
#include <QString>

class SDEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY generatingChanged)

public:
    explicit SDEngine(QObject *parent = nullptr);
    ~SDEngine();

    Q_INVOKABLE void generateImage(const QString &prompt, const QString &modelPath, const QString &outputPath);
    Q_INVOKABLE void stopGeneration();
    bool isGenerating() const;

signals:
    void generatingChanged();
    void imageGenerated(const QString &imagePath);
    void generationError(const QString &error);
    void generationProgress(int step, int totalSteps);

private slots:
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    bool m_isGenerating;
    QString m_currentOutputPath;
};

#endif // SD_ENGINE_H
