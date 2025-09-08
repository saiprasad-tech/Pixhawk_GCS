#pragma once

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QStringListModel>
#include <QString>

class BluetoothManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringListModel* deviceModel READ deviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager();

    // Property getters
    QStringListModel* deviceModel() { return m_deviceModel; }
    QString connectionStatus() const { return m_connectionStatus; }
    bool isConnected() const { return m_isConnected; }
    bool isScanning() const { return m_isScanning; }

public slots:
    void startScan();
    void stopScan();
    void connectToDevice(int index);
    void disconnect();
    void sendWifiCredentials(const QString &ssid, const QString &password);
    void startTracking();
    void stopTracking();

signals:
    void deviceModelChanged();
    void connectionStatusChanged();
    void isConnectedChanged();
    void isScanningChanged();

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onDiscoveryFinished();
    void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void onControllerConnected();
    void onControllerDisconnected();
    void onControllerError(QLowEnergyController::Error error);
    void onServiceDiscovered(const QBluetoothUuid &gatt);
    void onServiceDiscoveryFinished();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

private:
    void setConnectionStatus(const QString &status);
    void setIsConnected(bool connected);
    void setIsScanning(bool scanning);
    void writeCharacteristic(const QBluetoothUuid &uuid, const QByteArray &data);

    // Device discovery
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    QStringListModel *m_deviceModel = nullptr;
    QList<QBluetoothDeviceInfo> m_discoveredDevices;

    // BLE connection
    QLowEnergyController *m_controller = nullptr;
    QLowEnergyService *m_service = nullptr;

    // Service and characteristic UUIDs
    QBluetoothUuid m_serviceUuid;
    QBluetoothUuid m_ssidCharUuid;
    QBluetoothUuid m_passwordCharUuid;
    QBluetoothUuid m_commandCharUuid;

    // State
    QString m_connectionStatus = "Disconnected";
    bool m_isConnected = false;
    bool m_isScanning = false;
};