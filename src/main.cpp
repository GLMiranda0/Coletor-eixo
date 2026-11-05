#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NetBIOS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <esp_mac.h>
#include <sys/time.h>
#include "webui.h"

// LED azul onboard da placa (marcado "D2")
constexpr uint8_t LED_PIN = 2;

// Botao BOOT da placa, com o coletor rodando:
//   toque curto   -> abre o portal de configuracao (AP) sem apagar o Wi-Fi
//   segurar 5 s   -> apaga o Wi-Fi salvo e reabre o portal
constexpr uint8_t RESET_BUTTON_PIN = 0;
constexpr uint32_t PORTAL_PRESS_MAX_MS = 2000;
constexpr uint32_t WIFI_RESET_HOLD_MS = 5000;

// Linha de dados do DS18B20 (marcado "D4"); exige resistor de 4,7k entre DATA e 3V3
constexpr uint8_t ONE_WIRE_PIN = 4;
constexpr uint32_t READ_INTERVAL_MS = 2000;
constexpr uint8_t MAX_TEMP_SENSORS = 8;

// --- Sensores de corrente SCT-013-005 (5 A -> 1 V RMS, resistor de carga interno) ---
// Cada sensor: um fio no pino ADC, o outro no ponto de polarizacao de 1,65 V (divisor 10k/10k + 10uF).
// So pinos do ADC1: o ADC2 nao funciona com Wi-Fi ligado.
struct CurrentSensor {
  const char *name;
  uint8_t pin;
  float ampsPerVolt;  // 5.0 nominal; ajuste fino comparando com um alicate amperimetro
};

CurrentSensor currentSensors[] = {
    {"CT1", 34, 5.0f},  // D34
    {"CT2", 35, 5.0f},  // D35
    {"CT3", 32, 5.0f},  // D32
};
constexpr size_t CT_COUNT = sizeof(currentSensors) / sizeof(currentSensors[0]);

// 200 ms = 12 ciclos completos em 60 Hz (e 10 em 50 Hz)
constexpr uint32_t CT_WINDOW_US = 200000;
// Abaixo disso a leitura e ruido do ADC do ESP32; mostra 0
constexpr float CT_NOISE_FLOOR_A = 0.05f;

// --- Acelerometro ADXL345, modulo GY-291 (I2C: SDA no D21, SCL no D22, VCC 3V3, CS em 3V3) ---
// Endereco depende do pino SDO: GND (padrao do GY-291) = 0x53, 3V3 = 0x1D. Os dois sao testados.
constexpr uint8_t ADXL_ADDRS[] = {0x53, 0x1D};
constexpr uint8_t ADXL_DEVID = 0xE5;
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr int VIB_CAL_SAMPLES = 200;
constexpr int VIB_MAX_BATCH = 2000;
// Passa-altas da velocidade: tira a deriva da integracao. A ISO 10816 mede de 10 a 1000 Hz, mas
// um filtro de 1a ordem em 10 Hz ja corta ~10% em 29,5 Hz (1x de motor de 4 polos); em 5 Hz o erro
// fica em -3% a 30 Hz e +4% a 100 Hz, com ~0,2 mm/s de piso de ruido. O limite de cima e a metade
// da taxa de amostragem (250 Hz com 2 ms).
constexpr float VIB_HP_HZ = 5.0f;
constexpr float GRAVITY_MS2 = 9.80665f;

// Valores padrao da coleta de vibracao (ajustaveis pela interface web)
constexpr int DEF_VIB_BATCH = 500;     // amostras por janela
constexpr int DEF_VIB_DELAY_MS = 2;    // 2 ms = 500 Hz -> janela de 1 s
constexpr float DEF_VIB_ALPHA = 0.2f;  // K do filtro EMA
constexpr uint32_t DEF_POST_INTERVAL_S = 10;

// Nome na rede: http://coletor.local (mDNS), http://coletor no Windows (NetBIOS) e hostname no DHCP.
// Com mais de um coletor na mesma rede, de um nome diferente a cada um.
const char *const DEF_NAME = "coletor";
constexpr size_t NAME_MAX_LEN = 15;  // limite do NetBIOS

constexpr uint32_t PORTAL_TIMEOUT_S = 180;
constexpr uint32_t NTP_WAIT_MS = 10000;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

Preferences prefs;
WebServer server(80);
WiFiManager wm;

// Derivado do MAC, unico por placa: "coletor-XXXXXX" -> nome do AP de configuracao e device_id no POST
char apName[24];

// Configuracao persistida em flash (namespace "coletor")
struct Config {
  String name;     // nome na rede (ver DEF_NAME)
  String postUrl;  // vazio = envio desligado
  String token;    // token do dispositivo, gerado na tela do motor no Eixo -> p_token
  String apiKey;   // chave publicavel do Supabase -> header "apikey"
  uint32_t postIntervalS;
  int vibBatch;
  int vibDelayMs;
  float vibAlpha;
} cfg;

