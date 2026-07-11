#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <QObject>
#include <QString>
#include "inference_engine.h"
#include "database_manager.h"

class MemoryManager : public QObject {
    Q_OBJECT

public:
    explicit MemoryManager(DatabaseManager *db, InferenceEngine *engine, QObject *parent = nullptr);

    // QML can call this when a session is switched, or we handle it automatically.
    // For now, let's just make it public.
    Q_INVOKABLE void setCurrentSessionId(int sessionId) { m_currentSessionId = sessionId; }

private slots:
    void onGenerationFinished(const QString &fullResponse);
    void onBackgroundGenerationFinished(const QString &response);
    void onBackgroundGenerationError(const QString &error);

private:
    void triggerSummarization();

    DatabaseManager *m_db;
    InferenceEngine *m_engine;
    
    int m_currentSessionId = 0;
    int m_lastMsgIdSummarizing = 0;
    bool m_isSummarizing = false;
};

#endif // MEMORY_MANAGER_H
