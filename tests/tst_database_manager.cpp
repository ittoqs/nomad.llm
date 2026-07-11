#include <QTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include "database_manager.h"

class TestDatabaseManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        tempDir = new QTemporaryDir();
        QVERIFY(tempDir->isValid());
        dbPath = tempDir->path() + "/test_nomad.db";
    }

    void cleanupTestCase() {
        if (tempDir) {
            delete tempDir;
        }
    }

    void testInitialization() {
        DatabaseManager db(dbPath);
        QVERIFY(QFile::exists(dbPath));
        
        sessionId = db.createSession("Test Session");
        QVERIFY(sessionId > 0);
    }
    
    void testMessageRetrieval() {
        DatabaseManager db(dbPath);
        
        db.addMessage(sessionId, "user", "Hello World");
        
        QVariantList messages = db.getMessages(sessionId);
        QCOMPARE(messages.size(), 1);
        
        QVariantMap msg = messages[0].toMap();
        QCOMPARE(msg["sender"].toString(), QString("user"));
        QCOMPARE(msg["text"].toString(), QString("Hello World"));
    }

    void testClearChat() {
        DatabaseManager db(dbPath);
        db.clearChatHistory();
        QVariantList messages = db.getMessages(sessionId);
        QCOMPARE(messages.size(), 0);
    }

private:
    QTemporaryDir* tempDir = nullptr;
    QString dbPath;
    int sessionId = -1;
};

QTEST_MAIN(TestDatabaseManager)
#include "tst_database_manager.moc"