// Ultimas leituras, servidas pela interface web e enviadas no POST
struct Vibration {
  bool valid;
  int16_t x, y, z;  // ultima amostra bruta
  float ema, rms, peak, stdDev;  // normalizados pela gravidade (repouso ~ 1.0)
  float velAxes[3];  // velocidade RMS por eixo (X, Y, Z), mm/s, banda de VIB_HP_HZ ate Nyquist
  float vel;         // maior dos tres eixos (criterio da ISO 10816) -> "vib" no Eixo
};

struct Readings {
  float temps[MAX_TEMP_SENSORS];
  uint8_t tempCount;
  float amps[CT_COUNT];
  Vibration vib;
  int64_t tsMs;  // epoch em ms; 0 se o NTP ainda nao sincronizou
  bool valid;
} last = {};

bool accelOk = false;
float gravMagnitude = 0;  // magnitude de repouso em LSB; 0 = nao calibrado
float vibEma = 0;

struct PostStatus {
  int code;       // codigo HTTP, negativo = erro de conexao
  bool ok;
  uint32_t atMs;  // millis() do ultimo envio
} postStatus = {0, false, 0};

volatile bool reqCal = false;
volatile bool reqPostNow = false;
volatile bool reqRestart = false;
volatile bool reqWifiReset = false;
volatile bool reqPortal = false;

// ── Configuracao (Preferences) ───────────────────────────────────────────────

void loadConfig() {
  prefs.begin("coletor", true);
  cfg.name = prefs.getString("name", DEF_NAME);
  cfg.postUrl = prefs.getString("post_url", "");
  cfg.token = prefs.getString("token", "");
  cfg.apiKey = prefs.getString("api_key", "");
  cfg.postIntervalS = prefs.getUInt("post_int", DEF_POST_INTERVAL_S);
  cfg.vibBatch = prefs.getInt("vib_batch", DEF_VIB_BATCH);
  cfg.vibDelayMs = prefs.getInt("vib_delay", DEF_VIB_DELAY_MS);
  cfg.vibAlpha = prefs.getFloat("vib_alpha", DEF_VIB_ALPHA);
  gravMagnitude = prefs.getFloat("grav_adxl", 0);
  prefs.end();
}

void saveConfig() {
  prefs.begin("coletor", false);
  prefs.putString("name", cfg.name);
  prefs.putString("post_url", cfg.postUrl);
  prefs.putString("token", cfg.token);
  prefs.putString("api_key", cfg.apiKey);
  prefs.putUInt("post_int", cfg.postIntervalS);
  prefs.putInt("vib_batch", cfg.vibBatch);
  prefs.putInt("vib_delay", cfg.vibDelayMs);
  prefs.putFloat("vib_alpha", cfg.vibAlpha);
  prefs.end();
}

// Aceita URL vazia (envio desligado); sem esquema assume http://. Falso se o esquema nao for http(s).
bool normalizeUrl(String &url) {
  url.trim();
  if (url.isEmpty()) return true;
  if (url.indexOf("://") < 0) url = "http://" + url;
  return url.startsWith("http://") || url.startsWith("https://");
}

// Nome valido de host: minusculas, digitos e hifen ("Linha 1" -> "linha-1"). Falso se ficar vazio.
bool normalizeName(String &name) {
  name.trim();
  name.toLowerCase();
  String out;
  for (char c : name) {
    bool valid = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    if (valid) {
      out += c;
    } else if (!out.isEmpty() && !out.endsWith("-")) {
      out += '-';
    }
  }
  if (out.length() > NAME_MAX_LEN) out.remove(NAME_MAX_LEN);
  while (out.endsWith("-")) out.remove(out.length() - 1);
  name = out;
  return !name.isEmpty();
}

// ── Tarefas de fundo ─────────────────────────────────────────────────────────

void checkBootButton() {
  static uint32_t pressedSince = 0;
  bool pressed = digitalRead(RESET_BUTTON_PIN) == LOW;
  if (pressed && pressedSince == 0) {
    pressedSince = millis();
  } else if (pressed && millis() - pressedSince > WIFI_RESET_HOLD_MS) {
    reqWifiReset = true;
  } else if (!pressed && pressedSince != 0) {
    uint32_t held = millis() - pressedSince;
    if (held >= 50 && held < PORTAL_PRESS_MAX_MS) reqPortal = true;
    pressedSince = 0;
  }
}

// Atende a interface web e o botao; chamado em toda espera para nao travar o servidor
void serviceTasks() {
  server.handleClient();
  checkBootButton();
}

void waitServicing(uint32_t ms) {
  uint32_t start = millis();
  while (millis() - start < ms) {
    serviceTasks();
    delay(2);
  }
}

// ── DS18B20 ──────────────────────────────────────────────────────────────────

void printAddress(const DeviceAddress addr) {
  for (uint8_t i = 0; i < 8; i++) {
    Serial.printf("%02X", addr[i]);
  }
}

// ── SCT-013 ──────────────────────────────────────────────────────────────────

