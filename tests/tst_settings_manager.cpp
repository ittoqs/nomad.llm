#include <QTest>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include "settings_manager.h"

class TestSettingsManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        QCoreApplication::setOrganizationName("NomadTest");
        QCoreApplication::setApplicationName("NomadLLM_Test");
    }
    
    void testDefaultValues() {
        SettingsManager settings;
        
        // Check standard defaults
        QVERIFY(settings.fontSize() > 0);
        QVERIFY(!settings.defaultSystemPrompt().isEmpty());
        QVERIFY(settings.maxContextTokens() > 0);
        QVERIFY(settings.temperature() >= 0.0);
    }
    
    void testValuePersistence() {
        SettingsManager settings;
        
        int originalSize = settings.fontSize();
        int newSize = originalSize + 2;
        
        settings.setFontSize(newSize);
        QCOMPARE(settings.fontSize(), newSize);
        
        // Re-initialize to ensure it was written to disk
        SettingsManager settingsReloaded;
        QCOMPARE(settingsReloaded.fontSize(), newSize);
        
        // Restore
        settingsReloaded.setFontSize(originalSize);
    }
    
    void testPaths() {
        SettingsManager settings;
        QVERIFY(!settings.dataDirectory().isEmpty());
        QVERIFY(!settings.modelsDirectory().isEmpty());
        QVERIFY(!settings.databasePath().isEmpty());
    }
};

QTEST_MAIN(TestSettingsManager)
#include "tst_settings_manager.moc"
