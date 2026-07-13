#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileInfo>
#include <QSqlDriver>
#include <QVariant>

// SQLite and sqlite-vec
#include "sqlite3.h"
extern "C" {
#ifndef SQLITE_CORE
#define SQLITE_CORE
#endif
#include "sqlite-vec.h"
}

DatabaseManager::DatabaseManager(const QString &dbPath, QObject *parent)
    : QObject(parent), m_dbPath(dbPath)
{
    initDatabase();
}

DatabaseManager::~DatabaseManager()
{
    // Close all connections
    QStringList connections = QSqlDatabase::connectionNames();
    for (const QString &name : connections) {
        if (name.startsWith("nomad_")) {
            QSqlDatabase::removeDatabase(name);
        }
    }
}

QSqlDatabase DatabaseManager::getConnection()
{
    QMutexLocker locker(&m_mutex);
    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    QString connName = "nomad_" + threadId;

    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase db = QSqlDatabase::database(connName);
        if (db.isOpen()) return db;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(m_dbPath);

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        emit errorOccurred("Database error: " + db.lastError().text());
        return db;
    }

    // Initialize sqlite-vec for this connection
    QVariant v = db.driver()->handle();
    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*") == 0) {
        sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
        if (handle) {
            char *errMsg = nullptr;
            sqlite3_vec_init(handle, &errMsg, nullptr);
            if (errMsg) {
                qWarning() << "Failed to initialize sqlite-vec:" << errMsg;
                sqlite3_free(errMsg);
            }
        }
    }

    return db;
}

void DatabaseManager::enableWAL()
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.exec("PRAGMA journal_mode=WAL");
    q.exec("PRAGMA synchronous=NORMAL");
    q.exec("PRAGMA cache_size=-8000"); // 8MB cache
    q.exec("PRAGMA temp_store=MEMORY");
    q.exec("PRAGMA mmap_size=268435456"); // 256MB mmap
}

void DatabaseManager::initDatabase()
{
    enableWAL();
    createTables();
}

void DatabaseManager::createTables()
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);

    // Sessions table with metadata
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            model_name TEXT DEFAULT '',
            system_prompt TEXT DEFAULT '',
            personality TEXT DEFAULT 'general',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");

    // Messages table
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_id INTEGER NOT NULL,
            sender TEXT NOT NULL,
            text TEXT NOT NULL,
            image_base64 TEXT DEFAULT '',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (session_id) REFERENCES sessions(id) ON DELETE CASCADE
        )
    )");

    // Enable foreign keys
    q.exec("PRAGMA foreign_keys = ON");

    // Add new columns for dynamic context (safe to fail if already exist)
    q.exec("ALTER TABLE sessions ADD COLUMN summary TEXT DEFAULT ''");
    q.exec("ALTER TABLE sessions ADD COLUMN last_summarized_msg_id INTEGER DEFAULT 0");

    // Standard table for document text chunks
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS document_chunks (
            chunk_id INTEGER PRIMARY KEY AUTOINCREMENT,
            filename TEXT NOT NULL,
            content TEXT NOT NULL,
            chunk_index INTEGER
        )
    )");

    // Vector virtual table for embeddings
    q.exec(R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS vec_documents USING vec0(
            chunk_id INTEGER PRIMARY KEY,
            embedding float[1024]
        )
    )");

    // Settings key-value store
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS app_settings (
            key TEXT PRIMARY KEY,
            value TEXT
        )
    )");

    // Indexes for performance
    q.exec("CREATE INDEX IF NOT EXISTS idx_messages_session ON messages(session_id)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_sessions_updated ON sessions(updated_at DESC)");

    if (q.lastError().isValid()) {
        qWarning() << "Table creation error:" << q.lastError().text();
    }
}

// ============== Sessions ==============

int DatabaseManager::createSession(const QString &title, const QString &modelName,
                                    const QString &systemPrompt)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("INSERT INTO sessions (title, model_name, system_prompt) VALUES (?, ?, ?)");
    q.addBindValue(title);
    q.addBindValue(modelName);
    q.addBindValue(systemPrompt);

    if (q.exec()) {
        int id = q.lastInsertId().toInt();
        emit sessionCreated(id, title);
        return id;
    }
    qWarning() << "Create session failed:" << q.lastError().text();
    return -1;
}

