#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <string.h>

// ===== CAMBIADO: Pines válidos para ESP32-C6 =====
#define UART_DRV_TX_PIN       4    // <-- antes era 16
#define UART_DRV_RX_PIN       5    // <-- antes era 17
// ==================================================

#define UART_DRV_BAUDRATE     115200
#define UART_DRV_BUFFER_SIZE  64
#define UART_DRV_TIMEOUT_MS   500
#define UART_DRV_END_CHAR     '\n'

typedef enum {
    UART_FRAME_NONE    = 0,
    UART_FRAME_ACK     = 1,
    UART_FRAME_TEMP    = 2,
    UART_FRAME_INVALID = 3
} UartFrameType_t;

typedef struct {
    UartFrameType_t type;
    char raw  [UART_DRV_BUFFER_SIZE];
    char value[UART_DRV_BUFFER_SIZE];
} UartFrame_t;

typedef void (*UartFrameCallback_t)(const UartFrame_t* frame);

class UartDriver {
public:
    explicit UartDriver(HardwareSerial& serialPort);

    void               begin();
    void               update();
    bool               sendCommand(char cmd);
    void               setFrameCallback(UartFrameCallback_t callback);
    bool               isConnected()   const;
    const UartFrame_t& getLastFrame()  const;
    void               clearBuffer();

private:
    HardwareSerial&     _serial;
    char                _rxBuffer[UART_DRV_BUFFER_SIZE];
    uint8_t             _rxIndex;
    bool                _initialized;
    UartFrameCallback_t _callback;
    UartFrame_t         _lastFrame;
    unsigned long       _frameStartMs;

    void _parseFrame();
    void _resetBuffer();
    bool _isFrameTimedOut() const;
};