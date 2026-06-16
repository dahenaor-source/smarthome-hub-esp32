\# 📶 Smart Home Hub - ESP32-C6 Bridge



\[!\[ESP32-C6](https://img.shields.io/badge/MCU-ESP32--C6-red.svg)](https://www.espressif.com/en/products/socs/esp32-c6)

\[!\[Framework](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)

\[!\[PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)](https://platformio.org/)

\[!\[License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)



Bridge bidireccional \*\*BLE + WiFi + MQTT\*\* para el sistema Smart Home Hub. Conecta el microcontrolador STM32 L476RG con la nube (Telegram Bot vía MQTT) y con clientes locales (Web App vía Bluetooth Low Energy).



\---



\## 📋 Tabla de contenidos



\- \[Descripción](#-descripción)

\- \[Arquitectura](#-arquitectura)

\- \[Características](#-características)

\- \[Hardware requerido](#-hardware-requerido)

\- \[Conexiones físicas](#-conexiones-físicas)

\- \[Instalación](#-instalación)

\- \[Configuración](#-configuración)

\- \[Estructura del proyecto](#-estructura-del-proyecto)

\- \[Protocolo de comunicación](#-protocolo-de-comunicación)

\- \[Tópicos MQTT](#-tópicos-mqtt)

\- \[Drivers implementados](#-drivers-implementados)

\- \[Uso](#-uso)

\- \[Troubleshooting](#-troubleshooting)

\- \[Roadmap](#-roadmap)

\- \[Licencia](#-licencia)



\---



\## 📖 Descripción



El \*\*ESP32-C6\*\* actúa como gateway entre el mundo IoT (Telegram, MQTT) y el sistema embebido local (STM32 L476RG con FSM). 



Recibe comandos por dos canales independientes:

\- 📡 \*\*MQTT\*\* (vía WiFi) → Para control remoto desde Telegram (mundial)

\- 🔵 \*\*BLE\*\* (Bluetooth Low Energy) → Para control local desde Web App



Y los reenvía al STM32 por UART. También recibe las respuestas del STM32 y las publica en ambos canales.



\---



\## 🏗 Arquitectura



```

┌─────────────────────────────────────────────────────────────┐

│  CAPA USUARIO                                               │

│  📱 Telegram (mundial)        🌐 Web App (local, BLE)       │

└────────────┬───────────────────────────┬─────────────────────┘

&#x20;            │                           │

&#x20;            ▼                           ▼

┌─────────────────────────────────────────────────────────────┐

│  CAPA CLOUD                                                 │

│  🤖 Bot Python (Railway)      ☁️ HiveMQ Broker             │

└────────────┬─────────────────────────────────────────────────┘

&#x20;            │

&#x20;            │ WiFi + MQTT

&#x20;            ▼

┌─────────────────────────────────────────────────────────────┐

│  ESP32-C6 (este proyecto)                                  │

│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │

│  │ WiFi Driver  │  │ MQTT Driver  │  │ BLE Driver   │     │

│  │ (multi-red)  │  │ (PubSubClient│  │ (NimBLE)     │     │

│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘     │

│         │                  │                  │             │

│         └──────────────────┼──────────────────┘             │

│                            │                                │

│                    ┌───────▼────────┐                       │

│                    │  UART Driver   │                       │

│                    └───────┬────────┘                       │

└────────────────────────────┼─────────────────────────────────┘

&#x20;                            │

&#x20;                            │ UART (115200 bps)

&#x20;                            ▼

┌─────────────────────────────────────────────────────────────┐

│  STM32 L476RG (CMSIS o Zephyr)                              │

│  FSM + GPIO + ADC + PWM + UART                              │

└─────────────────────────────────────────────────────────────┘

```



\---



\## ✨ Características



\- ✅ \*\*Multi-protocolo\*\*: BLE + WiFi + MQTT simultáneos

\- ✅ \*\*Multi-red WiFi\*\*: Conexión automática a la primera red disponible

\- ✅ \*\*Reconexión automática\*\*: WiFi y MQTT se reconectan ante pérdida de señal

\- ✅ \*\*Bidireccional\*\*: Recibe comandos y publica respuestas

\- ✅ \*\*Bridge transparente\*\*: BLE ↔ UART y MQTT ↔ UART

\- ✅ \*\*Bajo footprint\*\*: \~500 KB de Flash, suficiente RAM libre

\- ✅ \*\*Credenciales seguras\*\*: Archivo de credenciales excluido de Git



\---



\## 🛠 Hardware requerido



| Componente | Modelo | Notas |

|------------|--------|-------|

| Microcontrolador | nanoESP32-C6 v1.0 | ESP32-C6 con RISC-V |

| Cable USB | USB-A a USB-C | Para alimentación y programación |

| Cables jumper | Hembra-hembra | Para conexión UART al STM32 |



> 💡 También compatible con `ESP32-C6-DevKitC-1` o cualquier placa basada en ESP32-C6.



\---



\## 🔌 Conexiones físicas



\### UART hacia STM32 L476RG



| ESP32-C6 Pin | STM32 Pin | Función |

|--------------|-----------|---------|

| GPIO4 (TX)   | PC11 (RX, USART3) | ESP32 → STM32 |

| GPIO5 (RX)   | PC10 (TX, USART3) | STM32 → ESP32 |

| GND          | GND       | Tierra común |



> ⚠️ \*\*Importante\*\*: Las conexiones TX/RX se cruzan (TX de uno va al RX del otro).



\---



\## 🚀 Instalación



\### Requisitos previos



\- \[PlatformIO](https://platformio.org/install) instalado en VS Code

\- Cuenta en \[GitHub](https://github.com/) (opcional, para clonar)

\- Acceso a red WiFi 2.4 GHz sin captive portal



\### Pasos



1\. \*\*Clonar el repositorio\*\*



```bash

git clone https://github.com/dahenaor-source/smarthome-hub-esp32.git

cd smarthome-hub-esp32

```



2\. \*\*Crear el archivo de credenciales WiFi\*\*



Copia la plantilla y edita con tus datos:



```bash

cp src/wifi\_credentials.example.h src/wifi\_credentials.h

```



3\. \*\*Editar `src/wifi\_credentials.h`\*\* con tus credenciales reales:



```cpp

\#define WIFI\_1\_SSID      "TU\_WIFI\_CASA"

\#define WIFI\_1\_PASSWORD  "tu\_password\_real"



\#define WIFI\_2\_SSID      "TU\_WIFI\_TRABAJO"

\#define WIFI\_2\_PASSWORD  "tu\_password\_real"



\#define WIFI\_3\_SSID      "TU\_HOTSPOT"

\#define WIFI\_3\_PASSWORD  "tu\_password\_real"

```



> ⚠️ \*\*Seguridad\*\*: Este archivo está en `.gitignore`. \*\*NUNCA\*\* lo subas a Git.



4\. \*\*Compilar el firmware\*\*



```bash

pio run

```



5\. \*\*Conectar el ESP32-C6 y subir\*\*



```bash

pio run --target upload

```



6\. \*\*Abrir el monitor serial\*\*



```bash

pio device monitor --baud 115200

```



\---



\## ⚙ Configuración



\### Tópicos MQTT (editables)



Modifica los `#define` en `src/drivers/mqtt\_driver.h`:



```cpp

\#define MQTT\_TOPIC\_CMD      "smarthome/davidhero/cmd"

\#define MQTT\_TOPIC\_ACK      "smarthome/davidhero/ack"

\#define MQTT\_TOPIC\_TEMP     "smarthome/davidhero/temp"

```



> 💡 Reemplaza `davidhero` por un prefijo único tuyo para evitar colisiones en el broker público.



\### Broker MQTT



Por defecto usa \*\*HiveMQ Public Broker\*\*:



```cpp

\#define MQTT\_BROKER         "broker.hivemq.com"

\#define MQTT\_PORT           1883

```



Puedes cambiarlo a otro broker (Mosquitto, EMQX, broker privado, etc.).



\---



\## 📁 Estructura del proyecto



```

esp32c6-ble/

├── platformio.ini                 # Configuración PlatformIO + librerías

├── .gitignore                     # Excluye wifi\_credentials.h y otros

├── README.md                      # Este archivo

└── src/

&#x20;   ├── main.cpp                   # Lógica principal: orquesta drivers

&#x20;   ├── wifi\_credentials.h         # ⚠️ NO subido (secreto)

&#x20;   ├── wifi\_credentials.example.h # Plantilla para nuevos desarrolladores

&#x20;   └── drivers/

&#x20;       ├── ble\_driver.h           # BLE GATT Server

&#x20;       ├── ble\_driver.cpp

&#x20;       ├── uart\_driver.h          # UART hacia STM32

&#x20;       ├── uart\_driver.cpp

&#x20;       ├── wifi\_driver.h          # WiFi cliente multi-red

&#x20;       ├── wifi\_driver.cpp

&#x20;       ├── mqtt\_driver.h          # MQTT cliente (PubSubClient)

&#x20;       └── mqtt\_driver.cpp

```



\---



\## 📡 Protocolo de comunicación



\### UART hacia STM32



\*\*Comandos enviados al STM32 (1 byte ASCII):\*\*



| Byte | Acción |

|------|--------|

| `'N'` | Modo Nocturno |

| `'D'` | Modo Día |

| `'R'` | Modo Relax |

| `'A'` | Modo Alarma |

| `'P'` | Modo Party |

| `'S'` | Modo Standby |

| `'T'` | Solicitar temperatura |



\*\*Tramas recibidas del STM32 (formato `PREFIX:VALUE\\n`):\*\*



```

ACK:NIGHT\\n     → Confirmación de modo activado

ACK:DAY\\n

ACK:RELAX\\n

ACK:ALARM\\n

ACK:PARTY\\n

ACK:STANDBY\\n



TEMP:25.3\\n     → Lectura de temperatura

```



\### BLE GATT



| Característica | UUID | Tipo |

|----------------|------|------|

| Service | `12345678-1234-1234-1234-123456789abc` | Custom |

| Command Write | `12345678-1234-1234-1234-123456789abd` | WRITE |

| Notify | `12345678-1234-1234-1234-123456789abe` | NOTIFY |



\---



\## 🌐 Tópicos MQTT



\### Suscripción (entrada)



```

smarthome/davidhero/cmd

&#x20; └─ Payload: 1 byte ASCII (N, D, R, A, P, S, T)

```



\### Publicación (salida)



```

smarthome/davidhero/ack

&#x20; └─ Payload: Nombre del estado (NIGHT, DAY, RELAX, etc.)



smarthome/davidhero/temp

&#x20; └─ Payload: Temperatura en °C (ej. "25.3")

```



\---



\## 🔧 Drivers implementados



\### 1. `wifi\_driver`



Cliente WiFi con soporte multi-red usando `WiFiMulti`. Permite definir múltiples SSIDs y se conecta automáticamente a la primera red disponible.



\*\*API principal:\*\*



```cpp

void addNetwork(const char\* ssid, const char\* password);

bool begin(uint32\_t timeoutMs = 30000);

void update();  // Llamar en loop()

bool isConnected();

```



\### 2. `mqtt\_driver`



Cliente MQTT basado en `PubSubClient`. Maneja conexión, reconexión automática, suscripción al topic de comandos y publicación de respuestas.



\*\*API principal:\*\*



```cpp

void begin();

void update();  // Llamar en loop()

bool publishAck(const char\* message);

bool publishTemp(const char\* message);

void setCommandCallback(MqttCommandCallback\_t callback);

```



\### 3. `ble\_driver`



Servidor BLE GATT con NimBLE. Implementa servicio personalizado para recibir comandos y enviar notificaciones.



\*\*API principal:\*\*



```cpp

void begin();

void update();  // Llamar en loop()

bool sendNotification(const char\* message);

void setCmdCallback(BleCmdCallback\_t callback);

```



\### 4. `uart\_driver`



Driver UART para comunicación con el STM32. Implementa parsing de tramas con formato `PREFIX:VALUE\\n` y buffer circular.



\*\*API principal:\*\*



```cpp

void begin();

void update();  // Llamar en loop()

bool sendCommand(char cmd);

void setFrameCallback(UartFrameCallback\_t callback);

```



\---



\## 💻 Uso



\### Arranque normal



Al encender el ESP32, verás en el monitor:



```

================================================

&#x20; Smart Home Hub - ESP32-C6 v2.0

&#x20; BLE + WiFi + MQTT

================================================

\[UART DRV] Driver inicializado.

\[BLE DRV] Inicializando...

\[BLE DRV] Listo.

\[BLE DRV] MAC: 20:6e:f1:09:d3:8a

\[WIFI] Red agregada: TU\_WIFI\_1

\[WIFI] Red agregada: TU\_WIFI\_2

\[WIFI] Iniciando conexion...

\[WIFI] Conectado a: TU\_WIFI\_1

\[WIFI] IP: 192.168.1.100

\[WIFI] RSSI: -52 dBm

\[MQTT] Inicializando...

\[MQTT] Configurado

\[MQTT] Intentando conectar al broker... OK

\[MQTT] Suscrito a: smarthome/davidhero/cmd

\[APP] Sistema listo.

```



\### Recibir comando vía MQTT (desde Telegram)



```

\[MQTT] RX \[smarthome/davidhero/cmd]: N

\[APP] MQTT -> STM32: 'N'

\[UART DRV] Enviado a STM32: 'N'

\[UART DRV] Trama: ACK:NIGHT

\[APP] ACK del STM32: NIGHT

\[MQTT] TX \[smarthome/davidhero/ack]: NIGHT

```



\### Recibir comando vía BLE (desde Web App)



```

\[BLE DRV] Cliente conectado.

\[BLE DRV] Comando recibido: 'D'

\[APP] BLE -> STM32: 'D'

\[UART DRV] Enviado a STM32: 'D'

\[UART DRV] Trama: ACK:DAY

\[APP] ACK del STM32: DAY

\[BLE DRV] Notificacion: ACK:DAY

```



\---



\## 🐛 Troubleshooting



\### El ESP32 no se conecta a WiFi



```

\[WIFI] ERROR: Timeout, no se pudo conectar

```



\*\*Soluciones:\*\*

\- Verifica que el SSID y contraseña sean correctos en `wifi\_credentials.h`

\- Confirma que la red sea \*\*2.4 GHz\*\* (el ESP32 también soporta 5GHz pero algunas configuraciones son inestables)

\- Verifica que la red NO tenga \*\*captive portal\*\* (típico en universidades/cafeterías)

\- Acerca el ESP32 al router



\### MQTT no conecta



```

\[MQTT] Intentando conectar al broker... FAIL, rc=-2

```



\*\*Soluciones:\*\*

\- Verifica conexión a internet del ESP32

\- Confirma que el broker esté disponible: `ping broker.hivemq.com`

\- Algunos firewalls bloquean el puerto 1883 (prueba con WebSocket en puerto 8000)



\### BLE no aparece en escaneo



\*\*Soluciones:\*\*

\- Verifica que NimBLE-Arduino esté instalado (`pio lib list`)

\- Reinicia el ESP32 con el botón RESET

\- En el celular, desactiva y reactiva Bluetooth



\### El STM32 no responde



\*\*Soluciones:\*\*

\- Verifica conexiones físicas TX/RX (cruzadas)

\- Confirma que el STM32 esté alimentado

\- Verifica el baudrate (debe ser 115200 en ambos lados)

\- Asegúrate que GND esté conectado entre ambas placas



\### Token expuesto en logs



Si por error compartes el token del bot, regenéralo inmediatamente con `/revoke` en @BotFather.



\---



\## 🗺 Roadmap



\- \[x] Driver UART hacia STM32

\- \[x] Driver BLE (NimBLE GATT Server)

\- \[x] Driver WiFi multi-red

\- \[x] Driver MQTT con PubSubClient

\- \[x] Bridge bidireccional BLE ↔ UART

\- \[x] Bridge bidireccional MQTT ↔ UART

\- \[x] Reconexión automática WiFi/MQTT

\- \[ ] Migración a MQTT con TLS (puerto 8883)

\- \[ ] Autenticación MQTT con username/password

\- \[ ] OTA (Over-The-Air) updates

\- \[ ] Web server local con dashboard

\- \[ ] Modo Access Point para configuración inicial WiFi



\---



\## 🔗 Proyectos relacionados



| Proyecto | Descripción | Repositorio |

|----------|-------------|-------------|

| Bot Telegram | Bot Python para control remoto vía MQTT | \[smarthome-hub-telegram-bot](https://github.com/dahenaor-source/smarthome-hub-telegram-bot) |

| Web App | Interfaz web con BLE local | \[smarthome-hub-web](https://github.com/dahenaor-source/smarthome-hub-web) |

| STM32 (CMSIS) | Firmware bare metal del STM32 | \_Privado\_ |

| STM32 (Zephyr) | Firmware con Zephyr RTOS | \_Privado\_ |



\---



\## 📚 Documentación adicional



\- \[Telegram Bot API](https://core.telegram.org/bots/api)

\- \[MQTT 3.1.1 Spec](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/mqtt-v3.1.1.html)

\- \[NimBLE-Arduino Docs](https://github.com/h2zero/NimBLE-Arduino)

\- \[PubSubClient Docs](https://github.com/knolleary/pubsubclient)

\- \[ESP32-C6 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6\_datasheet\_en.pdf)



\---



\## 👨‍💻 Autor



\*\*David Henao Rojas\*\*



\- GitHub: \[@dahenaor-source](https://github.com/dahenaor-source)

\- Proyecto académico: Estructuras Computacionales



\---



\## 📄 Licencia



Este proyecto está bajo la Licencia MIT. Ver el archivo \[LICENSE](LICENSE) para más detalles.



\---



\## 🙏 Agradecimientos



\- Comunidad de \[PlatformIO](https://platformio.org/)

\- \[Espressif Systems](https://www.espressif.com/) por el ESP32-C6

\- \[HiveMQ](https://www.hivemq.com/) por el broker MQTT público

\- \[Railway.app](https://railway.app/) por el hosting gratuito

\- \[Telegram](https://telegram.org/) por la Bot API gratuita



\---



<div align="center">



⭐ Si te resulta útil, déjale una estrella al repo



\[Reportar Bug](https://github.com/dahenaor-source/smarthome-hub-esp32/issues) · \[Solicitar Feature](https://github.com/dahenaor-source/smarthome-hub-esp32/issues)



</div>

