#include "CameraManager.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

CameraManager::CameraManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_fpsTimer(new QTimer(this))
{
    // Setup FPS timer
    m_fpsTimer->setInterval(1000); // 1 second
    m_fpsTimer->setSingleShot(false);
    connect(m_fpsTimer, &QTimer::timeout, this, &CameraManager::onFpsTimerTimeout);
    
    // Setup network manager
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &CameraManager::onTestReplyFinished);
}

CameraManager::~CameraManager()
{
    stopStream();
}

void CameraManager::setStreamUrl(const QString &url)
{
    if (m_streamUrl != url) {
        bool wasStreaming = m_isStreaming;
        if (wasStreaming) {
            stopStream();
        }
        
        m_streamUrl = url;
        emit streamUrlChanged();
        
        if (wasStreaming) {
            startStream();
        }
    }
}

void CameraManager::startStream()
{
    if (m_isStreaming) {
        return;
    }
    
    setIsStreaming(true);
    setStatus("Starting stream...");
    
    // Start FPS monitoring
    m_frameCount = 0;
    m_fpsTimer->start();
    
    qDebug() << "CameraManager: Started stream" << m_streamUrl;
}

void CameraManager::stopStream()
{
    if (!m_isStreaming) {
        return;
    }
    
    setIsStreaming(false);
    setStatus("Stream stopped");
    
    m_fpsTimer->stop();
    m_fps = 0;
    emit fpsChanged();
    
    qDebug() << "CameraManager: Stopped stream";
}

void CameraManager::refreshStream()
{
    if (!m_isStreaming) {
        return;
    }
    
    setStatus("Refreshing stream...");
    
    // Force refresh by stopping and starting
    bool wasStreaming = m_isStreaming;
    setIsStreaming(false);
    
    QTimer::singleShot(100, this, [this, wasStreaming]() {
        if (wasStreaming) {
            setIsStreaming(true);
            setStatus("Stream refreshed");
        }
    });
    
    qDebug() << "CameraManager: Refreshed stream";
}

void CameraManager::testConnection(const QString &url)
{
    QString testUrl = url.isEmpty() ? m_streamUrl : url;
    
    setStatus("Testing connection...");
    
    QNetworkRequest request(testUrl);
    request.setRawHeader("User-Agent", "PixhawkGCS/1.0");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    
    // Set a short timeout for testing
    request.setTransferTimeout(5000); // 5 seconds
    
    QNetworkReply *reply = m_networkManager->head(request);
    reply->setProperty("testUrl", testUrl);
    
    qDebug() << "CameraManager: Testing connection to" << testUrl;
}

void CameraManager::onTestReplyFinished(QNetworkReply *reply)
{
    QString testUrl = reply->property("testUrl").toString();
    
    if (reply->error() == QNetworkReply::NoError) {
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        
        if (contentType.contains("multipart/x-mixed-replace") || 
            contentType.contains("image/jpeg") ||
            testUrl.contains("mjpeg")) {
            setStatus("Connection test successful");
            qDebug() << "CameraManager: Connection test successful for" << testUrl;
        } else {
            setStatus("Warning: Unexpected content type: " + contentType);
            qDebug() << "CameraManager: Unexpected content type:" << contentType;
        }
    } else {
        setStatus("Connection test failed: " + reply->errorString());
        qWarning() << "CameraManager: Connection test failed for" << testUrl << reply->errorString();
    }
    
    reply->deleteLater();
}

void CameraManager::onFpsTimerTimeout()
{
    // This is a simplified FPS calculation
    // In a real implementation, you would track actual frame updates
    if (m_isStreaming) {
        m_frameCount++;
        
        // Simulate FPS based on typical MJPEG stream rates
        int simulatedFps = qrand() % 5 + 15; // 15-20 fps simulation
        
        if (simulatedFps != m_fps) {
            m_fps = simulatedFps;
            emit fpsChanged();
        }
        
        if (m_frameCount == 1) {
            setStatus("Streaming");
        }
    }
}

void CameraManager::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void CameraManager::setIsStreaming(bool streaming)
{
    if (m_isStreaming != streaming) {
        m_isStreaming = streaming;
        emit isStreamingChanged();
    }
}

void CameraManager::updateFps()
{
    // This would be called when actual frames are received
    m_frameCount++;
}