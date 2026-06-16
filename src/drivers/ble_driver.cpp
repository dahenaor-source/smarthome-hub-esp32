#include "ble_driver.h"

BleDriver* g_bleDriverInstance = nullptr;

// ================================================================
//  CONSTRUCTOR
// ================================================================
BleDriver::BleDriver()
    : _state(BLE_STATE_IDLE),
      _pServer(nullptr),
      _pCmdChar(nullptr),
      _pNotifyChar(nullptr),
      _cmdCallback(nullptr),
      _wasConnected(false)
{
    g_bleDriverInstance = this;
}

// ================================================================
//  begin()
// ================================================================
void BleDriver::begin() {
    Serial.println("[BLE DRV] Inicializando...");

    NimBLEDevice::init(BLE_DRV_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    _pServer = NimBLEDevice::createServer();

    NimBLEService* pService = _pServer->createService(BLE_DRV_SERVICE_UUID);

    // Caracteristica WRITE (sin callback, usaremos polling)
    _pCmdChar = pService->createCharacteristic(
        BLE_DRV_CHAR_CMD_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    // Valor inicial vacio para detectar cambios
    _pCmdChar->setValue("");

    _pNotifyChar = pService->createCharacteristic(
        BLE_DRV_CHAR_NTFY_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    pService->start();

    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->setName(BLE_DRV_DEVICE_NAME);
    pAdv->start();

    _state = BLE_STATE_ADVERTISING;
    _lastReadValue = "";

    Serial.println("[BLE DRV] Listo.");
    Serial.print("[BLE DRV] MAC: ");
    Serial.println(NimBLEDevice::getAddress().toString().c_str());
    Serial.println("[BLE DRV] Esperando conexion...");
}

// ================================================================
//  update() - Polling de la caracteristica
// ================================================================
void BleDriver::update() {
    static unsigned long lastAdvCheck = 0;

    uint32_t connectedCount = _pServer->getConnectedCount();
    bool physicallyConnected = (connectedCount > 0);

    // Transicion: desconectado -> conectado
    if (physicallyConnected && !_wasConnected) {
        _wasConnected = true;
        _state = BLE_STATE_CONNECTED;
        _lastReadValue = "";
        Serial.println("[BLE DRV] Cliente conectado.");
    }

    // Transicion: conectado -> desconectado
    if (!physicallyConnected && _wasConnected) {
        _wasConnected = false;
        _state = BLE_STATE_ADVERTISING;
        _lastReadValue = "";
        Serial.println("[BLE DRV] Cliente desconectado.");
        delay(300);
        NimBLEDevice::getAdvertising()->start();
    }

    // Polling de la caracteristica WRITE
    if (physicallyConnected && _pCmdChar != nullptr) {
        std::string currentValue = _pCmdChar->getValue();
        
        if (currentValue.length() > 0 && currentValue != _lastReadValue) {
            _lastReadValue = currentValue;
            _onCommandWrite(currentValue);
        }
    }

    // Mantener advertising activo (cada 15s)
    if (!physicallyConnected && (millis() - lastAdvCheck > 15000)) {
        lastAdvCheck = millis();
        if (!NimBLEDevice::getAdvertising()->isAdvertising()) {
            NimBLEDevice::getAdvertising()->start();
        }
    }
}

// ================================================================
//  sendNotification()
// ================================================================
bool BleDriver::sendNotification(const char* message) {
    if (_pServer->getConnectedCount() == 0) {
        Serial.println("[BLE DRV] Sin cliente conectado.");
        return false;
    }
    if (_pNotifyChar == nullptr) return false;

    _pNotifyChar->setValue(message);
    _pNotifyChar->notify();
    Serial.print("[BLE DRV] Notificacion: ");
    Serial.println(message);
    return true;
}

void BleDriver::setCmdCallback(BleCmdCallback_t callback) {
    _cmdCallback = callback;
    Serial.println("[BLE DRV] Callback registrado.");
}

BleState_t BleDriver::getState() const { return _state; }

bool BleDriver::isClientConnected() const {
    if (_pServer == nullptr) return false;
    return (_pServer->getConnectedCount() > 0);
}

void BleDriver::_onConnect() {
    _state = BLE_STATE_CONNECTED;
}

void BleDriver::_onDisconnect() {
    _state = BLE_STATE_DISCONNECTED;
}

void BleDriver::_onCommandWrite(const std::string& value) {
    char cmd = (char)value[0];
    Serial.print("[BLE DRV] Comando recibido: '");
    Serial.print(cmd);
    Serial.println("'");
    if (_cmdCallback != nullptr) {
        _cmdCallback(cmd);
    }
}