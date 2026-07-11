pragma Singleton

import QtQuick 2.15

QtObject {
    id: theme
    property color bgMain: "#f8fafc"
    property color bgPanel: "#ffffff"
    property color bgPanelElevated: "#f1f5f9"
    property color bgItem: "#e2e8f0"
    property color bgItemHover: "#cbd5e1"
    property color bgActive: "#bfdbfe"

    property color border: "#cbd5e1"
    property color borderFocus: "#2563eb"

    property color textMain: "#0f172a"
    property color textSecondary: "#475569"
    property color textMuted: "#94a3b8"
    property color textAccent: "#3b82f6"

    property color primary: "#2563eb"
    property color primaryHover: "#1d4ed8"

    property color success: "#059669"
    property color successBg: "#d1fae5"
    property color warning: "#d97706"
    property color warningBg: "#fef3c7"
    property color danger: "#dc2626"
    property color dangerBg: "#fee2e2"
    property color dangerHover: "#b91c1c"
}
