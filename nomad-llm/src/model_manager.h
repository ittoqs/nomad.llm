#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QElapsedTimer>

class ModelManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool downloading READ isDownloading NOTIFY downloadingChanged)
    Q_PROPERTY(double downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString downloadingModel READ downloadingModel NOTIFY downloadingChanged)
    Q_PROPERTY(double downloadSpeedMBps READ downloadSpeedMBps NOTIFY downloadProgressChanged)
    Q_PROPERTY(bool quantizing READ isQuantizing NOTIFY quantizingChanged)
    Q_PROPERTY(double quantizeProgress READ quantizeProgress NOTIFY quantizeProgressChanged)
    Q_PROPERTY(QString quantizingModel READ quantizingModel NOTIFY quantizingChanged)

public:
    explicit ModelManager(const QString &modelsDir, QObject *parent = nullptr);
    ~ModelManager();

    // Model catalog
    Q_INVOKABLE QVariantList getModelCatalog(double ramGB) const;
    Q_INVOKABLE QVariantList getDownloadedModels() const;
    Q_INVOKABLE bool isModelDownloaded(const QString &filename) const;
    Q_INVOKABLE qint64 getModelFileSize(const QString &filename) const;
    Q_INVOKABLE QString getModelPath(const QString &filename) const;

    // Download management
    Q_INVOKABLE void downloadModel(const QString &repoId, const QString &filename,
                                    const QString &mmprojFilename = "");
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void deleteModel(const QString &filename);

    // Quantization management
    Q_INVOKABLE void quantizeModel(const QString &filename, const QString &format);
    Q_INVOKABLE void cancelQuantize();

    // State
    bool isDownloading() const;
    double downloadProgress() const;
    QString downloadingModel() const;
    double downloadSpeedMBps() const;

    bool isQuantizing() const;
    double quantizeProgress() const;
    QString quantizingModel() const;

signals:
    void downloadingChanged();
    void downloadProgressChanged();
    void downloadStarted(const QString &filename);
    void downloadFinished(const QString &filename, const QString &path);
    void downloadError(const QString &filename, const QString &error);
    void modelDeleted(const QString &filename);

    void quantizingChanged();
    void quantizeProgressChanged();
    void quantizeStarted(const QString &filename);
    void quantizeFinished(const QString &filename, const QString &newFilename);
    void quantizeError(const QString &filename, const QString &error);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError code);

private:
    QString buildDownloadUrl(const QString &repoId, const QString &filename) const;
    void startSingleDownload(const QString &repoId, const QString &filename);

    void doQuantize(const QString &filename, const QString &format);

    QString m_modelsDir;
    QNetworkAccessManager *m_networkManager;

    // Download state
    QNetworkReply *m_currentDownload = nullptr;
    QFile *m_downloadFile = nullptr;
    QString m_currentFilename;
    QString m_pendingMmproj;
    QString m_pendingRepoId;
    double m_downloadProgressValue = 0.0;
    double m_downloadSpeed = 0.0;
    QElapsedTimer m_speedTimer;
    qint64 m_lastBytesReceived = 0;

    // Quantize state
    bool m_isQuantizing = false;
    double m_quantizeProgressValue = 0.0;
    QString m_quantizingFilename;
    std::atomic<bool> m_cancelQuantize{false};
};

#endif // MODEL_MANAGER_H
