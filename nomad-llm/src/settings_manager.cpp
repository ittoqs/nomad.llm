#include "settings_manager.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QCryptographicHash>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
{
    ensureDirectories();

    QString configPath = dataDirectory() + "/config.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);

    initTranslations();
    loadConfig();
}

void SettingsManager::ensureDirectories()
{
    QDir dir;
    dir.mkpath(dataDirectory());
    dir.mkpath(modelsDirectory());
}

QString SettingsManager::dataDirectory() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty()) {
        path = QDir::homePath() + "/.nomad-llm";
    }
    return path;
}

QString SettingsManager::modelsDirectory() const
{
    return dataDirectory() + "/models";
}

QString SettingsManager::databasePath() const
{
    return dataDirectory() + "/nomad.db";
}

// --- Language ---
QString SettingsManager::tr(const QString &key) const
{
    return m_translationsEn.value(key, key).toString();
}

// --- Font ---
int SettingsManager::fontSize() const
{
    return m_settings->value("fontSize", 14).toInt();
}

void SettingsManager::setFontSize(int size)
{
    size = qBound(10, size, 24);
    if (fontSize() != size) {
        m_settings->setValue("fontSize", size);
        emit fontSizeChanged();
    }
}

// --- System Prompt ---
QString SettingsManager::defaultSystemPrompt() const
{
    return m_settings->value("defaultSystemPrompt",
        "You are Nomad, a smart, friendly, and helpful AI assistant. ").toString();
}

void SettingsManager::setDefaultSystemPrompt(const QString &prompt)
{
    if (defaultSystemPrompt() != prompt) {
        m_settings->setValue("defaultSystemPrompt", prompt);
        emit defaultSystemPromptChanged();
    }
}

// --- First Run ---
bool SettingsManager::isFirstRun() const
{
    return m_settings->value("firstRunCompleted", false).toBool() == false;
}

void SettingsManager::completeFirstRun()
{
    m_settings->setValue("firstRunCompleted", true);
    emit firstRunChanged();
}

// --- Inference Settings ---
int SettingsManager::maxContextTokens() const
{
    return m_settings->value("maxContextTokens", 4096).toInt();
}

void SettingsManager::setMaxContextTokens(int tokens)
{
    tokens = qBound(512, tokens, 32768);
    if (maxContextTokens() != tokens) {
        m_settings->setValue("maxContextTokens", tokens);
        emit maxContextTokensChanged();
    }
}

int SettingsManager::maxResponseTokens() const
{
    return m_settings->value("maxResponseTokens", 2048).toInt();
}

void SettingsManager::setMaxResponseTokens(int tokens)
{
    tokens = qBound(64, tokens, 16384);
    if (maxResponseTokens() != tokens) {
        m_settings->setValue("maxResponseTokens", tokens);
        emit maxResponseTokensChanged();
    }
}

double SettingsManager::temperature() const
{
    return m_settings->value("temperature", 0.7).toDouble();
}

void SettingsManager::setTemperature(double temp)
{
    temp = qBound(0.0, temp, 2.0);
    if (qAbs(temperature() - temp) > 0.001) {
        m_settings->setValue("temperature", temp);
        emit temperatureChanged();
    }
}

double SettingsManager::topP() const
{
    return m_settings->value("topP", 0.9).toDouble();
}

void SettingsManager::setTopP(double p)
{
    p = qBound(0.0, p, 1.0);
    if (qAbs(topP() - p) > 0.001) {
        m_settings->setValue("topP", p);
        emit topPChanged();
    }
}

int SettingsManager::chatHistoryLimit() const
{
    return m_settings->value("chatHistoryLimit", 20).toInt();
}

void SettingsManager::setChatHistoryLimit(int limit)
{
    limit = qBound(4, limit, 100);
    if (chatHistoryLimit() != limit) {
        m_settings->setValue("chatHistoryLimit", limit);
        emit chatHistoryLimitChanged();
    }
}