// Amostra os sensores intercalados durante a janela e calcula a corrente RMS de cada um.
// O nivel DC (polarizacao de ~1,65 V) e removido pela media, entao nao precisa ser exato.
void measureCurrents(float ampsRms[CT_COUNT]) {
  double sum[CT_COUNT] = {};
  double sumSq[CT_COUNT] = {};
  uint32_t samples = 0;

  uint32_t start = micros();
  while (micros() - start < CT_WINDOW_US) {
    for (size_t i = 0; i < CT_COUNT; i++) {
      double mv = analogReadMilliVolts(currentSensors[i].pin);
      sum[i] += mv;
      sumSq[i] += mv * mv;
    }
    samples++;
  }

  for (size_t i = 0; i < CT_COUNT; i++) {
    double mean = sum[i] / samples;
    double variance = sumSq[i] / samples - mean * mean;
    double vRms = variance > 0 ? sqrt(variance) / 1000.0 : 0.0;
    float amps = vRms * currentSensors[i].ampsPerVolt;
    ampsRms[i] = amps < CT_NOISE_FLOOR_A ? 0.0f : amps;
  }
}

// ── ADXL345 ──────────────────────────────────────────────────────────────────

uint8_t accelAddr = 0;

bool accelWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(accelAddr);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool accelReadRegs(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(addr, len) != len) {
    return false;
  }
  Wire.readBytes(buf, len);
  return true;
}

bool accelBegin() {
  accelAddr = 0;
  for (uint8_t addr : ADXL_ADDRS) {
    uint8_t devId;
    if (accelReadRegs(addr, 0x00, &devId, 1) && devId == ADXL_DEVID) {  // DEVID
      accelAddr = addr;
      break;
    }
  }
  if (accelAddr == 0) return false;

  bool ok = accelWrite(0x2D, 0x00)     // POWER_CTL: standby enquanto configura
            && accelWrite(0x31, 0x0B)  // DATA_FORMAT: resolucao total (256 LSB/g) em +-16 g
            && accelWrite(0x2C, 0x0D)  // BW_RATE: 800 Hz de saida, banda de 400 Hz
            && accelWrite(0x38, 0x00)  // FIFO_CTL: bypass, sempre a amostra mais recente
            && accelWrite(0x2D, 0x08); // POWER_CTL: modo medicao
  delay(20);
  Serial.printf("ADXL345 em 0x%02X%s\n", accelAddr, ok ? "" : " - falha ao configurar");
  return ok;
}

bool readAccel(int16_t &ax, int16_t &ay, int16_t &az) {
  uint8_t buf[6];
  if (!accelReadRegs(accelAddr, 0x32, buf, 6)) return false;  // DATAX0..DATAZ1, little-endian
  ax = (int16_t)(buf[1] << 8 | buf[0]);
  ay = (int16_t)(buf[3] << 8 | buf[2]);
  az = (int16_t)(buf[5] << 8 | buf[4]);
  return true;
}

float magnitude(int16_t x, int16_t y, int16_t z) {
  return sqrtf((float)x * x + (float)y * y + (float)z * z);
}

// Mede a magnitude media em repouso; todas as leituras sao divididas por ela (repouso ~ 1.0).
// Fica salva em flash, entao so precisa refazer se o sensor mudar de posicao/placa.
void calibrateGravity() {
  if (!accelOk) {
    Serial.println("Calibracao: ADXL345 nao encontrado");
    return;
  }
  Serial.printf("Calibrando com %d amostras - mantenha o sensor parado...\n", VIB_CAL_SAMPLES);
  double sum = 0;
  int n = 0;
  for (int i = 0; i < VIB_CAL_SAMPLES; i++) {
    int16_t ax, ay, az;
    if (readAccel(ax, ay, az)) {
      sum += magnitude(ax, ay, az);
      n++;
    }
    delay(5);
  }
  if (n < VIB_CAL_SAMPLES / 2) {
    Serial.println("Calibracao falhou: erro de leitura no I2C");
    return;
  }
  gravMagnitude = sum / n;
  vibEma = 0;
  prefs.begin("coletor", false);
  prefs.putFloat("grav_adxl", gravMagnitude);
  prefs.end();
  Serial.printf("Magnitude de repouso: %.1f LSB\n", gravMagnitude);
}

// Amostras brutas da ultima janela, usadas para integrar a velocidade
int16_t vibSamples[VIB_MAX_BATCH][3];

// Velocidade de vibracao RMS (mm/s) por eixo, a grandeza da ISO 10816-3 que o Eixo usa:
// tira a media (gravidade) da aceleracao, integra e passa por um passa-altas de VIB_HP_HZ.
// O inicio da janela (3 constantes de tempo do filtro) fica fora da conta.
void computeVelocity(int n, float dt, Vibration &vib) {
  vib.vel = 0;
  for (float &v : vib.velAxes) v = 0;
  if (n < 10 || dt <= 0) return;

  const float rc = 1.0f / (2.0f * PI * VIB_HP_HZ);
  const float alpha = rc / (rc + dt);
  const int skip = min(n / 2, (int)ceilf(3.0f * rc / dt));
  const float lsbToMs2 = GRAVITY_MS2 / gravMagnitude;

  for (int axis = 0; axis < 3; axis++) {
    double mean = 0;
    for (int i = 0; i < n; i++) mean += vibSamples[i][axis];
    mean /= n;

    float vel = 0, prevVel = 0, hp = 0;
    double sumSq = 0;
    for (int i = 0; i < n; i++) {
      vel += (vibSamples[i][axis] - mean) * lsbToMs2 * dt;  // m/s
      hp = alpha * (hp + vel - prevVel);
      prevVel = vel;
      if (i >= skip) sumSq += (double)hp * hp;
    }
    vib.velAxes[axis] = sqrt(sumSq / (n - skip)) * 1000.0;  // mm/s
    vib.vel = max(vib.vel, vib.velAxes[axis]);
  }
}

