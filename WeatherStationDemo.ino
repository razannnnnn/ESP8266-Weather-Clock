#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <coredecls.h>
#include <JsonListener.h>
#include <time.h>
#include <sys/time.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "Wire.h"
#include "OpenWeatherMapCurrent.h"
#include "OpenWeatherMapForecast.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

/***************************
 * Settings
 **************************/
#define TZ              7
#define DST_MN          0

const int UPDATE_INTERVAL_SECS = 20 * 60;

const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = D2;
const int SDC_PIN = D1;

String OPEN_WEATHER_MAP_APP_ID = "b387f51e714a7e81a19e997c5f29491e";
float  OPEN_WEATHER_MAP_LOCATION_LAT = -8.0955;
float  OPEN_WEATHER_MAP_LOCATION_LON = 112.1608;
String OPEN_WEATHER_MAP_LANGUAGE = "id";

const uint8_t MAX_FORECASTS = 4;
const boolean IS_METRIC     = true;

const String WDAY_NAMES[]  = {"MIN","SEN","SEL","RAB","KAM","JUM","SAB"};
const String MONTH_NAMES[] = {"JAN","FEB","MAR","APR","MEI","JUN",
                               "JUL","AGU","SEP","OKT","NOV","DES"};

/***************************
 * End Settings
 **************************/

SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi   ui(&display);

OpenWeatherMapCurrentData  currentWeather;
OpenWeatherMapCurrent      currentWeatherClient;
OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
OpenWeatherMapForecast     forecastClient;

#define TZ_MN   ((TZ)*60)
#define TZ_SEC  ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)
time_t now;

bool readyForWeatherUpdate = false;
long timeSinceLastWUpdate  = 0;

// ── Prototypes ────────────────────────────────────────────────────────────────
void drawProgress(OLEDDisplay *display, int percentage, String label);
void updateData(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void setReadyForWeatherUpdate();

FrameCallback   frames[]   = { drawDateTime, drawCurrentWeather, drawForecast };
int numberOfFrames         = 3;
OverlayCallback overlays[] = { drawHeaderOverlay };
int numberOfOverlays       = 1;

// ─────────────────────────────────────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

    // ── Splash Screen ─────────────────────────────────────────────────────────
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 10, "Weather Clock");
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 35, "Made By Razan");
  display.drawHorizontalLine(20, 48, 88);
  display.display();
  delay(2500);  // tampil selama 2.5 detik
  // ─────────────────────────────────────────────────────────────────────────

  // ── WiFiManager ───────────────────────────────────────────────────────────
  display.clear();
  display.drawString(64, 10, "Connecting WiFi...");
  display.drawString(64, 28, "SSID: Clock-Setup");
  display.drawString(64, 42, "Jika gagal, buka");
  display.drawString(64, 52, "192.168.4.1");
  display.display();

  WiFiManager wm;
  wm.setConfigPortalTimeout(180);

  wm.setAPCallback([](WiFiManager* wm) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 5,  "-- WiFi Setup --");
    display.drawString(64, 20, "Connect ke:");
    display.drawString(64, 32, "Clock-Setup");
    display.drawString(64, 44, "Buka browser:");
    display.drawString(64, 54, "192.168.4.1");
    display.display();
  });

  bool connected = wm.autoConnect("Clock-Setup");

  if (!connected) {
    display.clear();
    display.drawString(64, 28, "Timeout. Reboot...");
    display.display();
    delay(2000);
    ESP.restart();
  }

  display.clear();
  display.drawString(64, 18, "WiFi Connected!");
  display.drawString(64, 36, WiFi.localIP().toString());
  display.display();
  delay(1500);

  // ── NTP ───────────────────────────────────────────────────────────────────
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org", "time.nist.gov");

  int dots = 0;
  while (time(nullptr) < 100000UL) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 18, "Syncing NTP...");
    String dotStr = "";
    for (int i = 0; i < (dots % 4); i++) dotStr += ".";
    display.drawString(64, 34, dotStr);
    display.display();
    delay(400);
    dots++;
  }

  // ── UI Setup ──────────────────────────────────────────────────────────────
  ui.setTargetFPS(30);
  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, numberOfFrames);
  ui.setOverlays(overlays, numberOfOverlays);
  ui.init();

  display.flipScreenVertically(); // ← setelah ui.init()

  updateData(&display);
}

// ─────────────────────────────────────────────────────────────────────────────
// LOOP
// ─────────────────────────────────────────────────────────────────────────────

void loop() {
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) {
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTIONS
// ─────────────────────────────────────────────────────────────────────────────

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display) {
  drawProgress(display, 10, "Update waktu...");
  drawProgress(display, 30, "Update cuaca...");
  currentWeatherClient.setMetric(IS_METRIC);
  currentWeatherClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  currentWeatherClient.updateCurrent(&currentWeather, OPEN_WEATHER_MAP_APP_ID,
    OPEN_WEATHER_MAP_LOCATION_LAT, OPEN_WEATHER_MAP_LOCATION_LON);

  drawProgress(display, 60, "Update forecast...");
  forecastClient.setMetric(IS_METRIC);
  forecastClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  uint8_t allowedHours[] = {12};
  forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
  forecastClient.updateForecasts(forecasts, OPEN_WEATHER_MAP_APP_ID,
    OPEN_WEATHER_MAP_LOCATION_LAT, OPEN_WEATHER_MAP_LOCATION_LON, MAX_FORECASTS);

  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Selesai!");
  delay(1000);
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  now = time(nullptr);
  struct tm* timeInfo = localtime(&now);
  char buff[32];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  sprintf_P(buff, PSTR("%s, %02d/%02d/%04d"),
    WDAY_NAMES[timeInfo->tm_wday].c_str(),
    timeInfo->tm_mday,
    timeInfo->tm_mon + 1,
    timeInfo->tm_year + 1900);
  display->drawString(64 + x, 5 + y, String(buff));

  display->setFont(ArialMT_Plain_24);
  sprintf_P(buff, PSTR("%02d:%02d:%02d"),
    timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 15 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C");
  display->drawString(60 + x, 5 + y, temp);

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  drawForecastDetails(display, x,      y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm* timeInfo = localtime(&observationTimestamp);

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, WDAY_NAMES[timeInfo->tm_wday]);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, forecasts[dayIndex].iconMeteoCon);

  String temp = String(forecasts[dayIndex].temp, 0) + (IS_METRIC ? "°C" : "°F");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  now = time(nullptr);
  struct tm* timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 54, String(buff));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(128, 54, temp);
  display->drawHorizontalLine(0, 52, 128);
}

void setReadyForWeatherUpdate() {
  readyForWeatherUpdate = true;
}
