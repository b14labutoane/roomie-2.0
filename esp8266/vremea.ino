// includerea librariilor
#include <Arduino.h>
#include <ESPWiFi.h>  // libraria de conectare la wifi a modulului esp86
#include <ESPHTTPClient.h>
#include <JsonListener.h>
#include <time.h>       // time() ctime()
#include <sys/time.h>   // struct timeval
#include <coredecls.h>  // settimeofday_cb()
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "Wire.h"
#include "OpenWeatherMapCurrent.h"
#include "OpenWeatherMapForecast.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"
#include "ThingspeakClient.h"
#include <DHT.h>  // libraria senzorului de temperatura


// definirea notelor muzicale
#define NOTE_AS4 466
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_F5 698
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_C5 523
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_DS6 1245
#define NOTE_E5 659
#define NOTE_F6 1397
#define REST 0

//definirea standardelor timpului
#define TZ_MN ((TZ)*60)
#define TZ_SEC ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)

//definirea tipului de senzor de umiditate
#define DHTTYPE DHT11

// partitura melodiei
int melody[] = {
  NOTE_AS4, NOTE_AS4, NOTE_AS4,
  NOTE_F5, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5, NOTE_C5, NOTE_C5, NOTE_C5,
  NOTE_F5, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,

  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5, NOTE_C5, NOTE_C5,
  NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,
  NOTE_F5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_D5, NOTE_E5, NOTE_C5, NOTE_C5,
  NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,

  NOTE_C6, NOTE_G5, NOTE_G5, REST, NOTE_C5,
  NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,
  NOTE_F5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_D5, NOTE_E5, NOTE_C6, NOTE_C6,
  NOTE_F6, NOTE_DS6, NOTE_CS6, NOTE_C6, NOTE_AS5, NOTE_GS5, NOTE_G5, NOTE_F5,
  NOTE_C6
};

// durata notelor melodiei
int durations[] = {
  8, 8, 8,
  2, 2,
  8, 8, 8, 2, 4,
  8, 8, 8, 2, 4,
  8, 8, 8, 2, 8, 8, 8,
  2, 2,
  8, 8, 8, 2, 4,

  8, 8, 8, 2, 4,
  8, 8, 8, 2, 8, 16,
  4, 8, 8, 8, 8, 8,
  8, 8, 8, 4, 8, 4, 8, 16,
  4, 8, 8, 8, 8, 8,

  8, 16, 2, 8, 8,
  4, 8, 8, 8, 8, 8,
  8, 8, 8, 4, 8, 4, 8, 16,
  4, 8, 4, 8, 4, 8, 4, 8,
  1
};

// WIFI
const char *WIFI_SSID = "SSID-parter";
const char *WIFI_PWD = "doctor13579vitamin24680!";

#define TZ 2       // (utc+) TZ in hours
#define DST_MN 60  // use 60mn for summer time in some countries

// Setup
const int UPDATE_INTERVAL_SECS = 10 * 60;  // Update every 10 minutes

// Setari de display
const int I2C_DISPLAY_ADDRESS = 0x3c;

// Definirea pinilor
const int SDA_PIN = 0;
const int SDC_PIN = 2;
const int DHT_PIN = 12;

// Id-urile de pe site-ul Open Weather
String OPEN_WEATHER_MAP_APP_ID = "83ee1d270f230e640405a19332c2fd61";
String OPEN_WEATHER_MAP_LOCATION_ID = "665087";
String OPEN_WEATHER_MAP_LANGUAGE = "en";
const uint8_t MAX_FORECASTS = 4;
const boolean IS_METRIC = true;

