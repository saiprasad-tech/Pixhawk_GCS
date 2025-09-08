#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString googleMapsApiKey READ googleMapsApiKey WRITE setGoogleMapsApiKey NOTIFY googleMapsApiKeyChanged)
    Q_PROPERTY(QString piHost READ piHost WRITE setPiHost NOTIFY piHostChanged)
    Q_PROPERTY(int telemetryPort READ telemetryPort WRITE setTelemetryPort NOTIFY telemetryPortChanged)
    Q_PROPERTY(int apiPort READ apiPort WRITE setApiPort NOTIFY apiPortChanged)
    Q_PROPERTY(QString cameraUrl READ cameraUrl WRITE setCameraUrl NOTIFY cameraUrlChanged)
    Q_PROPERTY(int cameraRefreshRate READ cameraRefreshRate WRITE setCameraRefreshRate NOTIFY cameraRefreshRateChanged)
    Q_PROPERTY(QString telemetryMode READ telemetryMode WRITE setTelemetryMode NOTIFY telemetryModeChanged)
    Q_PROPERTY(int telemetryUpdateRate READ telemetryUpdateRate WRITE setTelemetryUpdateRate NOTIFY telemetryUpdateRateChanged)
    Q_PROPERTY(QString bleServiceUuid READ bleServiceUuid WRITE setBleServiceUuid NOTIFY bleServiceUuidChanged)
    Q_PROPERTY(QString bleSsidUuid READ bleSsidUuid WRITE setBleSsidUuid NOTIFY bleSsidUuidChanged)
    Q_PROPERTY(QString blePasswordUuid READ blePasswordUuid WRITE setBlePasswordUuid NOTIFY blePasswordUuidChanged)
    Q_PROPERTY(QString bleCommandUuid READ bleCommandUuid WRITE setBleCommandUuid NOTIFY bleCommandUuidChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager();

    // Property getters
    QString googleMapsApiKey() const { return m_googleMapsApiKey; }
    QString piHost() const { return m_piHost; }
    int telemetryPort() const { return m_telemetryPort; }
    int apiPort() const { return m_apiPort; }
    QString cameraUrl() const { return m_cameraUrl; }
    int cameraRefreshRate() const { return m_cameraRefreshRate; }
    QString telemetryMode() const { return m_telemetryMode; }
    int telemetryUpdateRate() const { return m_telemetryUpdateRate; }
    QString bleServiceUuid() const { return m_bleServiceUuid; }
    QString bleSsidUuid() const { return m_bleSsidUuid; }
    QString blePasswordUuid() const { return m_blePasswordUuid; }
    QString bleCommandUuid() const { return m_bleCommandUuid; }

    // Property setters
    void setGoogleMapsApiKey(const QString &key);
    void setPiHost(const QString &host);
    void setTelemetryPort(int port);
    void setApiPort(int port);
    void setCameraUrl(const QString &url);
    void setCameraRefreshRate(int rate);
    void setTelemetryMode(const QString &mode);
    void setTelemetryUpdateRate(int rate);
    void setBleServiceUuid(const QString &uuid);
    void setBleSsidUuid(const QString &uuid);
    void setBlePasswordUuid(const QString &uuid);
    void setBleCommandUuid(const QString &uuid);

public slots:
    void resetToDefaults();
    void saveSettings();
    void loadSettings();
    void testConnection();

signals:
    void googleMapsApiKeyChanged();
    void piHostChanged();
    void telemetryPortChanged();
    void apiPortChanged();
    void cameraUrlChanged();
    void cameraRefreshRateChanged();
    void telemetryModeChanged();
    void telemetryUpdateRateChanged();
    void bleServiceUuidChanged();
    void bleSsidUuidChanged();
    void blePasswordUuidChanged();
    void bleCommandUuidChanged();

private:
    void loadDefaults();

    QSettings *m_settings = nullptr;

    // Settings values
    QString m_googleMapsApiKey;
    QString m_piHost = "pi.local";
    int m_telemetryPort = 14555;
    int m_apiPort = 8000;
    QString m_cameraUrl = "http://pi.local:8080/stream.mjpeg";
    int m_cameraRefreshRate = 30;
    QString m_telemetryMode = "udp";
    int m_telemetryUpdateRate = 2;
    QString m_bleServiceUuid = "0000abcd-0000-1000-8000-00805f9b34fb";
    QString m_bleSsidUuid = "0000abce-0000-1000-8000-00805f9b34fb";
    QString m_blePasswordUuid = "0000abcf-0000-1000-8000-00805f9b34fb";
    QString m_bleCommandUuid = "0000abd0-0000-1000-8000-00805f9b34fb";
};