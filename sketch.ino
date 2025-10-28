// Mackenzie 2025
// Alerta doméstico de calor

// Esse projeto visa contemplar a ODS-3 fornecendo um
// alerta de temperatura e umidade.

// Quando a temperatura e/ou umidade atingem determinado nível
// o sistema enviará uma mensagem via protocolo MQTT e acionará
// um relé para funcionamento de um umidificador.

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// ------------------ PINOS ------------------
#define DHTPIN       25
#define DHTTYPE      DHT22
#define BUZZER_PIN   16
#define RELAY_PIN     4
#define LED_PIN       2        // LED onboard

// I2C padrão ESP32
#define I2C_SDA_PIN  21
#define I2C_SCL_PIN  22

// ------------------ LIMITES ----------------
const float TEMP_ALTA     = 30.0; // °C
const float HUM_BAIXA_ON  = 45.0; // % liga umidificador
const float HUM_BAIXA_OFF = 50.0; // % desliga (histerese)

#define RELAY_ACTIVE   HIGH
#define RELAY_INACTIVE LOW

// ------------------ OBJETOS ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* MQTT_HOST = "test.mosquitto.org";
const uint16_t MQTT_PORT = 1883;

// Tópicos p/ IoT MQTT Panel
const char* TOPIC_STATE = "vini/esp32/umidade/state"; // JSON com temp, hum, humidificador (retained)
const char* TOPIC_ALERT = "vini/esp32/umidade/alert"; // JSON com motivo
const char* TOPIC_CMD   = "vini/esp32/umidade/cmd";   // payload: "on" / "off"

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ------------------ ESTADO -----------------
bool humidificadorOn = false;
float lastT = NAN, lastH = NAN;
unsigned long lastPublish = 0;

bool prevAlert = false;
unsigned long lastBeep = 0;
const unsigned long BEEP_COOLDOWN = 10; // 10 s

// --------------- FUNÇÕES AUX ---------------
void conectaWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(200);
}

void publica(float t, float h) {
  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"temp\":%.1f,\"hum\":%.1f,\"humidificador\":%s}",
           t, h, humidificadorOn ? "true" : "false");
  // retained = true para o painel receber o último estado ao conectar
  mqtt.publish(TOPIC_STATE, payload, true);
}

void publicaAlerta(const char* motivo, float t, float h) {
  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"motivo\":\"%s\",\"temp\":%.1f,\"hum\":%.1f}",
           motivo, t, h);
  mqtt.publish(TOPIC_ALERT, payload);
}

void onMqttMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim(); msg.toLowerCase();

  if (String(topic) == TOPIC_CMD) {
    if (msg == "on")  { humidificadorOn = true;  digitalWrite(RELAY_PIN, RELAY_ACTIVE); }
    if (msg == "off") { humidificadorOn = false; digitalWrite(RELAY_PIN, RELAY_INACTIVE); }
    // publica feedback imediato
    float t = isnan(lastT)?0:lastT, h = isnan(lastH)?0:lastH;
    publica(t, h);
  }
}

void reconectaMQTT() {
  while (!mqtt.connected()) {
    String id = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe(TOPIC_CMD);
      mqtt.publish(TOPIC_STATE, "boot", true); // opcional
    } else {
      delay(1000);
    }
  }
}

void atualizaLCD(float t, float h) {
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t, 1);
  lcd.print((char)223); // °
  lcd.print("C  H:");
  lcd.print(h, 1);
  lcd.print("%  ");

  lcd.setCursor(0, 1);
  lcd.print("UMIDIFIC:");
  lcd.print(humidificadorOn ? "ON " : "OFF");
  lcd.print(" ");
  if (t > TEMP_ALTA) lcd.print("ALTA");
  else lcd.print("    ");
}

// ------------------ SETUP ------------------
void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init(); lcd.backlight(); lcd.clear();

  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT); noTone(BUZZER_PIN); digitalWrite(BUZZER_PIN, LOW);
  pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);
  pinMode(RELAY_PIN, OUTPUT); digitalWrite(RELAY_PIN, RELAY_INACTIVE);

  // Beep de boot
  tone(BUZZER_PIN, 2000, 120);

  conectaWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);
  reconectaMQTT();
}

// ------------------- LOOP ------------------
void loop() {
  if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) reconectaMQTT();
  mqtt.loop();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    lcd.setCursor(0,0); lcd.print("Erro DHT22       ");
    lcd.setCursor(0,1); lcd.print("Verifique fios   ");
    delay(800);
    return;
  }

  lastT = t; lastH = h;

  // ---- Lógica do umidificador (histerese) ----
  bool deveLigar = (h < HUM_BAIXA_ON) || (humidificadorOn && h < HUM_BAIXA_OFF);
  if (deveLigar != humidificadorOn) {
    humidificadorOn = deveLigar;
    digitalWrite(RELAY_PIN, humidificadorOn ? RELAY_ACTIVE : RELAY_INACTIVE);
    publica(t, h); // feedback imediato
  }

  // ---- Alertas (LED + beep) ----
  bool alert = (t > TEMP_ALTA) || (h < HUM_BAIXA_ON);
  digitalWrite(LED_PIN, alert ? HIGH : LOW);
  unsigned long now = millis();
  if (alert && (!prevAlert || (now - lastBeep > BEEP_COOLDOWN))) {
    tone(BUZZER_PIN, 2500, 150);
    if (t > TEMP_ALTA) publicaAlerta("temperatura-alta", t, h);
    if (h < HUM_BAIXA_ON) publicaAlerta("umidade-baixa", t, h);
    lastBeep = now;
  }
  if (!alert) { noTone(BUZZER_PIN); digitalWrite(BUZZER_PIN, LOW); }
  prevAlert = alert;

  // ---- LCD ----
  atualizaLCD(t, h);

  // Telemetria periódica (retained atualiza o último valor)
  if (millis() - lastPublish > 10000) {
    publica(t, h);
    lastPublish = millis();
  }

  delay(300);
}
