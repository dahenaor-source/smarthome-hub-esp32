/* ============================================================
 * WIFI DRIVER SOURCE
 * ESP32-C6 - Smart Home Hub
 * ============================================================ */
#include "wifi_driver.h"

WifiDriver* g_wifiDriverInstance = nullptr;

WifiDriver::WifiDriver()
    : _wasConnected(false),
      _lastReconnectAttempt(0)
{
    g_wifiDriverInstance = this;
}

void WifiDriver::addNetwork(const char* ssid, const char* password) {
    _wifiMulti.addAP(ssid, password);
    Serial.print("[WIFI] Red agregada: ");
    Serial.println(ssid);
}

bool WifiDriver::begin(uint32_t timeoutMs) {
    Serial.println("[WIFI] Iniciando conexion...");
    
    WiFi.mode(WIFI_STA);
    
    unsigned long startTime = millis();
    Serial.print("[WIFI] Buscando red disponible");
    
    while ((millis() - startTime) < timeoutMs) {
        if (_wifiMulti.run() == WL_CONNECTED) {
            _wasConnected = true;
            Serial.println();
            Serial.print("[WIFI] Conectado a: ");
            Serial.println(WiFi.SSID());
            Serial.print("[WIFI] IP: ");
            Serial.println(WiFi.localIP());
            Serial.print("[WIFI] RSSI: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
            return true;
        }
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("[WIFI] ERROR: Timeout, no se pudo conectar");
    return false;
}

void WifiDriver::update() {
    bool nowConnected = (WiFi.status() == WL_CONNECTED);
    
    // Detectar desconexion
    if (_wasConnected && !nowConnected) {
        _wasConnected = false;
        Serial.println("[WIFI] WARN: Conexion perdida");
    }
    
    // Intentar reconectar cada 10 segundos
    if (!nowConnected) {
        if (millis() - _lastReconnectAttempt > 10000) {
            _lastReconnectAttempt = millis();
            Serial.println("[WIFI] Intentando reconectar...");
            
            if (_wifiMulti.run() == WL_CONNECTED) {
                _wasConnected = true;
                Serial.print("[WIFI] Reconectado a: ");
                Serial.println(WiFi.SSID());
            }
        }
    }
}

bool WifiDriver::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WifiDriver::getCurrentSSID() const {
    return WiFi.SSID();
}

String WifiDriver::getLocalIP() const {
    return WiFi.localIP().toString();
}

int WifiDriver::getRSSI() const {
    return WiFi.RSSI();
}