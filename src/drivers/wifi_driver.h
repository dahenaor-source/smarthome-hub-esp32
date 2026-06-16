/* ============================================================
 * WIFI DRIVER HEADER
 * ESP32-C6 - Smart Home Hub
 * 
 * Cliente WiFi con soporte multi-red.
 * Intenta conectar a múltiples redes hasta encontrar una.
 * ============================================================ */
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

class WifiDriver {
public:
    WifiDriver();
    
    /**
     * @brief Agrega una red WiFi a la lista de intentos
     */
    void addNetwork(const char* ssid, const char* password);
    
    /**
     * @brief Inicia el proceso de conexión WiFi
     * @return true si conecta, false si timeout
     */
    bool begin(uint32_t timeoutMs = 30000);
    
    /**
     * @brief Llamar periódicamente para mantener conexión
     */
    void update();
    
    /**
     * @brief Verifica si está conectado
     */
    bool isConnected() const;
    
    /**
     * @brief Obtiene el SSID actual
     */
    String getCurrentSSID() const;
    
    /**
     * @brief Obtiene la IP local
     */
    String getLocalIP() const;
    
    /**
     * @brief Obtiene la fuerza de la señal (RSSI)
     */
    int getRSSI() const;

private:
    WiFiMulti _wifiMulti;
    bool _wasConnected;
    unsigned long _lastReconnectAttempt;
};

extern WifiDriver* g_wifiDriverInstance;