// Coleta uma janela de amostras e extrai EMA, RMS, pico e desvio padrao da magnitude normalizada.
// Atende a interface web entre as amostras.
void measureVibration(Vibration &vib) {
  vib.valid = false;
  if (!accelOk) {
    accelOk = accelBegin();  // tenta de novo caso o sensor tenha sido ligado depois do boot
    if (!accelOk) return;
  }
  if (gravMagnitude <= 0) calibrateGravity();
  if (gravMagnitude <= 0) return;

  int batch = constrain(cfg.vibBatch, 10, VIB_MAX_BATCH);
  double sum = 0, sumSq = 0;
  float peak = 0;
  int n = 0;
  uint32_t firstUs = 0, lastUs = 0;

  uint32_t next = millis();
  for (int i = 0; i < batch; i++) {
    while ((int32_t)(millis() - next) < 0) {
      serviceTasks();
    }
    next += cfg.vibDelayMs;

    int16_t ax, ay, az;
    if (!readAccel(ax, ay, az)) continue;
    lastUs = micros();
    if (n == 0) firstUs = lastUs;
    vibSamples[n][0] = ax;
    vibSamples[n][1] = ay;
    vibSamples[n][2] = az;
    float v = magnitude(ax, ay, az) / gravMagnitude;
    vibEma = vibEma == 0 ? v : cfg.vibAlpha * v + (1.0f - cfg.vibAlpha) * vibEma;
    sum += v;
    sumSq += (double)v * v;
    if (v > peak) peak = v;
    vib.x = ax;
    vib.y = ay;
    vib.z = az;
    n++;
  }

  if (n < batch / 2) {
    Serial.println("Vibracao: muitas falhas de leitura no I2C, confira a fiacao");
    accelOk = false;
    return;
  }
  double mean = sum / n;
  double variance = sumSq / n - mean * mean;
  vib.ema = vibEma;
  vib.rms = sqrt(sumSq / n);
  vib.peak = peak;
  vib.stdDev = variance > 0 ? sqrt(variance) : 0;

  // Intervalo real entre amostras (o atendimento da interface web atrasa algumas)
  float dt = (lastUs - firstUs) / 1e6f / (n - 1);
  computeVelocity(n, dt, vib);
  vib.valid = true;
}

// ── Envio (HTTP POST) ────────────────────────────────────────────────────────

int64_t epochMs() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (tv.tv_sec < 1600000000) return 0;  // NTP ainda nao sincronizou
  return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void formatIso8601(int64_t ms, char *out, size_t len) {
  time_t secs = ms / 1000;
  struct tm t;
  gmtime_r(&secs, &t);
  size_t n = strftime(out, len, "%Y-%m-%dT%H:%M:%S", &t);
  snprintf(out + n, len - n, ".%03dZ", (int)(ms % 1000));
}

// Arredonda para o JSON nao sair com 7 casas de ruido
float round2(float v) { return roundf(v * 100.0f) / 100.0f; }

// Grandezas no formato do Eixo: temp, corr e vib (as que o ingest le). Sem leitura, a chave fica de fora.
// Usado no POST e na interface local, para a tela mostrar exatamente o que e enviado.
void addEixoValues(JsonObject p) {
  bool hasTemp = false;
  float maxTemp = 0;
  for (uint8_t i = 0; i < last.tempCount; i++) {
    if (last.temps[i] == DEVICE_DISCONNECTED_C) continue;
    if (!hasTemp || last.temps[i] > maxTemp) maxTemp = last.temps[i];
    hasTemp = true;
  }
  if (hasTemp) p["temp"] = round2(maxTemp);

  float sumAmps = 0;
  int phases = 0;
  for (size_t i = 0; i < CT_COUNT; i++) {
    if (last.amps[i] > 0) {
      sumAmps += last.amps[i];
      phases++;
    }
  }
  p["corr"] = round2(phases ? sumAmps / phases : 0);  // 0 = motor parado

  if (last.vib.valid) p["vib"] = round2(last.vib.vel);
}

