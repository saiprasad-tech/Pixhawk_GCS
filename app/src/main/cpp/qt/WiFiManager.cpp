#include "WiFiManager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>

WiFiManager::WiFiManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &WiFiManager::onReplyFinished);
}

WiFiManager::~WiFiManager()
{
}

void WiFiManager::sendCredentials(const QString &ssid, const QString &password)
{
    if (m_isSending) {
        return;
    }

    setIsSending(true);
    setSetupComplete(false);
    setSetupStatus("Sending WiFi credentials...");

    // Create JSON payload
    QJsonObject payload;
    payload["ssid"] = ssid;
    payload["password"] = password;
    payload["security"] = "WPA2"; // Default security type

    QJsonDocument doc(payload);
    QByteArray data = doc.toJson();

    // Send to Pi's SoftAP endpoint
    QNetworkRequest request(QUrl("http://192.168.4.1/api/wifi"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "PixhawkGCS/1.0");
    
    // Set timeout
    request.setTransferTimeout(10000); // 10 seconds

    QNetworkReply *reply = m_networkManager->post(request, data);
    reply->setProperty("requestType", "sendCredentials");

    qDebug() << "WiFiManager: Sending credentials to Pi" << ssid;
}

void WiFiManager::resetSetup()
{
    setSetupComplete(false);
    setIsSending(false);
    setSetupStatus("Ready to send credentials");
    qDebug() << "WiFiManager: Setup reset";
}

void WiFiManager::onReplyFinished(QNetworkReply *reply)
{
    QString requestType = reply->property("requestType").toString();
    
    setIsSending(false);

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject response = doc.object();
            
            if (response["success"].toBool()) {
                setSetupComplete(true);
                setSetupStatus("✅ WiFi credentials sent successfully! Pi will restart and connect to your network.");
                qDebug() << "WiFiManager: Credentials sent successfully";
            } else {
                QString errorMsg = response["error"].toString();
                setSetupStatus("❌ Error: " + errorMsg);
                qWarning() << "WiFiManager: Server error:" << errorMsg;
            }
        } else {
            setSetupComplete(true); // Assume success if we can't parse response
            setSetupStatus("✅ Credentials sent (response parsing failed, but likely successful)");
            qDebug() << "WiFiManager: Response parsing failed, assuming success";
        }
    } else {
        QString errorString;
        
        switch (reply->error()) {
        case QNetworkReply::TimeoutError:
            // Timeout might actually indicate success (Pi restarting)
            setSetupComplete(true);
            errorString = "✅ Request timed out (Pi likely restarting - this is normal)";
            break;
        case QNetworkReply::ConnectionRefusedError:
            errorString = "❌ Connection refused - make sure you're connected to Pi's WiFi";
            break;
        case QNetworkReply::HostNotFoundError:
            errorString = "❌ Pi not found - check your connection to Pi's WiFi network";
            break;
        default:
            errorString = "❌ Network error: " + reply->errorString();
            break;
        }
        
        setSetupStatus(errorString);
        qWarning() << "WiFiManager: Network error:" << reply->error() << reply->errorString();
    }

    reply->deleteLater();
}

void WiFiManager::setSetupStatus(const QString &status)
{
    if (m_setupStatus != status) {
        m_setupStatus = status;
        emit setupStatusChanged();
    }
}

void WiFiManager::setSetupComplete(bool complete)
{
    if (m_setupComplete != complete) {
        m_setupComplete = complete;
        emit setupCompleteChanged();
    }
}

void WiFiManager::setIsSending(bool sending)
{
    if (m_isSending != sending) {
        m_isSending = sending;
        emit isSendingChanged();
    }
}