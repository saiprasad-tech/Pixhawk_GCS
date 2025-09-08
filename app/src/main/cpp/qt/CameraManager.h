#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QString>

class CameraManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isStreaming READ isStreaming NOTIFY isStreamingChanged)
    Q_PROPERTY(QString streamUrl READ streamUrl WRITE setStreamUrl NOTIFY streamUrlChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(int fps READ fps NOTIFY fpsChanged)

public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

    // Property getters
    bool isStreaming() const { return m_isStreaming; }
    QString streamUrl() const { return m_streamUrl; }
    QString status() const { return m_status; }
    int fps() const { return m_fps; }

    // Property setters
    void setStreamUrl(const QString &url);

public slots:
    void startStream();
    void stopStream();
    void refreshStream();
    void testConnection(const QString &url = QString());

signals:
    void isStreamingChanged();
    void streamUrlChanged();
    void statusChanged();
    void fpsChanged();

private slots:
    void onTestReplyFinished(QNetworkReply *reply);
    void onFpsTimerTimeout();

private:
    void setStatus(const QString &status);
    void setIsStreaming(bool streaming);
    void updateFps();

    QString m_streamUrl = "http://pi.local:8080/stream.mjpeg";
    QString m_status = "Ready";
    bool m_isStreaming = false;
    int m_fps = 0;

    QNetworkAccessManager *m_networkManager = nullptr;
    QTimer *m_fpsTimer = nullptr;
    int m_frameCount = 0;
};