#include <QTest>
#include <QTemporaryDir>
#include "memory_manager.h"
#include "database_manager.h"
#include "inference_engine.h"

class TestMemoryManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        tempDir = new QTemporaryDir();
        dbPath = tempDir->path() + "/test_memory.db";
        db = new DatabaseManager(dbPath);
        engine = new InferenceEngine();
        
        sessionId = db->createSession("Test Memory Session");
    }

    void cleanupTestCase() {
        delete engine;
        delete db;
        delete tempDir;
    }

    void testInitialization() {
        MemoryManager memory(db, engine);
        QVERIFY(memory.parent() == nullptr); // Just a sanity check
        
        // Currently, MemoryManager usually intercepts messages or manages context.
        // As a basic integration test, we just verify it instantiates without crashing
        // when provided with a valid db and engine.
    }

private:
    QTemporaryDir* tempDir = nullptr;
    QString dbPath;
    DatabaseManager* db = nullptr;
    InferenceEngine* engine = nullptr;
    int sessionId = -1;
};

QTEST_MAIN(TestMemoryManager)
#include "tst_memory_manager.moc"
