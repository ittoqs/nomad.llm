#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDir>
#include <QFile>

#include "hardware_detector.h"
#include "settings_manager.h"
#include "database_manager.h"
#include "model_manager.h"
#include "inference_engine.h"
#include "document_processor.h"

class Setup : public QObject
{
    Q_OBJECT
public:
    Setup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        // Cleanup old db if exists
        QFile::remove("/tmp/test_ui.db");
        
        hwDetector = new HardwareDetector(this);
        settingsManager = new SettingsManager(this);
        dbManager = new DatabaseManager("/tmp/test_ui.db", this);
        modelManager = new ModelManager("/tmp/test_models", this);
        inferenceEngine = new InferenceEngine(); // No parent due to thread issues usually
        docProcessor = new DocumentProcessor(dbManager);
        
        engine->rootContext()->setContextProperty("HardwareDetector", hwDetector);
        engine->rootContext()->setContextProperty("Settings", settingsManager);
        engine->rootContext()->setContextProperty("Database", dbManager);
        engine->rootContext()->setContextProperty("ModelManager", modelManager);
        engine->rootContext()->setContextProperty("InferenceEngine", inferenceEngine);
        engine->rootContext()->setContextProperty("DocProcessor", docProcessor);
        engine->rootContext()->setContextProperty("AppVersion", "1.0.0-test");
    }
    
    void cleanupTestCase() {
        delete docProcessor;
        delete inferenceEngine;
        QFile::remove("/tmp/test_ui.db");
    }

private:
    HardwareDetector* hwDetector;
    SettingsManager* settingsManager;
    DatabaseManager* dbManager;
    ModelManager* modelManager;
    InferenceEngine* inferenceEngine;
    DocumentProcessor* docProcessor;
};

QUICK_TEST_MAIN_WITH_SETUP(UI_Tests, Setup)
#include "tst_ui_runner.moc"
