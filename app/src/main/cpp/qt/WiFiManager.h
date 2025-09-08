#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class WiFiManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString setupStatus READ setupStatus NOTIFY setupStatusChanged)
    Q_PROPERTY(bool setupComplete READ setupComplete NOTIFY setupCompleteChanged)
    Q_PROPERTY(bool isSending READ isSending NOTIFY isSendingChanged)

public:
    explicit WiFiManager(QObject *parent = nullptr);
    ~WiFiManager();

    // Property getters
    QString setupStatus() const { return m_setupStatus; }
    bool setupComplete() const { return m_setupComplete; }
    bool isSending() const { return m_isSending; }

public slots:
    void sendCredentials(const QString &ssid, const QString &password);
    void resetSetup();

signals:
    void setupStatusChanged();
    void setupCompleteChanged();
    void isSendingChanged();

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    void setSetupStatus(const QString &status);
    void setSetupComplete(bool complete);
    void setIsSending(bool sending);

    QString m_setupStatus = "Ready to send credentials";
    bool m_setupComplete = false;
    bool m_isSending = false;

    QNetworkAccessManager *m_networkManager = nullptr;
};