QVariantList DatabaseManager::getSessions()
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.exec(R"(
        SELECT s.id, s.title, s.model_name, s.personality, s.updated_at,
               (SELECT COUNT(*) FROM messages WHERE session_id = s.id) as msg_count
        FROM sessions s
        ORDER BY s.updated_at DESC
    )");

    QVariantList sessions;
    while (q.next()) {
        QVariantMap session;
        session["id"] = q.value(0).toInt();
        session["title"] = q.value(1).toString();
        session["model_name"] = q.value(2).toString();
        session["personality"] = q.value(3).toString();
        session["updated_at"] = q.value(4).toString();
        session["message_count"] = q.value(5).toInt();
        sessions.append(session);
    }
    return sessions;
}

QVariantMap DatabaseManager::getSession(int sessionId)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("SELECT id, title, model_name, system_prompt, personality, created_at FROM sessions WHERE id = ?");
    q.addBindValue(sessionId);

    QVariantMap session;
    if (q.exec() && q.next()) {
        session["id"] = q.value(0).toInt();
        session["title"] = q.value(1).toString();
        session["model_name"] = q.value(2).toString();
        session["system_prompt"] = q.value(3).toString();
        session["personality"] = q.value(4).toString();
        session["created_at"] = q.value(5).toString();
    }
    return session;
}

void DatabaseManager::deleteSession(int sessionId)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);

    // Delete messages first (cascade might not work with all SQLite builds)
    q.prepare("DELETE FROM messages WHERE session_id = ?");
    q.addBindValue(sessionId);
    q.exec();

    q.prepare("DELETE FROM sessions WHERE id = ?");
    q.addBindValue(sessionId);
    if (q.exec()) {
        emit sessionDeleted(sessionId);
    }
}

void DatabaseManager::renameSession(int sessionId, const QString &newTitle)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("UPDATE sessions SET title = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    q.addBindValue(newTitle);
    q.addBindValue(sessionId);
    if (q.exec()) {
        emit sessionRenamed(sessionId, newTitle);
    }
}

void DatabaseManager::updateSessionModel(int sessionId, const QString &modelName)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("UPDATE sessions SET model_name = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    q.addBindValue(modelName);
    q.addBindValue(sessionId);
    q.exec();
}

void DatabaseManager::updateSessionPrompt(int sessionId, const QString &systemPrompt)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("UPDATE sessions SET system_prompt = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    q.addBindValue(systemPrompt);
    q.addBindValue(sessionId);
    q.exec();
}

void DatabaseManager::updateSessionPersonality(int sessionId, const QString &personality)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("UPDATE sessions SET personality = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    q.addBindValue(personality);
    q.addBindValue(sessionId);
    q.exec();
}

void DatabaseManager::updateSessionSummary(int sessionId, const QString &summary, int lastMsgId)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("UPDATE sessions SET summary = ?, last_summarized_msg_id = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    q.addBindValue(summary);
    q.addBindValue(lastMsgId);
    q.addBindValue(sessionId);
    q.exec();
}

QString DatabaseManager::getSessionSummary(int sessionId)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("SELECT summary FROM sessions WHERE id = ?");
    q.addBindValue(sessionId);
    if (q.exec() && q.next()) {
        return q.value(0).toString();
    }
    return QString();
}

int DatabaseManager::getSessionLastSummarizedId(int sessionId)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("SELECT last_summarized_msg_id FROM sessions WHERE id = ?");
    q.addBindValue(sessionId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return 0;
}

// ============== Messages ==============

