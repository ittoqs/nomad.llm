import QtQuick
import QtTest

TestCase {
    name: "MainUITests"
    
    function test_injected_components_exist() {
        // Verify the context properties injected from C++ are available
        verify(HardwareDetector !== undefined, "HardwareDetector should be injected");
        verify(Settings !== undefined, "Settings should be injected");
        verify(Database !== undefined, "Database should be injected");
        verify(ModelManager !== undefined, "ModelManager should be injected");
        verify(AppVersion === "1.0.0-test", "AppVersion should match test injected value");
    }
}
