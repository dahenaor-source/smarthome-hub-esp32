/* ============================================================
 * MAIN.CPP - SMART HOME HUB - ESP32-C6
 * Versión 2.0 - Con WiFi + MQTT + BLE
 * 
 * Arquitectura:
 *   Telegram → Bot Railway → MQTT → ESP32 → UART → STM32
 *   Web App  → BLE         → ESP32 → UART → STM32
 *   STM32    → UART        → ESP32 → MQTT/BLE → cliente
 * ============================================================ */
#include <Arduino.h>
#include "drivers/ble_driver.h"
#include "drivers/uart_driver.h"
#include "drivers/wifi_driver.h"
#include "drivers/mqtt_driver.h"
#include "wifi_credentials.h"

/* ============================================================
 * INSTANCIAS DE DRIVERS
 * ============================================================ */
BleDriver  bleDriver;
UartDriver uartDriver(Serial1);
WifiDriver wifiDriver;
MqttDriver mqttDriver;

/* ============================================================
 * CALLBACK: Comando BLE -> UART al STM32
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
 * CALLBACK: Comando MQTT -> UART al STM32
 * ============================================================ */
void onMqttCommandReceived(char command) {
    Serial.print("[APP] MQTT -> STM32: '");
    Serial.print(command);
    Serial.println("'");
    
    bool ok = uartDriver.sendCommand(command);
    if (!ok) {
        mqttDriver.publishAck("ERROR:UART_FAIL");
    }
}

/* ============================================================
 * CALLBACK: Trama UART desde STM32
 * Reenvía por BLE Y por MQTT
 * ============================================================ */
void onUartFrameReceived(const UartFrame_t* frame) {
    switch (frame->type) {

        case UART_FRAME_ACK:
            Serial.print("[APP] ACK del STM32: ");
            Serial.println(frame->value);
            // Reenviar por BLE
            bleDriver.sendNotification(frame->raw);
            // Reenviar por MQTT
            mqttDriver.publishAck(frame->value);
            break;

        case UART_FRAME_TEMP:
            Serial.print("[APP] TEMP del STM32: ");
            Serial.print(frame->value);
            Serial.println(" C");
            // Reenviar por BLE
            bleDriver.sendNotification(frame->raw);
            // Reenviar por MQTT
            mqttDriver.publishTemp(frame->value);
            break;

        case UART_FRAME_INVALID:
            Serial.print("[APP] Trama invalida: ");
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
    Serial.println("================================================");
    Serial.println("  Smart Home Hub - ESP32-C6 v2.0");
    Serial.println("  BLE + WiFi + MQTT");
    Serial.println("================================================");

    // 1. Inicializar UART (al STM32)
    uartDriver.begin();
    uartDriver.setFrameCallback(onUartFrameReceived);

    // 2. Inicializar BLE
    bleDriver.begin();
    bleDriver.setCmdCallback(onBleCommandReceived);

    // 3. Configurar redes WiFi
    wifiDriver.addNetwork(WIFI_1_SSID, WIFI_1_PASSWORD);
    wifiDriver.addNetwork(WIFI_2_SSID, WIFI_2_PASSWORD);
    wifiDriver.addNetwork(WIFI_3_SSID, WIFI_3_PASSWORD);

    // 4. Conectar WiFi
    bool wifiOk = wifiDriver.begin(30000);  // 30 segundos timeout

    if (wifiOk) {
        // 5. Inicializar MQTT (solo si hay WiFi)
        mqttDriver.begin();
        mqttDriver.setCommandCallback(onMqttCommandReceived);
    } else {
        Serial.println("[APP] WARN: Sin WiFi, MQTT no disponible");
        Serial.println("[APP] BLE seguira funcionando como respaldo");
    }

    Serial.println("[APP] Sistema listo.");
    Serial.println();
}

/* ============================================================
 * LOOP
 * ============================================================ */
void loop() {
    // Procesar UART (siempre)
    uartDriver.update();
    
    // Procesar BLE (siempre)
    bleDriver.update();
    
    // Procesar WiFi y MQTT (si hay WiFi)
    wifiDriver.update();
    
    if (wifiDriver.isConnected()) {
        mqttDriver.update();
    }
    
    delay(10);
}