// --- Personality Presets ---
QVariantList SettingsManager::getPersonalityPresets() const
{
    QVariantList presets;

    QVariantMap general;
    general["id"] = "general";
    general["name"] = "General Assistant";
    general["icon"] = "💬";
    general["prompt"] = systemPrompt("assistant", false);

    QVariantMap coder;
    coder["id"] = "coder";
    coder["name"] = "Programmer";
    coder["icon"] = "💻";
    coder["prompt"] = systemPrompt("coder", false);

    QVariantMap writer;
    writer["id"] = "writer";
    writer["name"] = "Writer";
    writer["icon"] = "✍️";
    writer["prompt"] = systemPrompt("writer", false);

    QVariantMap translator;
    translator["id"] = "translator";
    translator["name"] = "Translator";
    translator["icon"] = "🌐";
    translator["prompt"] = systemPrompt("translator", false);

    QVariantMap analyst;
    analyst["id"] = "analyst";
    analyst["name"] = "Data Analyst";
    analyst["icon"] = "📊";
    analyst["prompt"] = systemPrompt("analyst", false);

    QVariantMap creative;
    creative["id"] = "creative";
    creative["name"] = "Creative";
    creative["icon"] = "🎨";
    creative["prompt"] = systemPrompt("creative", false);

    presets << general << coder << writer << translator << analyst << creative;
    return presets;
}

// --- Translation Strings ---
void SettingsManager::initTranslations()
{
    // English
    m_translationsEn["app_name"] = "Nomad LLM";
    m_translationsEn["welcome_title"] = "Welcome to Nomad LLM";
    m_translationsEn["welcome_subtitle"] = "Your Personal AI Desktop — Fully Offline & Private";
    m_translationsEn["next"] = "Next";
    m_translationsEn["back"] = "Back";
    m_translationsEn["finish"] = "Get Started";
    m_translationsEn["skip"] = "Skip";
    m_translationsEn["new_chat"] = "New Chat";
    m_translationsEn["chat_history"] = "Chat History";
    m_translationsEn["models"] = "AI Models";
    m_translationsEn["settings"] = "Settings";
    m_translationsEn["select_model"] = "Select Model";
    m_translationsEn["download"] = "Download";
    m_translationsEn["downloading"] = "Downloading...";
    m_translationsEn["downloaded"] = "Downloaded";
    m_translationsEn["delete"] = "Delete";
    m_translationsEn["rename"] = "Rename";
    m_translationsEn["cancel"] = "Cancel";
    m_translationsEn["confirm"] = "Confirm";
    m_translationsEn["send"] = "Send";
    m_translationsEn["type_message"] = "Type a message...";
    m_translationsEn["create_session_first"] = "Create a chat session first";
    m_translationsEn["select_model_first"] = "Select an AI model first";
    m_translationsEn["thinking"] = "Nomad is thinking...";
    m_translationsEn["hardware_detected"] = "Hardware Detected";
    m_translationsEn["ram"] = "RAM";
    m_translationsEn["cpu"] = "CPU";
    m_translationsEn["gpu"] = "GPU";
    m_translationsEn["cores"] = "Cores";
    m_translationsEn["recommended"] = "Recommended";
    m_translationsEn["language"] = "Language";
    m_translationsEn["theme"] = "Theme";
    m_translationsEn["font_size"] = "Font Size";
    m_translationsEn["system_prompt"] = "System Prompt";
    m_translationsEn["personality"] = "Personality";
    m_translationsEn["temperature_label"] = "Temperature";
    m_translationsEn["max_tokens"] = "Max Tokens";
    m_translationsEn["context_window"] = "Context Window";
    m_translationsEn["scanning_hardware"] = "Scanning Hardware...";
    m_translationsEn["model_recommendation"] = "Model Recommendation";
    m_translationsEn["ready"] = "Ready to Go!";
    m_translationsEn["ready_subtitle"] = "Nomad LLM is ready to assist you. Start your first conversation!";
    m_translationsEn["upload_document"] = "Upload Document";
    m_translationsEn["attach_image"] = "Attach Image";
    m_translationsEn["copy"] = "Copy";
    m_translationsEn["regenerate"] = "Regenerate";
    m_translationsEn["search"] = "Search";
    m_translationsEn["no_sessions"] = "No chats yet";
    m_translationsEn["session_deleted"] = "Session deleted";
    m_translationsEn["model_loaded"] = "Model loaded";
    m_translationsEn["model_unloaded"] = "Model unloaded";
    m_translationsEn["download_complete"] = "Download complete";
    m_translationsEn["download_failed"] = "Download failed";
    m_translationsEn["error"] = "Error";
    m_translationsEn["connected"] = "Connected";
    m_translationsEn["disconnected"] = "Disconnected";
    m_translationsEn["tokens_per_sec"] = "tokens/sec";
    m_translationsEn["export_data"] = "Export Data";
    m_translationsEn["clear_all_data"] = "Clear All Data";
    m_translationsEn["confirm_clear"] = "Are you sure you want to clear all data?";
    m_translationsEn["about"] = "About";
    m_translationsEn["version"] = "Version";
    m_translationsEn["loading_model"] = "Loading model...";
    m_translationsEn["stop"] = "Stop";
    m_translationsEn["dark"] = "Dark";
    m_translationsEn["light"] = "Light";
    m_translationsEn["general_assistant"] = "General Assistant";
    m_translationsEn["not_downloaded"] = "Not Downloaded";
    m_translationsEn["free_space"] = "Free Space";
    m_translationsEn["min_ram"] = "Min RAM";
    m_translationsEn["session"] = "Session";
    m_translationsEn["delete_model_confirm"] = "Are you sure you want to delete this model?";
    m_translationsEn["delete_session_confirm"] = "Are you sure you want to delete this session?";
}

