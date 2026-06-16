/* ============================================================
 * MAIN.CPP - SMART HOME HUB - ESP32-C6
 * 
 * Puente entre BLE (cliente móvil/web) y UART (STM32 L476RG)
 * 
 * Flujo:
 *   1. BLE recibe comando (1 char) → reenvía por UART al STM32
 *   2. UART recibe trama (ACK:xxx / TEMP:xxx) → reenvía por BLE
 * 
 * Conexiones físicas:
 *   ESP32 GPIO4 (TX) ──→ STM32 PC11 (RX)
 *   ESP32 GPIO5 (RX) ←── STM32 PC10 (TX)
 *   ESP32 GND        ─── STM32 GND
 * ============================================================ */
#include <Arduino.h>
#include "drivers/ble_driver.h"
#include "drivers/uart_driver.h"

/* ============================================================
 * INSTANCIAS DE LOS DRIVERS
 * ============================================================ */
BleDriver bleDriver;

// IMPORTANTE: usar referencia a Serial1 (ya predefinido en Arduino)
// en lugar de HardwareSerial(1)
UartDriver uartDriver(Serial1);

/* ============================================================
 * CALLBACK - Comando BLE recibido -> reenviar por UART
 * ============================================================ */
void onBleCommandReceived(char command) {
    Serial.print("[APP] BLE -> STM32: '");
    Serial.print(command);
    Serial.println("'");

    bool ok = uartDriver.sendCommand(command);
    if (!ok) {
        bleDriver.sendNotification("ERROR:UART_FAIL");
    }
}

/* ============================================================
 * CALLBACK - Trama UART recibida -> reenviar por BLE
 * ============================================================ */
void onUartFrameReceived(const UartFrame_t* frame) {
    switch (frame->type) {

        case UART_FRAME_ACK:
            Serial.print("[APP] ACK recibido: ");
            Serial.println(frame->value);
            bleDriver.sendNotification(frame->raw);
            break;

        case UART_FRAME_TEMP:
            Serial.print("[APP] TEMP recibida: ");
            Serial.print(frame->value);
            Serial.println(" C");
            bleDriver.sendNotification(frame->raw);
            break;

        case UART_FRAME_INVALID:
            Serial.print("[APP] Trama con error: ");
            Serial.println(frame->raw);
            bleDriver.sendNotification("ERROR:BAD_FRAME");
            break;

        default:
            break;
    }
}

/* ============================================================
 * SETUP
 * ============================================================ */
void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("================================");
    Serial.println("  Smart Home Hub - ESP32-C6");
    Serial.println("================================");

    // Inicializar UART hacia STM32
    uartDriver.begin();
    uartDriver.setFrameCallback(onUartFrameReceived);

    // Inicializar BLE
    bleDriver.begin();
    bleDriver.setCmdCallback(onBleCommandReceived);

    Serial.println("[APP] Sistema listo.");
    Serial.println();
}

/* ============================================================
 * LOOP
 * ============================================================ */
void loop() {
    uartDriver.update();
    bleDriver.update();
    delay(10);
}