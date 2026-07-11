#include <QTest>
#include <QSignalSpy>
#include "hardware_detector.h"

class TestHardwareDetector : public QObject
{
    Q_OBJECT

private slots:
    void testDetection() {
        HardwareDetector detector;
        
        // Ensure values are zero/empty before detection if it doesn't auto-detect in ctor
        // Actually the ctor might run detect(). Let's force a detect anyway.
        detector.detect();
        
        // Total RAM should be > 0 on any system running this test
        QVERIFY(detector.totalRamBytes() > 0);
        
        // Available RAM should be > 0 and <= Total RAM
        QVERIFY(detector.availableRamBytes() >= 0);
        QVERIFY(detector.availableRamBytes() <= detector.totalRamBytes());
        
        // CPU Cores > 0
        QVERIFY(detector.cpuCores() > 0);
        
        // Disk free space should be > 0
        QVERIFY(detector.diskFreeBytes() > 0);
    }
    
    void testSummaryMap() {
        HardwareDetector detector;
        detector.detect();
        
        QVariantMap summary = detector.getHardwareSummary();
        QVERIFY(summary.contains("totalRamBytes"));
        QVERIFY(summary.contains("cpuCores"));
        QVERIFY(summary.contains("recommendedModel"));
    }
    
    void testMonitoringSignals() {
        HardwareDetector detector;
        QSignalSpy spy(&detector, &HardwareDetector::hardwareDetected);
        
        detector.startMonitoring(100);
        QVERIFY(spy.wait(500));
        QVERIFY(spy.count() > 0);
        
        detector.stopMonitoring();
    }
};

QTEST_MAIN(TestHardwareDetector)
#include "tst_hardware_detector.moc"
