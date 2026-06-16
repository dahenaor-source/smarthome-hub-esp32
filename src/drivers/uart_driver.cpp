#include "uart_driver.h"

UartDriver::UartDriver(HardwareSerial& serialPort)
    : _serial(serialPort),
      _rxIndex(0),
      _initialized(false),
      _callback(nullptr),
      _frameStartMs(0)
{
    memset(_rxBuffer,   0, sizeof(_rxBuffer));
    memset(&_lastFrame, 0, sizeof(_lastFrame));
    _lastFrame.type = UART_FRAME_NONE;
}

void UartDriver::begin() {
    _serial.begin(
        UART_DRV_BAUDRATE,
        SERIAL_8N1,
        UART_DRV_RX_PIN,
        UART_DRV_TX_PIN
    );

    _initialized = true;
    _resetBuffer();

    Serial.println("[UART DRV] Driver inicializado.");
    Serial.print("[UART DRV] TX GPIO: ");
    Serial.println(UART_DRV_TX_PIN);
    Serial.print("[UART DRV] RX GPIO: ");
    Serial.println(UART_DRV_RX_PIN);
    Serial.print("[UART DRV] Baudrate: ");
    Serial.println(UART_DRV_BAUDRATE);
}

void UartDriver::update() {
    if (!_initialized) return;

    if (_rxIndex > 0 && _isFrameTimedOut()) {
        Serial.println("[UART DRV] WARN: Trama incompleta (timeout).");
        _resetBuffer();
    }

    while (_serial.available()) {
        char c = (char)_serial.read();

        if (_rxIndex == 0) {
            _frameStartMs = millis();
        }

        if (c == UART_DRV_END_CHAR) {
            if (_rxIndex > 0) {
                _rxBuffer[_rxIndex] = '\0';
                _parseFrame();

                if (_callback != nullptr) {
                    _callback(&_lastFrame);
                }

                Serial.print("[UART DRV] Trama: ");
                Serial.println(_lastFrame.raw);
            }
            _resetBuffer();

        } else if (c == '\r') {
            continue;

        } else {
            if (_rxIndex < (UART_DRV_BUFFER_SIZE - 1)) {
                _rxBuffer[_rxIndex++] = c;
            } else {
                Serial.println("[UART DRV] ERROR: Buffer overflow.");
                _resetBuffer();
            }
        }
    }
}

bool UartDriver::sendCommand(char cmd) {
    if (!_initialized) {
        Serial.println("[UART DRV] ERROR: No inicializado.");
        return false;
    }

    size_t written = _serial.write((uint8_t)cmd);

    if (written == 1) {
        Serial.print("[UART DRV] Enviado a STM32: '");
        Serial.print(cmd);
        Serial.println("'");
        return true;
    }

    Serial.println("[UART DRV] ERROR: Fallo al enviar.");
    return false;
}

void UartDriver::setFrameCallback(UartFrameCallback_t callback) {
    _callback = callback;
    Serial.println("[UART DRV] Callback registrado.");
}

bool UartDriver::isConnected() const {
    return _initialized;
}

const UartFrame_t& UartDriver::getLastFrame() const {
    return _lastFrame;
}

void UartDriver::clearBuffer() {
    _resetBuffer();
    Serial.println("[UART DRV] Buffer limpiado.");
}

void UartDriver::_parseFrame() {
    strncpy(_lastFrame.raw, _rxBuffer, UART_DRV_BUFFER_SIZE - 1);
    _lastFrame.raw[UART_DRV_BUFFER_SIZE - 1] = '\0';
    memset(_lastFrame.value, 0, sizeof(_lastFrame.value));
    _lastFrame.type = UART_FRAME_NONE;

    char* sep = strchr(_rxBuffer, ':');

    if (sep == nullptr) {
        _lastFrame.type = UART_FRAME_INVALID;   // <-- cambiado
        Serial.print("[UART DRV] ERROR: Sin separador ':' en: ");
        Serial.println(_rxBuffer);
        return;
    }

    size_t prefixLen = (size_t)(sep - _rxBuffer);
    char   prefix[16] = {0};
    strncpy(prefix, _rxBuffer, prefixLen);
    prefix[prefixLen] = '\0';

    strncpy(_lastFrame.value, sep + 1, UART_DRV_BUFFER_SIZE - 1);
    _lastFrame.value[UART_DRV_BUFFER_SIZE - 1] = '\0';

    if (strcmp(prefix, "ACK") == 0) {
        _lastFrame.type = UART_FRAME_ACK;
    } else if (strcmp(prefix, "TEMP") == 0) {
        _lastFrame.type = UART_FRAME_TEMP;
    } else {
        _lastFrame.type = UART_FRAME_INVALID;   // <-- cambiado
        Serial.print("[UART DRV] ERROR: Prefijo desconocido: ");
        Serial.println(prefix);
    }
}

void UartDriver::_resetBuffer() {
    memset(_rxBuffer, 0, sizeof(_rxBuffer));
    _rxIndex      = 0;
    _frameStartMs = 0;
}

bool UartDriver::_isFrameTimedOut() const {
    return (millis() - _frameStartMs) > UART_DRV_TIMEOUT_MS;
}