// Vectorii de nume de zile, de luni
const String WDAY_NAMES[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
const String MONTH_NAMES[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

// Declararea display-ului
SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi ui(&display);

// Declararea variabilelor care au legatura cu vremea
OpenWeatherMapCurrentData currentWeather;
OpenWeatherMapCurrent currentWeatherClient;
OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
OpenWeatherMapForecast forecastClient;
time_t now;
bool readyForWeatherUpdate = false;
String lastUpdate = "--";
long timeSinceLastWUpdate = 0;

// Id-urile Thingspeak
const String THINGSPEAK_CHANNEL_ID = "1843455";
const String THINGSPEAK_API_READ_KEY = "ZCB5RT7ANFKMJ1WU";

//Declararea functiilor
void drawProgress(OLEDDisplay *display, int percentage, String label);
void updateData(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState *state);
void setReadyForWeatherUpdate();
void drawHomePage(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawIndoorWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);

// Frame-urile de pe ecran
FrameCallback frames[] = { drawHomePage, drawDateTime, drawCurrentWeather, drawForecast, drawIndoorWeather };
int numberOfFrames = 5;
OverlayCallback overlays[] = { drawHeaderOverlay };
int numberOfOverlays = 1;

// Declararea variabilei senzorului de temperatura
DHT dht(DHT_PIN, DHTTYPE);

// Declararea pinilor
const int trigPin = 5;
const int echoPin = 4;
const int buzzer = 14;

// Delararea variabilelor
long duration;
int distance;
int safetyDistance = 10;
float temperature = 0.0;
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int divider = 0, noteDuration = 0;

// Functie care returneaza distanta dintre un posibil obiect si senzorul de miscare
int ultrasonic() {
  // Reseteaza unda
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Transmite o unda timp de 10 secunde
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Primeste unda si returneaza viteza sunetului
  duration = pulseIn(echoPin, HIGH);

  // Calculeaza distanta
  distance = duration * 0.034 / 2;

  return distance;
}

// Functie care canta melodia "Star Wars Intro"
void song() {
   noTone(buzzer);
  int size = sizeof(durations) / sizeof(int);
  for (int note = 0; note < size; note++) {
    //to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int durationz = 1000 / durations[note];
    tone(buzzer, melody[note], durationz);

    //to distinguish the notes, set a minimum time between them.
    //the note's duration + 30% seems to work well:
    int pauseBetweenNotes = durationz * 1.30;
    delay(pauseBetweenNotes);

    //stop the tone playing:
    noTone(buzzer);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  dht.begin();

  // Pornirea display-ului
  display.init();
  display.clear();
  display.display();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  // Realizarea conexiunii WI-FI
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.clear();
    display.drawString(64, 10, "Connecting to WiFi");
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
    display.display();
    counter++;
  }

  // Obtine timpul de pe internet
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");
  ui.setTargetFPS(30);
  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, numberOfFrames);
  ui.setOverlays(overlays, numberOfOverlays);
  ui.init();
  Serial.println("");
  updateData(&display);

  // Setarea pinilor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  safetyDistance = 1;
  noTone(buzzer);
}

void loop() {
  //Se modifica vremea
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

// Functie care afiseaza bara de progres
void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

// Functie care updateaza datele si porneste ecranul
void updateData(OLEDDisplay *display) {
  drawProgress(display, 10, "Updating time...");
  drawProgress(display, 30, "Updating weather...");
  currentWeatherClient.setMetric(IS_METRIC);
  currentWeatherClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  currentWeatherClient.updateCurrentById(&currentWeather, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID);
  drawProgress(display, 50, "Updating forecasts...");
  forecastClient.setMetric(IS_METRIC);
  forecastClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  uint8_t allowedHours[] = { 12 };
  forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
  forecastClient.updateForecastsById(forecasts, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, MAX_FORECASTS);
  readyForWeatherUpdate = false;
  temperature = dht.readTemperature(!IS_METRIC);
  drawProgress(display, 100, "Done...");
  delay(1000);
  drawProgress(display, 100, "Hi there!");
  delay(3000);
  drawProgress(display, 100, "I'm Roomie!");
  delay(5000);
}

// Functie care arata timpul
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%s, %02d/%02d/%04d"), WDAY_NAMES[timeInfo->tm_wday].c_str(), timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
  display->drawString(64 + x, 5 + y, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 15 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

// Functie care arata vremea
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 35 + y, currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(50 + x, 5 + y, temp);

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(22 + x, 0 + y, currentWeather.iconMeteoCon);
}

// Functie care arata prima pagina
void drawHomePage(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  delay(1000);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, "Hi there, I'm Roomie!");
}

// Functie care arata temperatura din camera
void drawIndoorWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0 + y, "Indoor");
  display->setFont(ArialMT_Plain_24);
  temperature = dht.readTemperature(!IS_METRIC);
  display->drawString(64 + x, 10 + y, String(temperature) + "°C");
}

// Functie care arata detaliile vremii din cele 3 zile
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

// Functie care arata detaliile vremii dintr-o zi
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm *timeInfo;
  timeInfo = localtime(&observationTimestamp);
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

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState *state) {
  now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&now);
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
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}
