import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Dialogs
import "components"
import "onboarding"
import Nomad

Window {
    id: root
    width: 1280
    height: 860
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: "Nomad LLM"
    color: Theme.bgMain

    // ================ STATE ================
    property double ramGB: HardwareDetector.totalRamBytes / (1024.0 * 1024.0 * 1024.0)
    property double availRamGB: HardwareDetector.availableRamBytes / (1024.0 * 1024.0 * 1024.0)
    property double diskGB: HardwareDetector.diskFreeBytes / (1024.0 * 1024.0 * 1024.0)

    property string activeModelName: ""
    property string activeFilename: ""
    property bool isThinking: false
    property int currentSessionId: 0
    property string currentResponseBuffer: ""
    property string selectedImagePath: ""
    property string selectedImageBase64: ""
    property string currentSystemPrompt: Settings.defaultSystemPrompt
    property var currentCitations: []
    property string currentPersonality: "general"

    property bool sidebarVisible: root.width >= 900
    property int sidebarWidth: sidebarVisible ? 300 : 0
    
    // Convenience i18n
    function t(key) { return Settings.tr(key); }

    // ================ FONTS ================
    FontLoader { id: interFont; source: "https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700" }

    // ================ MODELS ================
    ListModel { id: chatModel }
    ListModel { id: sessionsModel }

    // ================ STARTUP ================
    Component.onCompleted: {
        if (Settings.firstRun) {
            stackView.push(onboardingComponent);
        } else {
            loadSessions();
        }
    }

    function loadSessions() {
        var sessions = Database.getSessions();
        sessionsModel.clear();
        for (var i = 0; i < sessions.length; i++) {
            sessionsModel.append(sessions[i]);
        }
        if (sessionsModel.count > 0 && currentSessionId === 0) {
            switchToSession(sessionsModel.get(0).id);
        }
    }

    function switchToSession(sessionId) {
        currentSessionId = sessionId;
        MemoryManager.setCurrentSessionId(sessionId);
        var session = Database.getSession(sessionId);
        currentSystemPrompt = session.system_prompt || Settings.defaultSystemPrompt;
        currentPersonality = session.personality || "general";

        var msgs = Database.getMessages(sessionId, 200);
        chatModel.clear();
        for (var i = 0; i < msgs.length; i++) {
            chatModel.append({
                "sender": msgs[i].sender,
                "text": msgs[i].text,
                "imageBase64": msgs[i].image_base64 || "",
                "msgId": msgs[i].id
            });
        }
    }

    function createNewSession() {
        var num = sessionsModel.count + 1;
        var title = t("session") + " " + num;
        var id = Database.createSession(title, activeModelName, currentSystemPrompt);
        if (id > 0) {
            loadSessions();
            switchToSession(id);
            chatModel.clear();
        }
    }

    function sendMessage(msg) {
        if (msg === "" || !InferenceEngine.modelLoaded || currentSessionId === 0) return;

        chatModel.append({"sender": "User", "text": msg, "imageBase64": selectedImageBase64, "msgId": -1});
        Database.addMessage(currentSessionId, "User", msg, selectedImageBase64);
        selectedImagePath = "";
        selectedImageBase64 = "";

        isThinking = true;
        currentResponseBuffer = "";

        var messages = [];
        currentCitations = [];
        if (currentSystemPrompt) {
            var sysPrompt = currentSystemPrompt;
            
            var summary = Database.getSessionSummary(currentSessionId);
            if (summary && summary !== "") {
                sysPrompt += "\n\nSession Context Summary:\n" + summary + "\n";
            }
            
            var context = DocProcessor.searchContext(msg, 2);
            if (context.length > 0) {
                sysPrompt += "\n\nRelevant context:\n";
                var citations = [];
                for (var c = 0; c < context.length; c++) {
                    sysPrompt += context[c].content + "\n";
                    if (citations.indexOf(context[c].filename) === -1) {
                        citations.push(context[c].filename);
                    }
                }
                currentCitations = citations;
            }
            messages.push({"role": "system", "content": sysPrompt});
        }

        var lastSummarizedId = Database.getSessionLastSummarizedId(currentSessionId);
        var history = Database.getRecentMessages(currentSessionId, Settings.chatHistoryLimit);
        for (var i = 0; i < history.length; i++) {
            var h = history[i];
            if (h.id <= lastSummarizedId) continue;
            if (h.sender === "User" && h.text === msg && i === history.length - 1) continue;
            messages.push({
                "role": h.sender === "User" ? "user" : "assistant",
                "content": h.text
            });
        }
        messages.push({"role": "user", "content": msg});

        InferenceEngine.generate(messages, Settings.maxResponseTokens, Settings.temperature, Settings.topP);
    }

    // ================ SIGNAL HANDLERS ================
    Connections {
        target: VoiceManager
        function onVoiceProcessed(transcription) {
            if (chatInputBar) {
                chatInputBar.setInputText(transcription);
            }
        }
    }

    Connections {
        target: InferenceEngine

        function onTokenGenerated(token) {
            isThinking = false;
            currentResponseBuffer += token;
            if (chatModel.count > 0 && chatModel.get(chatModel.count - 1).sender === "Nomad") {
                chatModel.setProperty(chatModel.count - 1, "text", currentResponseBuffer);
            } else {
                chatModel.append({"sender": "Nomad", "text": currentResponseBuffer, "imageBase64": "", "msgId": -1});
            }
        }

        function onGenerationFinished(fullResponse) {
            isThinking = false;
            var finalResponse = fullResponse;
            if (currentCitations && currentCitations.length > 0) {
                finalResponse += "\n\n**" + (typeof t === 'function' ? t("sources") || "Sources" : "Sources") + ":**\n";
                for (var i = 0; i < currentCitations.length; i++) {
                    finalResponse += "- " + currentCitations[i] + "\n";
                }
                if (chatModel.count > 0 && chatModel.get(chatModel.count - 1).sender === "Nomad") {
                    chatModel.setProperty(chatModel.count - 1, "text", finalResponse);
                }
            }

            Database.addMessage(currentSessionId, "Nomad", finalResponse, "");
            currentResponseBuffer = "";
            currentCitations = [];
        }

        function onGenerationError(error) {
            isThinking = false;
            currentResponseBuffer = "";
            chatModel.append({"sender": "Nomad", "text": "❌ " + error, "imageBase64": "", "msgId": -1});
        }

        function onModelLoadStarted(modelName) {
            chatModel.append({"sender": "Nomad", "text": "⏳ " + t("loading_model") + " " + modelName + "...", "imageBase64": "", "msgId": -1});
        }

        function onModelLoadFinished(success, error) {
            if (success) {
                chatModel.append({"sender": "Nomad", "text": "✅ " + t("model_loaded") + ": **" + activeModelName + "**", "imageBase64": "", "msgId": -1});
            } else {
                chatModel.append({"sender": "Nomad", "text": "❌ " + error, "imageBase64": "", "msgId": -1});
                activeModelName = "";
                activeFilename = "";
            }
        }
    }

    Connections {
        target: ModelManager
        function onDownloadStarted(filename) {
            chatModel.append({"sender": "Nomad", "text": "📥 " + t("downloading") + " `" + filename + "`", "imageBase64": "", "msgId": -1});
        }
        function onDownloadFinished(filename, path) {
            chatModel.append({"sender": "Nomad", "text": "✅ " + t("download_complete") + ": `" + filename + "`", "imageBase64": "", "msgId": -1});
        }
        function onDownloadError(filename, error) {
            chatModel.append({"sender": "Nomad", "text": "❌ " + t("download_failed") + ": " + error, "imageBase64": "", "msgId": -1});
        }
    }

    // ================ FILE DIALOG ================
    FileDialog {
        id: fileDialog
        title: t("upload_document")
        nameFilters: ["All Supported (*.pdf *.txt *.md *.csv *.json)", "Documents (*.pdf *.txt *.md)", "Images (*.png *.jpg *.jpeg)"]
        onAccepted: {
            var urlStr = fileDialog.fileUrl.toString();
            var isImage = urlStr.endsWith(".png") || urlStr.endsWith(".jpg") || urlStr.endsWith(".jpeg");
            if (isImage) {
                selectedImagePath = urlStr;
            } else {
                chatModel.append({"sender": "Nomad", "text": "📄 " + t("upload_document") + "...", "imageBase64": "", "msgId": -1});
                DocProcessor.processFile(urlStr);
            }
        }
    }

    Connections {
        target: DocProcessor
        function onDocumentProcessed(filename, chunkCount) {
            chatModel.append({"sender": "Nomad", "text": "✅ **" + filename + "** — " + chunkCount + " chunks indexed", "imageBase64": "", "msgId": -1});
        }
        function onProcessingError(filename, error) {
            chatModel.append({"sender": "Nomad", "text": "❌ " + filename + ": " + error, "imageBase64": "", "msgId": -1});
        }
    }

    // ================ MAIN LAYOUT ================
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainPageComponent
    }

    Component {
        id: onboardingComponent
        OnboardingWizard {
            ramGB: root.ramGB
            diskGB: root.diskGB
            activeFilename: root.activeFilename
            onWizardFinished: {
                Settings.completeFirstRun();
                stackView.pop();
                loadSessions();
            }
        }
    }

    Component {
        id: mainPageComponent
        Rectangle {
            color: Theme.bgMain

            // ---- SIDEBAR ----
            SidebarView {
                id: sidebar
                width: root.sidebarWidth
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: root.sidebarVisible

                currentSessionId: root.currentSessionId
                sessionsModel: root.sessionsModel
                ramGB: root.ramGB
                activeFilename: root.activeFilename
                activeModelName: root.activeModelName

                onOpenSettingsRequested: settingsDrawer.open()
                onNewSessionRequested: createNewSession()
                onSessionSelected: function(id) { switchToSession(id) }
                onSessionDeleted: function(id) {
                    Database.deleteSession(id);
                    if (root.currentSessionId === id) root.currentSessionId = 0;
                    loadSessions();
                }
                onModelDownloadRequested: function(repoId, filename, mmproj) {
                    ModelManager.downloadModel(repoId, filename, mmproj);
                }
                onModelLoadRequested: function(name, filename) {
                    root.activeModelName = name;
                    root.activeFilename = filename;
                    var path = ModelManager.getModelPath(filename);
                    InferenceEngine.loadModel(path, HardwareDetector.recommendedContextSize, HardwareDetector.recommendedGpuLayers, HardwareDetector.recommendedCpuThreads);
                    if (root.currentSessionId > 0) {
                        Database.updateSessionModel(root.currentSessionId, name);
                    }
                }
            }

            // ---- MAIN CHAT AREA ----
            Rectangle {
                anchors.left: sidebar.right
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                color: Theme.bgMain

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Top Bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 60
                        color: Theme.bgPanel
                        border.color: Theme.bgItem
                        border.width: 0

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            // Sidebar toggle (for small screens)
                            Rectangle {
                                width: 36; height: 36; radius: 8
                                color: sideToggle.containsMouse ? Theme.bgItemHover : "transparent"
                                visible: root.width < 900
                                Text { text: "☰"; color: Theme.textSecondary; font.pixelSize: 18; anchors.centerIn: parent }
                                MouseArea {
                                    id: sideToggle
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    hoverEnabled: true
                                    onClicked: root.sidebarVisible = !root.sidebarVisible
                                }
                            }

                            // Active model / personality
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 0
                                Text {
                                    text: root.activeModelName || t("select_model_first")
                                    color: Theme.textMain
                                    font.pixelSize: 16
                                    font.bold: true
                                }
                                Text {
                                    text: root.currentPersonality !== "general" ? ("🎭 " + root.currentPersonality) : ""
                                    color: Theme.textMuted
                                    font.pixelSize: 12
                                    visible: text !== ""
                                }
                            }

                            // Personality selector
                            Rectangle {
                                width: personIcon.width + 16
                                height: 32; radius: 8
                                color: personBtn.containsMouse ? Theme.bgItemHover : "transparent"
                                Text { id: personIcon; text: "🎭"; font.pixelSize: 16; anchors.centerIn: parent }
                                MouseArea {
                                    id: personBtn
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    hoverEnabled: true
                                    onClicked: personalityPopup.open()
                                }
                            }

                            // Stop button
                            Rectangle {
                                width: 70; height: 32; radius: 16
                                color: Theme.danger
                                visible: InferenceEngine.generating
                                Text { text: "⏹ " + t("stop"); color: "white"; font.pixelSize: 12; font.bold: true; anchors.centerIn: parent }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: InferenceEngine.stopGeneration()
                                }
                            }
                        }
                    }

                    ChatArea {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.margins: 16
                        chatModel: root.chatModel
                        isThinking: root.isThinking
                    }

                    ChatInputBar {
                        id: chatInputBar
                        Layout.fillWidth: true
                        currentSessionId: root.currentSessionId
                        isGenerating: InferenceEngine.generating
                        
                        onSendMessageRequested: function(msg) { root.sendMessage(msg) }
                        onAttachFileRequested: fileDialog.open()
                        onClearAttachmentRequested: { root.selectedImagePath = ""; root.selectedImageBase64 = ""; }
                    }

                    StatusBar {
                        ramGB: root.ramGB
                        availRamGB: root.availRamGB
                    }
                }
            }

            PersonalityPopup {
                id: personalityPopup
                x: parent.width - 340
                y: 64
                currentPersonality: root.currentPersonality
                currentSystemPrompt: root.currentSystemPrompt
                
                onPersonalitySelected: function(id, prompt) {
                    root.currentPersonality = id;
                    root.currentSystemPrompt = prompt;
                    if (root.currentSessionId > 0) {
                        Database.updateSessionPrompt(root.currentSessionId, prompt);
                        Database.updateSessionPersonality(root.currentSessionId, id);
                    }
                }
                
                onSystemPromptChanged: function(prompt) {
                    root.currentSystemPrompt = prompt;
                    if (root.currentSessionId > 0) {
                        Database.updateSessionPrompt(root.currentSessionId, prompt);
                    }
                }
            }
        }
    }

    SettingsDrawer {
        id: settingsDrawer
        width: Math.min(root.width * 0.85, 400)
        height: root.height
        
        onClearDataRequested: {
            root.sessionsModel.clear();
            root.chatModel.clear();
            root.currentSessionId = 0;
        }

        onStorageManagerRequested: {
            settingsDrawer.close();
            storageManager.open();
        }
    }

    StorageManagerDrawer {
        id: storageManager
    }
}