void DatabaseManager::addMessage(int sessionId, const QString &sender, const QString &text,
                                  const QString &imageBase64)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("INSERT INTO messages (session_id, sender, text, image_base64) VALUES (?, ?, ?, ?)");
    q.addBindValue(sessionId);
    q.addBindValue(sender);
    q.addBindValue(text);
    q.addBindValue(imageBase64);

    if (q.exec()) {
        int msgId = q.lastInsertId().toInt();
        // Update session timestamp
        QSqlQuery u(db);
        u.prepare("UPDATE sessions SET updated_at = CURRENT_TIMESTAMP WHERE id = ?");
        u.addBindValue(sessionId);
        u.exec();
        emit messageAdded(sessionId, msgId);
    }
}

QVariantList DatabaseManager::getMessages(int sessionId, int limit)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare(R"(
        SELECT id, sender, text, image_base64, created_at
        FROM messages WHERE session_id = ?
        ORDER BY id ASC LIMIT ?
    )");
    q.addBindValue(sessionId);
    q.addBindValue(limit);

    QVariantList messages;
    if (q.exec()) {
        while (q.next()) {
            QVariantMap msg;
            msg["id"] = q.value(0).toInt();
            msg["sender"] = q.value(1).toString();
            msg["text"] = q.value(2).toString();
            msg["image_base64"] = q.value(3).toString();
            msg["created_at"] = q.value(4).toString();
            messages.append(msg);
        }
    }
    return messages;
}

QVariantList DatabaseManager::getRecentMessages(int sessionId, int limit)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    // Get the N most recent, but return them in chronological order
    q.prepare(R"(
        SELECT id, sender, text, image_base64, created_at FROM (
            SELECT id, sender, text, image_base64, created_at
            FROM messages WHERE session_id = ?
            ORDER BY id DESC LIMIT ?
        ) sub ORDER BY id ASC
    )");
    q.addBindValue(sessionId);
    q.addBindValue(limit);

    QVariantList messages;
    if (q.exec()) {
        while (q.next()) {
            QVariantMap msg;
            msg["id"] = q.value(0).toInt();
            msg["sender"] = q.value(1).toString();
            msg["text"] = q.value(2).toString();
            msg["image_base64"] = q.value(3).toString();
            msg["created_at"] = q.value(4).toString();
            messages.append(msg);
        }
    }
    return messages;
}

void DatabaseManager::deleteMessage(int messageId)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("DELETE FROM messages WHERE id = ?");
    q.addBindValue(messageId);
    q.exec();
}

int DatabaseManager::getMessageCount(int sessionId)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM messages WHERE session_id = ?");
    q.addBindValue(sessionId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return 0;
}

QVariantList DatabaseManager::searchMessages(const QString &query, int limit)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare(R"(
        SELECT m.id, m.session_id, m.sender, m.text, s.title
        FROM messages m
        JOIN sessions s ON m.session_id = s.id
        WHERE m.text LIKE ?
        ORDER BY m.created_at DESC LIMIT ?
    )");
    q.addBindValue("%" + query + "%");
    q.addBindValue(limit);

    QVariantList results;
    if (q.exec()) {
        while (q.next()) {
            QVariantMap msg;
            msg["id"] = q.value(0).toInt();
            msg["session_id"] = q.value(1).toInt();
            msg["sender"] = q.value(2).toString();
            msg["text"] = q.value(3).toString();
            msg["session_title"] = q.value(4).toString();
            results.append(msg);
        }
    }
    return results;
}

// ============== Documents (Vector RAG) ==============

void DatabaseManager::indexDocument(const QString &filename, const QString &content, const QVariantList &embedding)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);
    q.prepare("SELECT MAX(chunk_index) FROM document_chunks WHERE filename = ?");
    q.addBindValue(filename);
    int chunkIndex = 0;
    if (q.exec() && q.next() && !q.value(0).isNull()) {
        chunkIndex = q.value(0).toInt() + 1;
    }

    QSqlQuery ins(db);
    ins.prepare("INSERT INTO document_chunks (filename, content, chunk_index) VALUES (?, ?, ?)");
    ins.addBindValue(filename);
    ins.addBindValue(content);
    ins.addBindValue(chunkIndex);
    if (!ins.exec()) return;
    
    qint64 chunkId = ins.lastInsertId().toLongLong();

    QByteArray embeddingData;
    embeddingData.resize(embedding.size() * sizeof(float));
    float *ptr = reinterpret_cast<float*>(embeddingData.data());
    for (int i = 0; i < embedding.size(); i++) {
        ptr[i] = static_cast<float>(embedding[i].toDouble());
    }

    QSqlQuery vec(db);
    vec.prepare("INSERT INTO vec_documents (chunk_id, embedding) VALUES (?, ?)");
    vec.addBindValue(chunkId);
    vec.addBindValue(embeddingData);
    vec.exec();

    emit documentIndexed(filename, chunkIndex + 1);
}

