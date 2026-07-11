#include "memory_manager.h"
#include <QVariantList>
#include <QVariantMap>
#include <QDebug>

MemoryManager::MemoryManager(DatabaseManager *db, InferenceEngine *engine, QObject *parent)
    : QObject(parent), m_db(db), m_engine(engine)
{
    // We trigger a check whenever a standard generation finishes
    connect(m_engine, &InferenceEngine::generationFinished, this, &MemoryManager::onGenerationFinished);
    
    // We also need to listen for our own background generation results
    connect(m_engine, &InferenceEngine::backgroundGenerationFinished, this, &MemoryManager::onBackgroundGenerationFinished);
    connect(m_engine, &InferenceEngine::backgroundGenerationError, this, &MemoryManager::onBackgroundGenerationError);
}

void MemoryManager::onGenerationFinished(const QString &/*fullResponse*/)
{
    if (m_currentSessionId == 0) return;
    if (m_isSummarizing) return;

    // Check if context is getting full (e.g., > 80% used)
    int used = m_engine->contextUsed();
    int total = m_engine->contextTotal();
    
    // Safety check: ensure total > 0 to avoid division by zero
    if (total > 0 && used > (total * 0.8)) {
        triggerSummarization();
    }
}

void MemoryManager::triggerSummarization()
{
    if (m_isSummarizing || m_currentSessionId == 0) return;

    // 1. Fetch unsummarized messages
    int lastSummarizedId = m_db->getSessionLastSummarizedId(m_currentSessionId);
    QString currentSummary = m_db->getSessionSummary(m_currentSessionId);
    
    // Get all messages for this session
    QVariantList allMessages = m_db->getMessages(m_currentSessionId, 1000); 
    
    QVariantList messagesToSummarize;
    int newLastMsgId = 0;
    
    // We want to summarize older messages, leaving the most recent (e.g. last 5) intact
    int keepCount = 5;
    
    if (allMessages.size() <= keepCount) {
        return; // Not enough messages to bother summarizing
    }

    QString conversationText;
    if (!currentSummary.isEmpty()) {
        conversationText += "Previous Summary: " + currentSummary + "\n\n";
    }

    int endIndex = allMessages.size() - keepCount;
    for (int i = 0; i < endIndex; ++i) {
        QVariantMap msg = allMessages[i].toMap();
        int msgId = msg["id"].toInt();
        if (msgId > lastSummarizedId) {
            QString sender = msg["sender"].toString();
            QString text = msg["text"].toString();
            conversationText += sender + ": " + text + "\n";
            newLastMsgId = msgId;
        }
    }

    if (conversationText.isEmpty() || newLastMsgId == 0) {
        return; // Nothing new to summarize
    }

    m_isSummarizing = true;
    m_lastMsgIdSummarizing = newLastMsgId;

    QVariantList promptMessages;
    QString sysPrompt = "You are a memory summarization assistant. Your task is to read the following conversation and provide a concise, unified summary of the key points, facts, and context established so far. Only output the summary, nothing else. Do not use conversational filler.";
    
    promptMessages.append(QVariantMap{{"role", "system"}, {"content", sysPrompt}});
    promptMessages.append(QVariantMap{{"role", "user"}, {"content", conversationText}});

    qDebug() << "MemoryManager: Triggering background summarization for session" << m_currentSessionId;
    m_engine->generateBackground(promptMessages, 1024, 0.3, 0.9);
}

void MemoryManager::onBackgroundGenerationFinished(const QString &response)
{
    if (!m_isSummarizing) return;
    
    qDebug() << "MemoryManager: Summarization finished. Updating DB.";
    m_db->updateSessionSummary(m_currentSessionId, response, m_lastMsgIdSummarizing);
    
    m_isSummarizing = false;
    m_lastMsgIdSummarizing = 0;
}

void MemoryManager::onBackgroundGenerationError(const QString &error)
{
    qDebug() << "MemoryManager: Summarization error:" << error;
    m_isSummarizing = false;
    m_lastMsgIdSummarizing = 0;
}