// Formato da funcao ingest(p_token, p) do Eixo (GLMiranda0/eixo-tcc, prototipos/supabase/schema.sql):
//   {"p_token": "<token>", "p": {"ts": "...", "temp": 58.1, "corr": 9.4, "vib": 2.6}}
// temp: maior leitura valida dos DS18B20 (C); corr: media das fases com leitura (A);
// vib: velocidade RMS do eixo mais alto (mm/s). Grandeza sem leitura fica fora do JSON.
// Os demais campos de "p" sao ignorados pelo ingest e ficam como detalhe para depuracao.
void buildPayload(String &body) {
  JsonDocument doc;
  doc["p_token"] = cfg.token;
  JsonObject p = doc["p"].to<JsonObject>();

  if (last.tsMs > 0) {
    char ts[32];
    formatIso8601(last.tsMs, ts, sizeof(ts));
    p["ts"] = ts;  // sem NTP o banco usa a hora de chegada
  }

  addEixoValues(p);

  // Detalhe (fora do schema do Eixo)
  p["device_id"] = apName;
  p["ip"] = WiFi.localIP().toString();
  JsonObject fases = p["corr_fases"].to<JsonObject>();
  for (size_t i = 0; i < CT_COUNT; i++) {
    fases[currentSensors[i].name] = round2(last.amps[i]);
  }
  if (last.vib.valid) {
    JsonArray axes = p["vib_eixos"].to<JsonArray>();
    for (float v : last.vib.velAxes) axes.add(round2(v));
  }

  serializeJson(doc, body);
}

void postReadings() {
  if (cfg.postUrl.isEmpty() || !last.valid || WiFi.status() != WL_CONNECTED) return;

  String body;
  buildPayload(body);

  WiFiClient plainClient;
  WiFiClientSecure secureClient;
  secureClient.setInsecure();  // sem verificacao de certificado
  bool https = cfg.postUrl.startsWith("https://");

  HTTPClient http;
  http.setTimeout(5000);
  postStatus.atMs = millis();
  if (!(https ? http.begin(secureClient, cfg.postUrl) : http.begin(plainClient, cfg.postUrl))) {
    Serial.println("POST: URL invalida");
    postStatus = {-1, false, postStatus.atMs};
    return;
  }
  http.addHeader("Content-Type", "application/json");
  if (!cfg.apiKey.isEmpty()) http.addHeader("apikey", cfg.apiKey);  // PostgREST do Supabase

  int code = http.POST(body);
  postStatus.code = code;
  postStatus.ok = code >= 200 && code < 300;
  if (code > 0) {
    Serial.printf("POST %d -> %s\n", code, cfg.postUrl.c_str());
    if (!postStatus.ok) Serial.println("Resposta: " + http.getString());
  } else {
    Serial.printf("POST erro: %s\n", http.errorToString(code).c_str());
  }
  http.end();
}

// ── Interface web ────────────────────────────────────────────────────────────

void sendJson(int code, const JsonDocument &doc) {
  String buf;
  serializeJson(doc, buf);
  server.send(code, "application/json", buf);
}

void sendOk() {
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleRoot() {
  server.send(200, "text/html", WEBUI_HTML);
}

void handleStatus() {
  JsonDocument doc;
  doc["ip"] = WiFi.localIP().toString();
  doc["hostname"] = cfg.name;
  doc["ap"] = apName;
  doc["ssid"] = WiFi.SSID();
  doc["rssi"] = WiFi.RSSI();
  doc["accel"] = accelOk;
  doc["cal"] = gravMagnitude > 0;
  doc["grav"] = gravMagnitude;
  doc["post_enabled"] = !cfg.postUrl.isEmpty();
  doc["post_ok"] = postStatus.ok;
  doc["post_code"] = postStatus.code;
  doc["post_age_s"] = postStatus.atMs ? (long)((millis() - postStatus.atMs) / 1000) : -1L;
  doc["uptime"] = millis() / 1000;
  sendJson(200, doc);
}

void handleReadings() {
  if (!last.valid) {
    server.send(503, "application/json", "{}");
    return;
  }
  JsonDocument doc;
  doc["ts"] = last.tsMs;
  addEixoValues(doc["eixo"].to<JsonObject>());
  JsonArray temps = doc["temp_c"].to<JsonArray>();
  for (uint8_t i = 0; i < last.tempCount; i++) {
    if (last.temps[i] == DEVICE_DISCONNECTED_C) {
      temps.add(nullptr);
    } else {
      temps.add(last.temps[i]);
    }
  }
  JsonObject amps = doc["current_a"].to<JsonObject>();
  for (size_t i = 0; i < CT_COUNT; i++) {
    amps[currentSensors[i].name] = last.amps[i];
  }
  if (last.vib.valid) {
    JsonObject vib = doc["vib"].to<JsonObject>();
    vib["ema"] = last.vib.ema;
    vib["rms"] = last.vib.rms;
    vib["peak"] = last.vib.peak;
    vib["std"] = last.vib.stdDev;
    vib["vel"] = last.vib.vel;
    JsonArray axes = vib["vel_axes"].to<JsonArray>();
    for (float v : last.vib.velAxes) axes.add(v);
  }
  sendJson(200, doc);
}

void handleGetConfig() {
  JsonDocument doc;
  doc["name"] = cfg.name;
  doc["post_url"] = cfg.postUrl;
  doc["device_token"] = cfg.token;
  doc["api_key"] = cfg.apiKey;
  doc["post_interval"] = cfg.postIntervalS;
  doc["batch_size"] = cfg.vibBatch;
  doc["sample_delay"] = cfg.vibDelayMs;
  doc["alpha"] = cfg.vibAlpha;
  sendJson(200, doc);
}

void handlePostConfig() {
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"json invalido\"}");
    return;
  }
  bool renamed = false;
  if (doc["name"].is<const char *>()) {
    String name = doc["name"].as<String>();
    if (!normalizeName(name)) {
      server.send(400, "application/json", "{\"error\":\"nome invalido: use letras, numeros e hifen\"}");
      return;
    }
    renamed = name != cfg.name;
    cfg.name = name;
  }
  if (doc["post_url"].is<const char *>()) {
    String url = doc["post_url"].as<String>();
    if (!normalizeUrl(url)) {
      server.send(400, "application/json", "{\"error\":\"a URL deve comecar com http:// ou https://\"}");
      return;
    }
    cfg.postUrl = url;
  }
  if (doc["device_token"].is<const char *>()) {
    cfg.token = doc["device_token"].as<String>();
    cfg.token.trim();
  }
  if (doc["api_key"].is<const char *>()) {
    cfg.apiKey = doc["api_key"].as<String>();
    cfg.apiKey.trim();
  }
  if (doc["post_interval"].is<int>()) cfg.postIntervalS = constrain((int)doc["post_interval"], 2, 3600);
  if (doc["batch_size"].is<int>()) cfg.vibBatch = constrain((int)doc["batch_size"], 10, 2000);
  if (doc["sample_delay"].is<int>()) cfg.vibDelayMs = constrain((int)doc["sample_delay"], 1, 1000);
  if (doc["alpha"].is<float>()) {
    cfg.vibAlpha = constrain((float)doc["alpha"], 0.01f, 1.0f);
    vibEma = 0;
  }
  saveConfig();
  if (renamed) {
    // mDNS, NetBIOS e DHCP so pegam o nome novo do zero
    JsonDocument resp;
    resp["ok"] = true;
    resp["restart"] = true;
    resp["name"] = cfg.name;
    sendJson(200, resp);
    reqRestart = true;
  } else {
    sendOk();
  }
}

