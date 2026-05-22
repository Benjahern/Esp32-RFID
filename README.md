# ESP32 RFID - Control de Asistencia

Sistema lector RFID conectado al backend de Turnos para registrar entrada/salida de trabajadores.

## Hardware

### Conexiones ESP32 ↔ MFRC522

| MFRC522 | ESP32 (DOIT DevKit) |
|---------|---------------------|
| SDA (CS) | GPIO 5 (D5) |
| SCK | GPIO 18 (D18) |
| MOSI | GPIO 23 (D23) |
| MISO | GPIO 19 (D19) |
| RST | GPIO 21 (D21) |
| 3.3V | 3.3V |
| GND | GND |

> **Nota:** El MFRC522 funciona a 3.3V. Conectar a 5V puede dañar el módulo.

### Materiales
- ESP32 DevKit v1 (o compatible)
- MFRC522 + tarjeta/llavero RFID (13.56MHz)
- Cables dupont
- (Opcional) Protoboard

## Configuración

### 1. Variables en `lib/config.h`

```cpp
// WiFi - Credenciales de tu red
#define WIFI_SSID "Tu_SSID"
#define WIFI_PASSWORD "TuPassword"

// Backend API - IP del servidor donde corre Turnos
#define API_HOST "192.168.1.133"  // IP de tu máquina
#define API_PORT 8080
#define API_BASE_PATH "/api/v1/attendance/rfid"

// Empresa - UUID de la empresa en el sistema Turnos
#define API_COMPANY "f9e94890-9f0f-4b71-8b7c-70b2a6b90172"
```

### 2. Pines en `lib/includes.h` (si usas otros pines)

```cpp
#define SS_PIN 5    // GPIO del pin SDA
#define RST_PIN 21  // GPIO del pin RST
```

### 3. Cómo obtener el `API_COMPANY`

1. Inicia sesión en el dashboard de Turnos como admin
2. Ve a Configuración > Información de Empresa
3. Copia el **Company ID** (UUID)

## Build y Upload

```bash
cd ~/Documents/PlatformIO/Projects/Esp32_Rfid

# Compilar
pio run

# Subir al ESP32
pio run --target upload

# Abrir monitor serie (115200 baud)
pio device monitor
```

## Cómo funciona

1. El ESP32 se conecta al WiFi
2. Al aproximar una tarjeta RFID al lector, lee el UID
3. Envía el UID al backend: `POST /api/v1/attendance/rfid`
4. El servidor alterna automáticamente:
   - Sin registros hoy → **checkin**
   - Último registro fue checkin → **checkout**
   - Último registro fue checkout → **checkin**
5. El ESP32 recibe la respuesta y la muestra en el monitor serie

## Parámetros ajustables

En `lib/config.h`:

| Variable | Default | Descripción |
|----------|---------|-------------|
| `RFID_POLL_INTERVAL` | 200ms | Intervalo de lectura del RFID |
| `READER_RESET_INTERVAL` | 60000ms | Cada cuánto reiniciar el lector |
| `UID_DEBOUNCE_MS` | 5000ms | Tiempo mínimo entre lecturas del mismo UID |

## Solución de problemas

### "DNS Failed" o "TCP FALLO"
- Verifica que `API_HOST` tenga la IP correcta (formato `192.168.x.x`, no `192.268`)
- Verifica que el firewall permita conexiones al puerto 8080

### Verificar conexión desde otro equipo
```bash
curl -X POST http://192.168.1.133:8080/api/v1/attendance/rfid \
  -H "Content-Type: application/json" \
  -H "X-API-Key: TU_COMPANY_UUID" \
  -d '{"rfid_uuid":"da:cf:a:35"}'
```

### El lector no detecta tarjetas
- Verifica las conexiones SPI (SDA, SCK, MOSI, MISO)
- Verifica que la tarjeta RFID sea de 13.56MHz (compatible con MFRC522)
- Asegúrate de que la antena del MFRC522 esté correctamente soldada

### Solo marca checkin siempre
- El sistema ya tiene registros de checkin+checkout hoy
- Espera hasta mañana o borra los registros desde el dashboard

## Dashboard

Accede al frontend para ver los registros de asistencia:
```
http://192.168.1.133:3000
```