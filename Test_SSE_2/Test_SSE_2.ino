#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"

/* ------------------------------------------------------------------------- */

#define SCL 14
#define SDA 2

RTC_DS1307 rtc;

String timeRTC = "";
String dateRTC = "";

char timeBuf[] = "hh:mm:ss";
char dateBuf[] = "DD/MM/YYYY";

unsigned long checkRTC;
#define TIME_CHECK_RTC 1000 // Unit(ms)

/* ------------------------------------------------------------------------- */

// Replace with your network credentials
// const char *ssid = "Truong Ngoc";
// const char *password = "NGOC5G65@382N%#1974";
#define WIFI_SSID "Relay ESP"
#define WIFI_PASSWORD "Demo123#"

/* ------------------------------------------------------------------------- */

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

/* ------------------------------------------------------------------------- */

void getSensorReadings()
{
  DateTime now = rtc.now();

  char timeBuf[] = "hh:mm:ss";
  timeRTC = now.toString(timeBuf);

  char dateBuf[] = "DD/MM/YYYY";
  dateRTC = now.toString(dateBuf);
}

// Initialize WiFi
void initWiFi()
{
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi ..");
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   Serial.print('.');
  //   delay(1000);
  // }
  // Serial.println(WiFi.localIP());

  Serial.print(F("Setting AP (Access Point) ..."));
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print(F("AP IP address: "));
  Serial.println(IP);

  Serial.println(WiFi.localIP());
}

String processor(const String &var)
{
  getSensorReadings();

  if (var == "TIME_RTC")
  {
    return timeRTC;
  }
  else if (var == "DATE_RTC")
  {
    return dateRTC;
  }
  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>BME280 WEB SERVER (SSE)</h1>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <p><i class="fas fa-thermometer-half" style="color:#059e8a;"></i> SHOW TIME</p><p><span class="reading"><span id="temp">%TIME_RTC%</span> &deg;C</span></p>
      </div>
      <div class="card">
        <p><i class="fas fa-tint" style="color:#00add6;"></i> SHOW DATE</p><p><span class="reading"><span id="hum">%DATE_RTC%</span> &percnt;</span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('time', function(e) {
  console.log("time", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('date', function(e) {
  console.log("date", e.data);
  document.getElementById("hum").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup()
{
  Serial.begin(115200);
  initWiFi();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.print("Client reconnected! Last message ID that it got is: ");
      Serial.println(client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 0.1 second
    client->send("hello!", NULL, millis(), 1000); });
  server.addHandler(&events);
  server.begin();
}

void loop()
{
  if ((millis() - checkRTC) > TIME_CHECK_RTC)
  {
    getSensorReadings();
    Serial.println(timeRTC);
    Serial.println(dateRTC);
    Serial.println();

    // Send Events to the Web Server with the Sensor Readings
    events.send("ping", NULL, millis());
    events.send(timeRTC.c_str(), "time", millis());
    events.send(dateRTC.c_str(), "date", millis());

    checkRTC = millis();
  }
}