void handlePostNow() {
  if (cfg.postUrl.isEmpty()) {
    server.send(400, "application/json", "{\"error\":\"configure a URL de destino\"}");
    return;
  }
  reqPostNow = true;
  sendOk();
}

void handleCalibrate() {
  reqCal = true;
  sendOk();
}

void handleWifiReset() {
  sendOk();
  reqWifiReset = true;
}

void handleRestart() {
  sendOk();
  reqRestart = true;
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/readings", HTTP_GET, handleReadings);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handlePostConfig);
  server.on("/api/post-now", HTTP_POST, handlePostNow);
  server.on("/api/calibrate", HTTP_POST, handleCalibrate);
  server.on("/api/wifi/reset", HTTP_POST, handleWifiReset);
  server.on("/api/restart", HTTP_POST, handleRestart);
  server.begin();
}

// ── Wi-Fi (portal de configuracao) ───────────────────────────────────────────

// Campos extras do portal; globais porque o WiFiManager guarda ponteiros para eles
WiFiManagerParameter paramName("name", "Nome na rede (acesso por http://NOME.local)", "", NAME_MAX_LEN);
constexpr int URL_MAX_LEN = 200;
constexpr int TOKEN_MAX_LEN = 100;
constexpr int APIKEY_MAX_LEN = 300;  // a chave anon antiga (JWT) passa de 200 caracteres
WiFiManagerParameter paramUrl("post_url", "URL de destino (POST)", "", URL_MAX_LEN,
                              "placeholder='https://PROJETO.supabase.co/rest/v1/rpc/ingest'");
WiFiManagerParameter paramToken("token", "Token do dispositivo (tela do motor no Eixo)", "", TOKEN_MAX_LEN);
WiFiManagerParameter paramApiKey("api_key", "Chave publicavel do Supabase (apikey)", "", APIKEY_MAX_LEN);
char portalInfo[400];

// Preenche o portal com a configuracao atual e mostra como achar o coletor depois
void preparePortal() {
  paramName.setValue(cfg.name.c_str(), NAME_MAX_LEN);
  paramUrl.setValue(cfg.postUrl.c_str(), URL_MAX_LEN);
  paramToken.setValue(cfg.token.c_str(), TOKEN_MAX_LEN);
  paramApiKey.setValue(cfg.apiKey.c_str(), APIKEY_MAX_LEN);

  String ipLine = WiFi.isConnected() ? "<br>IP atual na rede: <b>" + WiFi.localIP().toString() + "</b>" : "";
  snprintf(portalInfo, sizeof(portalInfo),
           "<p style='text-align:center;font-size:14px'>Ap&oacute;s configurar, volte para a sua rede e acesse:<br>"
           "<b>http://%s.local</b><br>ou <b>http://%s</b> (Windows)%s</p>",
           cfg.name.c_str(), cfg.name.c_str(), ipLine.c_str());
  wm.setCustomMenuHTML(portalInfo);  // so aparece com "custom" no menu
}

