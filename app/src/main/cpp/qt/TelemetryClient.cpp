#include "TelemetryClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

TelemetryClient::TelemetryClient(QObject *parent)
    : QObject(parent)
    , m_udpSocket(new QUdpSocket(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_httpTimer(new QTimer(this))
{
    // Setup UDP socket
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &TelemetryClient::onUdpDataReceived);
    
    // Setup HTTP timer
    m_httpTimer->setSingleShot(false);
    m_httpTimer->setInterval(1000); // 1 Hz default
    connect(m_httpTimer, &QTimer::timeout, this, &TelemetryClient::requestTelemetryData);
    
    // Setup network manager
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &TelemetryClient::onHttpReplyFinished);
}

TelemetryClient::~TelemetryClient()
{
    stop();
}

void TelemetryClient::setMode(const QString &mode)
{
    if (m_mode != mode) {
        bool wasRunning = m_isRunning;
        if (wasRunning) {
            stop();
        }
        
        m_mode = mode;
        emit modeChanged();
        
        if (wasRunning) {
            start();
        }
    }
}

void TelemetryClient::setHost(const QString &host)
{
    if (m_host != host) {
        m_host = host;
        emit hostChanged();
    }
}

void TelemetryClient::setPort(int port)
{
    if (m_port != port) {
        bool wasRunning = m_isRunning;
        if (wasRunning && m_mode == "udp") {
            stop();
        }
        
        m_port = port;
        emit portChanged();
        
        if (wasRunning && m_mode == "udp") {
            start();
        }
    }
}

void TelemetryClient::start()
{
    if (m_isRunning) {
        return;
    }
    
    m_isRunning = true;
    
    if (m_mode == "udp") {
        // Bind UDP socket to listen for telemetry
        if (m_udpSocket->bind(QHostAddress::Any, m_port)) {
            qDebug() << "TelemetryClient: UDP listening on port" << m_port;
        } else {
            qWarning() << "TelemetryClient: Failed to bind UDP socket to port" << m_port;
        }
    } else if (m_mode == "http") {
        // Start HTTP polling timer
        m_httpTimer->start();
        qDebug() << "TelemetryClient: HTTP polling started for" << m_host;
    }
}

void TelemetryClient::stop()
{
    if (!m_isRunning) {
        return;
    }
    
    m_isRunning = false;
    
    if (m_udpSocket->state() == QAbstractSocket::BoundState) {
        m_udpSocket->close();
    }
    
    m_httpTimer->stop();
    
    qDebug() << "TelemetryClient: Stopped";
}

void TelemetryClient::onUdpDataReceived()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        
        QHostAddress sender;
        quint16 senderPort;
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        
        // Parse JSON telemetry data
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            updateFromJson(doc.object());
        }
    }
}

void TelemetryClient::onHttpReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            updateFromJson(doc.object());
        }
    } else {
        qWarning() << "TelemetryClient HTTP error:" << reply->errorString();
    }
    
    reply->deleteLater();
}

void TelemetryClient::requestTelemetryData()
{
    if (!m_isRunning) {
        return;
    }
    
    QString url = QString("http://%1:%2/api/telemetry").arg(m_host).arg(8000); // Default API port
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    m_networkManager->get(request);
}

void TelemetryClient::updateFromJson(const QJsonObject &data)
{
    bool positionChanged = false;
    
    // Update coordinate
    if (data.contains("lat") && data.contains("lon")) {
        double lat = data["lat"].toDouble();
        double lon = data["lon"].toDouble();
        
        QGeoCoordinate newCoord(lat, lon);
        if (newCoord.isValid() && newCoord != m_coordinate) {
            m_coordinate = newCoord;
            m_hasValidPosition = true;
            positionChanged = true;
        }
    }
    
    // Update altitude
    if (data.contains("alt")) {
        double newAlt = data["alt"].toDouble();
        if (qAbs(newAlt - m_altitude) > 0.1) {
            m_altitude = newAlt;
            emit altitudeChanged();
        }
    }
    
    // Update fix type
    if (data.contains("fix")) {
        QString newFix = data["fix"].toString();
        if (newFix != m_fixType) {
            m_fixType = newFix;
            emit fixTypeChanged();
        }
    }
    
    // Update satellites
    if (data.contains("sats")) {
        int newSats = data["sats"].toInt();
        if (newSats != m_satellites) {
            m_satellites = newSats;
            emit satellitesChanged();
        }
    }
    
    if (positionChanged) {
        emit positionChanged();
    }
    
    updateLastUpdateTime();
}

void TelemetryClient::updateLastUpdateTime()
{
    QString newTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    if (newTime != m_lastUpdateTime) {
        m_lastUpdateTime = newTime;
        emit lastUpdateChanged();
    }
}