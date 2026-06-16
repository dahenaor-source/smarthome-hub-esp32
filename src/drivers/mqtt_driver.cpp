/* ============================================================
 * MQTT DRIVER SOURCE
 * ESP32-C6 - Smart Home Hub
 * ============================================================ */
#include "mqtt_driver.h"

MqttDriver* g_mqttDriverInstance = nullptr;

MqttDriver::MqttDriver()
    : _mqttClient(_wifiClient),
      _cmdCallback(nullptr),
      _lastReconnectAttempt(0)
{
    g_mqttDriverInstance = this;
}

void MqttDriver::begin() {
    Serial.println("[MQTT] Inicializando...");
    Serial.print("[MQTT] Broker: ");
    Serial.print(MQTT_BROKER);
    Serial.print(":");
    Serial.println(MQTT_PORT);
    
    _mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    _mqttClient.setCallback(_staticCallback);
    _mqttClient.setBufferSize(256);
    
    Serial.println("[MQTT] Configurado");
}

void MqttDriver::update() {
    // Solo intenta conectar si WiFi esta listo
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    
    // Si no esta conectado al broker, reconectar
    if (!_mqttClient.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = now;
            if (_reconnect()) {
                _lastReconnectAttempt = 0;
            }
        }
    } else {
        // Procesar mensajes pendientes
        _mqttClient.loop();
    }
}

bool MqttDriver::_reconnect() {
    Serial.print("[MQTT] Intentando conectar al broker... ");
    
    if (_mqttClient.connect(MQTT_CLIENT_ID)) {
        Serial.println("OK");
        
        // Suscribirse al topic de comandos
        if (_mqttClient.subscribe(MQTT_TOPIC_CMD, 1)) {
            Serial.print("[MQTT] Suscrito a: ");
            Serial.println(MQTT_TOPIC_CMD);
        } else {
            Serial.println("[MQTT] ERROR: No se pudo suscribir");
        }
        
        return true;
    } else {
        Serial.print("FAIL, rc=");
        Serial.println(_mqttClient.state());
        return false;
    }
}

void MqttDriver::_staticCallback(char* topic, byte* payload, unsigned int length) {
    if (g_mqttDriverInstance == nullptr) return;
    
    // Convertir payload a string
    char message[64];
    unsigned int len = (length < 63) ? length : 63;
    memcpy(message, payload, len);
    message[len] = '\0';
    
    Serial.print("[MQTT] RX [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);
    
    // Si es el topic de comandos y tenemos callback
    if (strcmp(topic, MQTT_TOPIC_CMD) == 0 && length > 0) {
        if (g_mqttDriverInstance->_cmdCallback != nullptr) {
            g_mqttDriverInstance->_cmdCallback(message[0]);
        }
    }
}

bool MqttDriver::publishAck(const char* message) {
    return publish(MQTT_TOPIC_ACK, message);
}

bool MqttDriver::publishTemp(const char* message) {
    return publish(MQTT_TOPIC_TEMP, message);
}

bool MqttDriver::publish(const char* topic, const char* payload) {
    if (!isConnected()) {
        Serial.println("[MQTT] WARN: No conectado, no se puede publicar");
        return false;
    }
    
    bool result = _mqttClient.publish(topic, payload);
    
    if (result) {
        Serial.print("[MQTT] TX [");
        Serial.print(topic);
        Serial.print("]: ");
        Serial.println(payload);
    } else {
        Serial.println("[MQTT] ERROR: Publish fallo");
    }
    
    return result;
}

bool MqttDriver::isConnected() {
    return _mqttClient.connected();
}

void MqttDriver::setCommandCallback(MqttCommandCallback_t callback) {
    _cmdCallback = callback;
    Serial.println("[MQTT] Callback registrado");
}