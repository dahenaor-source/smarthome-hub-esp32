/* ============================================================
 * MQTT DRIVER HEADER
 * ESP32-C6 - Smart Home Hub
 * 
 * Cliente MQTT que se conecta a HiveMQ broker.
 * Se suscribe a comandos y publica ACK/TEMP.
 * ============================================================ */
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configuracion del broker (igual al bot Python)
#define MQTT_BROKER         "broker.hivemq.com"
#define MQTT_PORT           1883
#define MQTT_CLIENT_ID      "smarthome-esp32-davidhero"

// Topics (deben coincidir con .env del bot)
#define MQTT_TOPIC_CMD      "smarthome/davidhero/cmd"
#define MQTT_TOPIC_ACK      "smarthome/davidhero/ack"
#define MQTT_TOPIC_TEMP     "smarthome/davidhero/temp"

// Callback para comandos recibidos
typedef void (*MqttCommandCallback_t)(char command);

class MqttDriver {
public:
    MqttDriver();
    
    /**
     * @brief Inicializa el cliente MQTT
     */
    void begin();
    
    /**
     * @brief Llamar continuamente desde loop()
     * Maneja reconexion y procesa mensajes
     */
    void update();
    
    /**
     * @brief Publica un ACK
     */
    bool publishAck(const char* message);
    
    /**
     * @brief Publica una temperatura
     */
    bool publishTemp(const char* message);
    
    /**
     * @brief Publica en un topic generico
     */
    bool publish(const char* topic, const char* payload);
    
    /**
     * @brief Verifica si esta conectado al broker
     */
    bool isConnected();
    
    /**
     * @brief Registra callback para comandos recibidos
     */
    void setCommandCallback(MqttCommandCallback_t callback);

private:
    WiFiClient      _wifiClient;
    PubSubClient    _mqttClient;
    MqttCommandCallback_t _cmdCallback;
    unsigned long   _lastReconnectAttempt;
    
    bool _reconnect();
    static void _staticCallback(char* topic, byte* payload, unsigned int length);
};

extern MqttDriver* g_mqttDriverInstance;