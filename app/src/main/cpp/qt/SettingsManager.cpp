#include "SettingsManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
{
    // Create settings with app-specific organization and application name
    m_settings = new QSettings("PixhawkGCS", "Settings", this);
    loadSettings();
}

SettingsManager::~SettingsManager()
{
    saveSettings();
}

void SettingsManager::setGoogleMapsApiKey(const QString &key)
{
    if (m_googleMapsApiKey != key) {
        m_googleMapsApiKey = key;
        emit googleMapsApiKeyChanged();
    }
}

void SettingsManager::setPiHost(const QString &host)
{
    if (m_piHost != host) {
        m_piHost = host;
        emit piHostChanged();
    }
}

void SettingsManager::setTelemetryPort(int port)
{
    if (m_telemetryPort != port) {
        m_telemetryPort = port;
        emit telemetryPortChanged();
    }
}

void SettingsManager::setApiPort(int port)
{
    if (m_apiPort != port) {
        m_apiPort = port;
        emit apiPortChanged();
    }
}

void SettingsManager::setCameraUrl(const QString &url)
{
    if (m_cameraUrl != url) {
        m_cameraUrl = url;
        emit cameraUrlChanged();
    }
}

void SettingsManager::setCameraRefreshRate(int rate)
{
    if (m_cameraRefreshRate != rate) {
        m_cameraRefreshRate = rate;
        emit cameraRefreshRateChanged();
    }
}

void SettingsManager::setTelemetryMode(const QString &mode)
{
    if (m_telemetryMode != mode) {
        m_telemetryMode = mode;
        emit telemetryModeChanged();
    }
}

void SettingsManager::setTelemetryUpdateRate(int rate)
{
    if (m_telemetryUpdateRate != rate) {
        m_telemetryUpdateRate = rate;
        emit telemetryUpdateRateChanged();
    }
}

void SettingsManager::setBleServiceUuid(const QString &uuid)
{
    if (m_bleServiceUuid != uuid) {
        m_bleServiceUuid = uuid;
        emit bleServiceUuidChanged();
    }
}

void SettingsManager::setBleSsidUuid(const QString &uuid)
{
    if (m_bleSsidUuid != uuid) {
        m_bleSsidUuid = uuid;
        emit bleSsidUuidChanged();
    }
}

void SettingsManager::setBlePasswordUuid(const QString &uuid)
{
    if (m_blePasswordUuid != uuid) {
        m_blePasswordUuid = uuid;
        emit blePasswordUuidChanged();
    }
}

void SettingsManager::setBleCommandUuid(const QString &uuid)
{
    if (m_bleCommandUuid != uuid) {
        m_bleCommandUuid = uuid;
        emit bleCommandUuidChanged();
    }
}

void SettingsManager::resetToDefaults()
{
    loadDefaults();
    saveSettings();
    qDebug() << "SettingsManager: Reset to defaults";
}

void SettingsManager::saveSettings()
{
    m_settings->setValue("googleMapsApiKey", m_googleMapsApiKey);
    m_settings->setValue("piHost", m_piHost);
    m_settings->setValue("telemetryPort", m_telemetryPort);
    m_settings->setValue("apiPort", m_apiPort);
    m_settings->setValue("cameraUrl", m_cameraUrl);
    m_settings->setValue("cameraRefreshRate", m_cameraRefreshRate);
    m_settings->setValue("telemetryMode", m_telemetryMode);
    m_settings->setValue("telemetryUpdateRate", m_telemetryUpdateRate);
    m_settings->setValue("bleServiceUuid", m_bleServiceUuid);
    m_settings->setValue("bleSsidUuid", m_bleSsidUuid);
    m_settings->setValue("blePasswordUuid", m_blePasswordUuid);
    m_settings->setValue("bleCommandUuid", m_bleCommandUuid);
    
    m_settings->sync();
    qDebug() << "SettingsManager: Settings saved";
}

void SettingsManager::loadSettings()
{
    // Load with defaults
    m_googleMapsApiKey = m_settings->value("googleMapsApiKey", "").toString();
    m_piHost = m_settings->value("piHost", "pi.local").toString();
    m_telemetryPort = m_settings->value("telemetryPort", 14555).toInt();
    m_apiPort = m_settings->value("apiPort", 8000).toInt();
    m_cameraUrl = m_settings->value("cameraUrl", "http://pi.local:8080/stream.mjpeg").toString();
    m_cameraRefreshRate = m_settings->value("cameraRefreshRate", 30).toInt();
    m_telemetryMode = m_settings->value("telemetryMode", "udp").toString();
    m_telemetryUpdateRate = m_settings->value("telemetryUpdateRate", 2).toInt();
    m_bleServiceUuid = m_settings->value("bleServiceUuid", "0000abcd-0000-1000-8000-00805f9b34fb").toString();
    m_bleSsidUuid = m_settings->value("bleSsidUuid", "0000abce-0000-1000-8000-00805f9b34fb").toString();
    m_blePasswordUuid = m_settings->value("blePasswordUuid", "0000abcf-0000-1000-8000-00805f9b34fb").toString();
    m_bleCommandUuid = m_settings->value("bleCommandUuid", "0000abd0-0000-1000-8000-00805f9b34fb").toString();
    
    qDebug() << "SettingsManager: Settings loaded";
}

void SettingsManager::testConnection()
{
    // This could test various connections (Pi host, camera URL, etc.)
    qDebug() << "SettingsManager: Testing connections...";
    qDebug() << "Pi Host:" << m_piHost;
    qDebug() << "Camera URL:" << m_cameraUrl;
    qDebug() << "Telemetry Port:" << m_telemetryPort;
}

void SettingsManager::loadDefaults()
{
    setGoogleMapsApiKey("");
    setPiHost("pi.local");
    setTelemetryPort(14555);
    setApiPort(8000);
    setCameraUrl("http://pi.local:8080/stream.mjpeg");
    setCameraRefreshRate(30);
    setTelemetryMode("udp");
    setTelemetryUpdateRate(2);
    setBleServiceUuid("0000abcd-0000-1000-8000-00805f9b34fb");
    setBleSsidUuid("0000abce-0000-1000-8000-00805f9b34fb");
    setBlePasswordUuid("0000abcf-0000-1000-8000-00805f9b34fb");
    setBleCommandUuid("0000abd0-0000-1000-8000-00805f9b34fb");
}