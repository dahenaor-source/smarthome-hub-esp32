#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <string>

#define BLE_DRV_DEVICE_NAME    "SmartHomeHub"
#define BLE_DRV_SERVICE_UUID   "12345678-1234-1234-1234-123456789abc"
#define BLE_DRV_CHAR_CMD_UUID  "12345678-1234-1234-1234-123456789abd"
#define BLE_DRV_CHAR_NTFY_UUID "12345678-1234-1234-1234-123456789abe"
#define BLE_DRV_TX_POWER        ESP_PWR_LVL_P9

typedef enum {
    BLE_STATE_IDLE         = 0,
    BLE_STATE_ADVERTISING  = 1,
    BLE_STATE_CONNECTED    = 2,
    BLE_STATE_DISCONNECTED = 3
} BleState_t;

typedef void (*BleCmdCallback_t)(char command);

class BleDriver {
public:
    BleDriver();
    void       begin();
    void       update();
    bool       sendNotification(const char* message);
    void       setCmdCallback(BleCmdCallback_t callback);
    BleState_t getState() const;
    bool       isClientConnected() const;

    void _onConnect();
    void _onDisconnect();
    void _onCommandWrite(const std::string& value);

private:
    NimBLEServer*         _pServer;
    BleState_t            _state;
    NimBLECharacteristic* _pCmdChar;
    NimBLECharacteristic* _pNotifyChar;
    BleCmdCallback_t      _cmdCallback;
    bool                  _wasConnected;
    std::string           _lastReadValue;   // <-- NUEVO
};

extern BleDriver* g_bleDriverInstance;