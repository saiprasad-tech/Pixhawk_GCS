#include "BluetoothManager.h"
#include <QDebug>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
    , m_deviceModel(new QStringListModel(this))
{
    // Default UUIDs (configurable via settings)
    m_serviceUuid = QBluetoothUuid(QString("0000abcd-0000-1000-8000-00805f9b34fb"));
    m_ssidCharUuid = QBluetoothUuid(QString("0000abce-0000-1000-8000-00805f9b34fb"));
    m_passwordCharUuid = QBluetoothUuid(QString("0000abcf-0000-1000-8000-00805f9b34fb"));
    m_commandCharUuid = QBluetoothUuid(QString("0000abd0-0000-1000-8000-00805f9b34fb"));

    // Setup discovery agent
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothManager::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothManager::onDiscoveryFinished);
    connect(m_discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &BluetoothManager::onDiscoveryError);
}

BluetoothManager::~BluetoothManager()
{
    disconnect();
}

void BluetoothManager::startScan()
{
    if (m_isScanning) {
        return;
    }

    m_discoveredDevices.clear();
    m_deviceModel->setStringList(QStringList());
    
    setIsScanning(true);
    setConnectionStatus("Scanning for devices...");
    
    m_discoveryAgent->start();
    qDebug() << "BluetoothManager: Started device discovery";
}

void BluetoothManager::stopScan()
{
    if (m_isScanning) {
        m_discoveryAgent->stop();
        setIsScanning(false);
        setConnectionStatus("Scan stopped");
    }
}

void BluetoothManager::connectToDevice(int index)
{
    if (index < 0 || index >= m_discoveredDevices.size()) {
        qWarning() << "BluetoothManager: Invalid device index" << index;
        return;
    }

    if (m_controller) {
        m_controller->deleteLater();
        m_controller = nullptr;
    }

    const QBluetoothDeviceInfo device = m_discoveredDevices.at(index);
    setConnectionStatus("Connecting to " + device.name() + "...");
    
    m_controller = QLowEnergyController::createCentral(device, this);
    
    connect(m_controller, &QLowEnergyController::connected,
            this, &BluetoothManager::onControllerConnected);
    connect(m_controller, &QLowEnergyController::disconnected,
            this, &BluetoothManager::onControllerDisconnected);
    connect(m_controller, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
            this, &BluetoothManager::onControllerError);
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, &BluetoothManager::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &BluetoothManager::onServiceDiscoveryFinished);

    m_controller->connectToDevice();
}

void BluetoothManager::disconnect()
{
    if (m_controller && m_controller->state() == QLowEnergyController::ConnectedState) {
        m_controller->disconnectFromDevice();
    }
    
    setIsConnected(false);
    setConnectionStatus("Disconnected");
}

void BluetoothManager::sendWifiCredentials(const QString &ssid, const QString &password)
{
    if (!m_isConnected || !m_service) {
        qWarning() << "BluetoothManager: Not connected or service not available";
        return;
    }

    // Write SSID
    writeCharacteristic(m_ssidCharUuid, ssid.toUtf8());
    
    // Write password
    writeCharacteristic(m_passwordCharUuid, password.toUtf8());
    
    qDebug() << "BluetoothManager: Sent WiFi credentials";
}

void BluetoothManager::startTracking()
{
    if (!m_isConnected || !m_service) {
        qWarning() << "BluetoothManager: Not connected or service not available";
        return;
    }

    writeCharacteristic(m_commandCharUuid, "TRACK_START");
    qDebug() << "BluetoothManager: Sent start tracking command";
}

void BluetoothManager::stopTracking()
{
    if (!m_isConnected || !m_service) {
        qWarning() << "BluetoothManager: Not connected or service not available";
        return;
    }

    writeCharacteristic(m_commandCharUuid, "TRACK_STOP");
    qDebug() << "BluetoothManager: Sent stop tracking command";
}

void BluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        m_discoveredDevices.append(device);
        
        QStringList deviceNames;
        for (const auto &dev : m_discoveredDevices) {
            QString name = dev.name().isEmpty() ? "Unknown Device" : dev.name();
            deviceNames.append(name + " (" + dev.address().toString() + ")");
        }
        
        m_deviceModel->setStringList(deviceNames);
        emit deviceModelChanged();
        
        qDebug() << "BluetoothManager: Discovered BLE device:" << device.name() << device.address().toString();
    }
}

void BluetoothManager::onDiscoveryFinished()
{
    setIsScanning(false);
    setConnectionStatus(QString("Found %1 devices").arg(m_discoveredDevices.size()));
    qDebug() << "BluetoothManager: Discovery finished, found" << m_discoveredDevices.size() << "BLE devices";
}

void BluetoothManager::onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    setIsScanning(false);
    setConnectionStatus("Discovery error: " + m_discoveryAgent->errorString());
    qWarning() << "BluetoothManager: Discovery error:" << error << m_discoveryAgent->errorString();
}

void BluetoothManager::onControllerConnected()
{
    setConnectionStatus("Connected, discovering services...");
    m_controller->discoverServices();
}

void BluetoothManager::onControllerDisconnected()
{
    setIsConnected(false);
    setConnectionStatus("Disconnected");
    
    if (m_service) {
        m_service->deleteLater();
        m_service = nullptr;
    }
}

void BluetoothManager::onControllerError(QLowEnergyController::Error error)
{
    setIsConnected(false);
    setConnectionStatus("Connection error: " + m_controller->errorString());
    qWarning() << "BluetoothManager: Controller error:" << error << m_controller->errorString();
}

void BluetoothManager::onServiceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == m_serviceUuid) {
        setConnectionStatus("Target service found, connecting...");
        qDebug() << "BluetoothManager: Found target service:" << gatt.toString();
    }
}

void BluetoothManager::onServiceDiscoveryFinished()
{
    if (m_service) {
        m_service->deleteLater();
        m_service = nullptr;
    }

    m_service = m_controller->createServiceObject(m_serviceUuid, this);
    
    if (!m_service) {
        setConnectionStatus("Target service not found");
        qWarning() << "BluetoothManager: Could not create service object";
        return;
    }

    connect(m_service, &QLowEnergyService::stateChanged,
            this, &BluetoothManager::onServiceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicWritten,
            this, &BluetoothManager::onCharacteristicWritten);

    m_service->discoverDetails();
}

void BluetoothManager::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    if (state == QLowEnergyService::ServiceDiscovered) {
        setIsConnected(true);
        setConnectionStatus("Connected and ready");
        qDebug() << "BluetoothManager: Service ready, connection established";
    }
}

void BluetoothManager::onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "BluetoothManager: Characteristic written:" << characteristic.uuid().toString() << newValue;
}

void BluetoothManager::setConnectionStatus(const QString &status)
{
    if (m_connectionStatus != status) {
        m_connectionStatus = status;
        emit connectionStatusChanged();
    }
}

void BluetoothManager::setIsConnected(bool connected)
{
    if (m_isConnected != connected) {
        m_isConnected = connected;
        emit isConnectedChanged();
    }
}

void BluetoothManager::setIsScanning(bool scanning)
{
    if (m_isScanning != scanning) {
        m_isScanning = scanning;
        emit isScanningChanged();
    }
}

void BluetoothManager::writeCharacteristic(const QBluetoothUuid &uuid, const QByteArray &data)
{
    if (!m_service) {
        qWarning() << "BluetoothManager: Service not available for writing";
        return;
    }

    const QLowEnergyCharacteristic characteristic = m_service->characteristic(uuid);
    if (!characteristic.isValid()) {
        qWarning() << "BluetoothManager: Characteristic not found:" << uuid.toString();
        return;
    }

    m_service->writeCharacteristic(characteristic, data);
}