QVariantList DatabaseManager::searchDocuments(const QVariantList &queryEmbedding, int limit)
{
    QSqlDatabase db = getConnection();
    
    QByteArray queryData;
    queryData.resize(queryEmbedding.size() * sizeof(float));
    float *ptr = reinterpret_cast<float*>(queryData.data());
    for (int i = 0; i < queryEmbedding.size(); i++) {
        ptr[i] = static_cast<float>(queryEmbedding[i].toDouble());
    }

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT dc.filename, dc.content, vec_distance_cosine(v.embedding, ?) as distance
        FROM vec_documents v
        JOIN document_chunks dc ON v.chunk_id = dc.chunk_id
        WHERE v.embedding MATCH ? AND k = ?
        ORDER BY distance LIMIT ?
    )");
    
    q.addBindValue(queryData);
    q.addBindValue(queryData);
    q.addBindValue(limit);
    q.addBindValue(limit);

    QVariantList results;
    if (q.exec()) {
        while (q.next()) {
            QVariantMap doc;
            doc["filename"] = q.value(0).toString();
            doc["content"] = q.value(1).toString();
            doc["distance"] = q.value(2).toDouble();
            results.append(doc);
        }
    }
    return results;
}

void DatabaseManager::deleteDocument(const QString &filename)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    
    // Get chunk IDs
    q.prepare("SELECT chunk_id FROM document_chunks WHERE filename = ?");
    q.addBindValue(filename);
    QVariantList chunkIds;
    if (q.exec()) {
        while (q.next()) {
            chunkIds.append(q.value(0));
        }
    }
    
    // Delete from vec_documents
    for (const QVariant& id : chunkIds) {
        QSqlQuery dq(db);
        dq.prepare("DELETE FROM vec_documents WHERE chunk_id = ?");
        dq.addBindValue(id);
        dq.exec();
    }
    
    // Delete from document_chunks
    QSqlQuery dq(db);
    dq.prepare("DELETE FROM document_chunks WHERE filename = ?");
    dq.addBindValue(filename);
    dq.exec();
}

QVariantList DatabaseManager::getIndexedDocuments()
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.exec("SELECT DISTINCT filename, COUNT(*) as chunks FROM document_chunks GROUP BY filename");

    QVariantList docs;
    while (q.next()) {
        QVariantMap doc;
        doc["filename"] = q.value(0).toString();
        doc["chunk_count"] = q.value(1).toInt();
        docs.append(doc);
    }
    return docs;
}

// ============== Settings KV Store ==============

QVariant DatabaseManager::getSetting(const QString &key, const QVariant &defaultValue)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("SELECT value FROM app_settings WHERE key = ?");
    q.addBindValue(key);
    if (q.exec() && q.next()) {
        return q.value(0);
    }
    return defaultValue;
}

void DatabaseManager::setSetting(const QString &key, const QVariant &value)
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO app_settings (key, value) VALUES (?, ?)");
    q.addBindValue(key);
    q.addBindValue(value.toString());
    q.exec();
}

// ============== Data Management ==============

void DatabaseManager::clearChatHistory()
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.exec("DELETE FROM messages");
    q.exec("DELETE FROM sessions");
}

void DatabaseManager::clearAllData()
{
    QSqlDatabase db = getConnection();
    QSqlQuery q(db);
    q.exec("DELETE FROM messages");
    q.exec("DELETE FROM sessions");
    q.exec("DELETE FROM documents");
    q.exec("DELETE FROM app_settings");
}