void SettingsManager::loadConfig() {
    QString configPath = QCoreApplication::applicationDirPath() + "/config.json";
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        // Fallback for tests/build dir
        file.setFileName("config.json");
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open config.json at" << configPath;
            return;
        }
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        m_config = doc.object();
    } else {
        qWarning() << "Invalid JSON in config.json";
    }
}

QString SettingsManager::hfBaseUrl() const {
    if (m_config.contains("huggingface_base_url")) {
        return m_config["huggingface_base_url"].toString();
    }
    return "https://huggingface.co/%1/resolve/main/%2";
}

QString SettingsManager::systemPrompt(const QString &key, bool isId) const {
    QString configKey = isId ? "system_prompts_id" : "system_prompts_en";
    if (m_config.contains(configKey)) {
        QJsonObject prompts = m_config[configKey].toObject();
        if (prompts.contains(key)) {
            return prompts[key].toString();
        }
    }
    return "";
}

// Basic obfuscation for secure setting storage
// For production, consider using OS keychain/credential vault
static QString obfuscateString(const QString &str) {
    QByteArray data = str.toUtf8();
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ 0x42;
    }
    return QString::fromLatin1(data.toBase64());
}

static QString deobfuscateString(const QString &str) {
    QByteArray data = QByteArray::fromBase64(str.toLatin1());
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ 0x42;
    }
    return QString::fromUtf8(data);
}

void SettingsManager::secureSet(const QString &key, const QString &value) {
    if (value.isEmpty()) {
        m_settings->remove("Secure/" + key);
    } else {
        m_settings->setValue("Secure/" + key, obfuscateString(value));
    }
}

QString SettingsManager::secureGet(const QString &key, const QString &defaultValue) const {
    QString stored = m_settings->value("Secure/" + key).toString();
    if (stored.isEmpty()) {
        return defaultValue;
    }
    return deobfuscateString(stored);
}