// Os campos so mudam se o usuario salvou o formulario do portal
void applyPortalParams() {
  String name = paramName.getValue();
  String url = paramUrl.getValue();
  String token = paramToken.getValue();
  String apiKey = paramApiKey.getValue();
  token.trim();
  apiKey.trim();
  if (!normalizeName(name)) name = cfg.name;
  if (!normalizeUrl(url)) {
    Serial.println("URL do portal ignorada: deve comecar com http:// ou https://");
    url = cfg.postUrl;
  }
  if (name != cfg.name || url != cfg.postUrl || token != cfg.token || apiKey != cfg.apiKey) {
    cfg.name = name;
    cfg.postUrl = url;
    cfg.token = token;
    cfg.apiKey = apiKey;
    saveConfig();
    Serial.println("Configuracao salva pelo portal");
  }
}

void startNameServices() {
  if (MDNS.begin(cfg.name.c_str())) {
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Erro ao iniciar mDNS");
  }
  NBNS.begin(cfg.name.c_str());  // NetBIOS: http://coletor no Windows
}

// Sem Wi-Fi salvo (ou rede fora do ar), abre o AP "coletor-XXXXXX" com portal em 192.168.4.1,
// onde o usuario escolhe a rede, o nome do coletor e o destino do POST.
// O IP vem sempre por DHCP; o nome vai junto no pedido, e roteadores com DNS local o registram.
void setupWifi() {
  WiFi.setHostname(cfg.name.c_str());
  wm.setHostname(cfg.name.c_str());

  wm.addParameter(&paramName);
  wm.addParameter(&paramUrl);
  wm.addParameter(&paramToken);
  wm.addParameter(&paramApiKey);
  std::vector<const char *> menu = {"custom", "wifi", "info", "exit"};
  wm.setMenu(menu);
  wm.setConfigPortalTimeout(PORTAL_TIMEOUT_S);
  wm.setAPCallback([](WiFiManager *) {
    digitalWrite(LED_PIN, HIGH);  // LED aceso fixo = portal aberto
    Serial.printf("Portal aberto: conecte ao Wi-Fi \"%s\" e acesse http://192.168.4.1\n", apName);
  });
  preparePortal();

  Serial.printf("Conectando ao Wi-Fi como \"%s\" (DHCP)...\n", cfg.name.c_str());
  if (!wm.autoConnect(apName)) {
    Serial.println("Portal expirou sem conexao, reiniciando...");
    ESP.restart();
  }
  digitalWrite(LED_PIN, LOW);

  String oldName = cfg.name;
  applyPortalParams();
  if (cfg.name != oldName) {
    Serial.println("Nome alterado, reiniciando para registrar o nome novo no DHCP...");
    delay(100);
    ESP.restart();
  }

  startNameServices();
  Serial.printf("Wi-Fi: %s | IP (DHCP): %s | gateway: %s | DNS: %s\n", WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(),
                WiFi.dnsIP().toString().c_str());

  configTime(0, 0, "pool.ntp.org", "time.google.com");
  Serial.print("Sincronizando NTP");
  uint32_t start = millis();
  while (epochMs() == 0 && millis() - start < NTP_WAIT_MS) {
    delay(200);
    Serial.print('.');
  }
  Serial.println(epochMs() ? " OK" : " sem resposta (envia sem recorded_at ate sincronizar)");
}

// Toque curto no BOOT: abre o AP "coletor-XXXXXX" sem apagar o Wi-Fi salvo, mostrando o IP atual
// e permitindo trocar nome/destino. A coleta pausa ate salvar ou o portal expirar; depois reinicia.
void runConfigPortal() {
  Serial.printf("Abrindo portal sob demanda (%lu s)...\n", PORTAL_TIMEOUT_S);
  preparePortal();  // captura o IP antes do AP subir
  std::vector<const char *> menu = {"custom", "param", "wifi", "info", "exit"};
  wm.setMenu(menu);  // campos numa pagina propria: salvar nao exige redigitar a senha do Wi-Fi
  wm.setSaveParamsCallback([]() { wm.stopConfigPortal(); });
  server.stop();  // o portal usa a porta 80
  MDNS.end();

  wm.startConfigPortal(apName);

  applyPortalParams();
  Serial.println("Portal fechado, reiniciando...");
  delay(500);
  ESP.restart();
}

