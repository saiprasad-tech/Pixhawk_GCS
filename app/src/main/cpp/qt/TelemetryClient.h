#pragma once

#include <QObject>
#include <QGeoCoordinate>
#include <QTimer>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QDateTime>

class TelemetryClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasValidPosition READ hasValidPosition NOTIFY positionChanged)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY positionChanged)
    Q_PROPERTY(double altitude READ altitude NOTIFY altitudeChanged)
    Q_PROPERTY(QString fixType READ fixType NOTIFY fixTypeChanged)
    Q_PROPERTY(int satellites READ satellites NOTIFY satellitesChanged)
    Q_PROPERTY(QString lastUpdateTime READ lastUpdateTime NOTIFY lastUpdateChanged)
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit TelemetryClient(QObject *parent = nullptr);
    ~TelemetryClient();

    // Property getters
    bool hasValidPosition() const { return m_hasValidPosition; }
    QGeoCoordinate coordinate() const { return m_coordinate; }
    double altitude() const { return m_altitude; }
    QString fixType() const { return m_fixType; }
    int satellites() const { return m_satellites; }
    QString lastUpdateTime() const { return m_lastUpdateTime; }
    QString mode() const { return m_mode; }
    QString host() const { return m_host; }
    int port() const { return m_port; }

    // Property setters
    void setMode(const QString &mode);
    void setHost(const QString &host);
    void setPort(int port);

public slots:
    void start();
    void stop();

signals:
    void positionChanged();
    void altitudeChanged();
    void fixTypeChanged();
    void satellitesChanged();
    void lastUpdateChanged();
    void modeChanged();
    void hostChanged();
    void portChanged();

private slots:
    void onUdpDataReceived();
    void onHttpReplyFinished(QNetworkReply *reply);
    void requestTelemetryData();

private:
    void updateFromJson(const QJsonObject &data);
    void updateLastUpdateTime();

    // Properties
    bool m_hasValidPosition = false;
    QGeoCoordinate m_coordinate;
    double m_altitude = 0.0;
    QString m_fixType = "No Fix";
    int m_satellites = 0;
    QString m_lastUpdateTime;
    QString m_mode = "udp"; // "udp" or "http"
    QString m_host = "192.168.1.100";
    int m_port = 14555;

    // Network components
    QUdpSocket *m_udpSocket = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    QTimer *m_httpTimer = nullptr;
    bool m_isRunning = false;
};