// ── Setup / loop ─────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  Serial.println();
  Serial.println("ESP32 iniciado - DS18B20 + SCT-013-005 + ADXL345");

  loadConfig();
  // Ultimos 3 bytes do MAC (os 3 primeiros sao o fabricante, iguais em toda placa)
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(apName, sizeof(apName), "coletor-%02x%02x%02x", mac[3], mac[4], mac[5]);

  analogReadResolution(12);
  for (const auto &ct : currentSensors) {
    analogSetPinAttenuation(ct.pin, ADC_11db);  // faixa ~0,15-3,1 V
  }

  sensors.begin();
  sensors.setResolution(12);
  // Nao bloqueia os 750 ms da conversao; medimos corrente e vibracao enquanto isso
  sensors.setWaitForConversion(false);

  uint8_t count = sensors.getDeviceCount();
  Serial.printf("Sensores encontrados no GPIO%d: %d\n", ONE_WIRE_PIN, count);

  DeviceAddress addr;
  for (uint8_t i = 0; i < count; i++) {
    if (sensors.getAddress(addr, i)) {
      Serial.printf("  #%d endereco: ", i);
      printAddress(addr);
      Serial.println();
    }
  }

  if (count == 0) {
    Serial.println("Nenhum sensor! Confira fiacao e o resistor de 4,7k (DATA -> 3V3).");
  }

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);
  accelOk = accelBegin();
  if (!accelOk) {
    Serial.printf("ADXL345 nao encontrado! Confira SDA->D%d, SCL->D%d, CS->3V3 e alimentacao 3V3.\n",
                  I2C_SDA_PIN, I2C_SCL_PIN);
  } else if (gravMagnitude <= 0) {
    calibrateGravity();
  } else {
    Serial.printf("Calibracao salva: %.1f LSB (comando CAL para refazer)\n", gravMagnitude);
  }

  setupWifi();
  setupServer();

  Serial.println("=======================================");
  Serial.printf("  Acesse: http://%s.local  ou  http://%s (Windows)\n", cfg.name.c_str(), cfg.name.c_str());
  Serial.println("  IP:     http://" + WiFi.localIP().toString());
  Serial.printf("  Destino: %s\n", cfg.postUrl.isEmpty() ? "(nao configurado)" : cfg.postUrl.c_str());
  Serial.println("=======================================");
  Serial.println("Serial: CAL recalibra, PORTAL abre a configuracao, IP mostra o endereco.");
  Serial.println("BOOT: toque curto abre a configuracao; 5 s apaga o Wi-Fi.");
}

void processSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toUpperCase();
  if (cmd == "CAL") {
    reqCal = true;
  } else if (cmd == "PORTAL") {
    reqPortal = true;
  } else if (cmd == "IP") {
    Serial.printf("IP: %s | http://%s.local | Wi-Fi: %s (%d dBm)\n", WiFi.localIP().toString().c_str(),
                  cfg.name.c_str(), WiFi.SSID().c_str(), WiFi.RSSI());
  } else {
    Serial.println("Comandos: CAL, PORTAL, IP");
  }
}

void loop() {
  uint32_t cycleStart = millis();
  processSerial();

  if (reqRestart) {
    delay(100);
    ESP.restart();
  }
  if (reqWifiReset) {
    Serial.println("Apagando Wi-Fi salvo e reiniciando no modo de configuracao...");
    digitalWrite(LED_PIN, HIGH);
    wm.resetSettings();
    delay(100);
    ESP.restart();
  }
  if (reqPortal) {
    runConfigPortal();
  }
  if (reqCal) {
    reqCal = false;
    calibrateGravity();
  }

  digitalWrite(LED_PIN, HIGH);

  uint8_t count = sensors.getDeviceCount();
  if (count == 0) {
    // Tenta redetectar caso o sensor tenha sido ligado depois do boot
    sensors.begin();
    count = sensors.getDeviceCount();
  }
  sensors.requestTemperatures();

  measureCurrents(last.amps);
  measureVibration(last.vib);

  while (count > 0 && !sensors.isConversionComplete()) {
    serviceTasks();
    delay(10);
  }
  digitalWrite(LED_PIN, LOW);

  last.tempCount = min(count, MAX_TEMP_SENSORS);
  for (uint8_t i = 0; i < last.tempCount; i++) {
    last.temps[i] = sensors.getTempCByIndex(i);
  }
  last.tsMs = epochMs();
  last.valid = true;

  for (size_t i = 0; i < CT_COUNT; i++) {
    Serial.printf("%s: %.2f A   ", currentSensors[i].name, last.amps[i]);
  }
  Serial.println();

  if (count == 0) {
    Serial.println("Temperatura: aguardando sensor...");
  }
  for (uint8_t i = 0; i < last.tempCount; i++) {
    if (last.temps[i] == DEVICE_DISCONNECTED_C) {
      Serial.printf("Sensor #%d: erro de leitura (desconectado?)\n", i);
    } else {
      Serial.printf("Sensor #%d: %.2f C\n", i, last.temps[i]);
    }
  }

  if (last.vib.valid) {
    Serial.printf("Vibracao: %.2f mm/s (X %.2f Y %.2f Z %.2f) | EMA=%.4f RMS=%.4f Pico=%.4f Std=%.4f\n",
                  last.vib.vel, last.vib.velAxes[0], last.vib.velAxes[1], last.vib.velAxes[2],
                  last.vib.ema, last.vib.rms, last.vib.peak, last.vib.stdDev);
  } else {
    Serial.println("Vibracao: aguardando ADXL345...");
  }

  if (reqPostNow || postStatus.atMs == 0 || millis() - postStatus.atMs >= cfg.postIntervalS * 1000) {
    reqPostNow = false;
    postReadings();
  }

  uint32_t elapsed = millis() - cycleStart;
  if (elapsed < READ_INTERVAL_MS) {
    waitServicing(READ_INTERVAL_MS - elapsed);
  }
}
