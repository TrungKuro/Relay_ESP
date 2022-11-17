/* ------------------------------------------------------------------------- */
/*                                  LIBRARY                                  */
/* ------------------------------------------------------------------------- */

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"

/* -------------- DEBUG (uncomment to open the Debug function) ------------- */

// #define ENABLE_DEBUG

#if defined(ENABLE_DEBUG)
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

/* --------------------------------- Button -------------------------------- */

/* Read ADC from pin "TOUT" (A0)
** When no 'click' the button, the value voltage is 0V ~ 3ADC
**
**          | VDC | ADC | Real value | Range ± 0.05 VDC
** Button 1 = 0.2 : 062 ~ 057 -5    -> [0.15 - 0.25] = [046 - 078]
** Button 2 = 0.4 : 124 ~ 116 +8    -> [0.35 - 0.45] = [108 - 140]
** Button 3 = 0.6 : 186 ~ 180 +6    -> [0.55 - 0.65] = [170 - 202]
** Button 4 = 0.8 : 248 ~ 237 +11   -> [0.75 - 0.85] = [232 - 264]
**
** The ADC pin has a 10-bit resolution, which means you’ll get values between 0 and 1023
** The ESP8266 ADC pin input voltage range is 0 to 1V if you’re using the bare chip
*/

#define BUTTON A0

#define BUTTON_1_MIN 46
#define BUTTON_1_MAX 78

#define BUTTON_2_MIN 108
#define BUTTON_2_MAX 140

#define BUTTON_3_MIN 170
#define BUTTON_3_MAX 202

#define BUTTON_4_MIN 232
#define BUTTON_4_MAX 264

bool statusBtn = false;

/* ------------------------------- Status LED ------------------------------ */

/* Trigger HIGH to turn-on LED
** Trigger LOW to turn-off LED
*/

#define LED 16

#define LED_ON digitalWrite(LED, HIGH)
#define LED_OFF digitalWrite(LED, LOW)

/* --------------------------------- Relay --------------------------------- */

/* Trigger LOW to turn-on Relay
** Trigger HIGH to turn-off Relay
**
** Relay 1 - IO 04
** Relay 2 - IO 05
** Relay 3 - IO 12
** Relay 4 - IO 13
*/

#define RELAY_1 4
#define RELAY_2 5
#define RELAY_3 12
#define RELAY_4 13

/* /update?output=[?]&state=[?] */
#define PARAM_INPUT_1 "output" // What pin output of Relay?
#define PARAM_INPUT_2 "state"  // State output of pin, with 1 is HIGH, 0 is LOW

bool statusRelay[4] = {
    true, // Relay 1
    true, // Relay 2
    true, // Relay 3
    true  // Relay 4
};        // Turn-off all Relays

#define TRIGGER_RELAY_1 digitalWrite(RELAY_1, statusRelay[0])
#define TRIGGER_RELAY_2 digitalWrite(RELAY_2, statusRelay[1])
#define TRIGGER_RELAY_3 digitalWrite(RELAY_3, statusRelay[2])
#define TRIGGER_RELAY_4 digitalWrite(RELAY_4, statusRelay[3])

String statusRelay1 = "";
String statusRelay2 = "";
String statusRelay3 = "";
String statusRelay4 = "";

/* ---------------------------------- RTC ---------------------------------- */

/* SCL (DS1307) - IO 14
** SDA (DS1307) - IO 02
*/

#define SCL 14
#define SDA 2

RTC_DS1307 rtc;

String timeRTC = "";
String dateRTC = "";

/* --------------------- Equipment inspection frequency -------------------- */

unsigned long checkBtn;
unsigned long checkRTC;

#define TIME_CHECK_BTN 10   // Unit (ms)
#define TIME_CHECK_RTC 1000 // Unit(ms)

/* ----------------- Replace with your network credentials ----------------- */

#define WIFI_SSID "Relay ESP"
#define WIFI_PASSWORD "Demo123#"

/* --------------------------- Access Point (AP) --------------------------- */

/* Create AsyncWebServer object on port 80 */
AsyncWebServer server(80);

/* Create an Event Source on "/events" */
AsyncEventSource events("/events");

/* When the slider is red, it means the output is on (its state is HIGH)
** When the slider is gray, it means the output is off (its state is LOW)
**
** HTTP GET request on the ... URL
** /update?output=[?]&state=[?]
**
** When ESP receives a URL, it has info to know what to do
*/

/* Web (HTML, CSS, JAVASCRIPT) */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Demo Relay ESP</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }
    body {
      max-width: 600px;
      margin: 0px auto;
      padding-bottom: 25px;
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 120px;
      height: 68px
    }
    .switch input {
      display: none
    }
    .slider {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      border-radius: 6px
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 52px;
      width: 52px;
      left: 8px;
      bottom: 8px;
      background-color: #fff;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 3px
    }
    input:checked+.slider {
      background-color: #b30000
    }
    input:checked+.slider:before {
      -webkit-transform: translateX(52px);
      -ms-transform: translateX(52px);
      transform: translateX(52px)
    }
  </style>
</head>
<body>
  <h1>ESP8266 Relay (RTC) Server</h1>
  <h2>TIME - <span id="time">%TIME%</span></h2>
  <h2>DATE - <span id="date">%DATE%</span></h2>
  %RELAY%
</body>
<script>
  function toggleCheckbox(element, relay) {
    var stateRelay = document.getElementById(relay);
    var xhr = new XMLHttpRequest();
    if (element.checked) {
      stateRelay.innerHTML = "ON";
      xhr.open("GET", "/update?output=" + element.id + "&state=0", true);
    }
    else {
      stateRelay.innerHTML = "OFF";
      xhr.open("GET", "/update?output=" + element.id + "&state=1", true);
    }
    xhr.send();
  };
  if (!!window.EventSource) {
    var source = new EventSource('/events');
    source.addEventListener('open', function (e) {
      console.log("Events Connected");
    }, false);
    source.addEventListener('error', function (e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
    }, false);
    source.addEventListener('message', function (e) {
      console.log("message", e.data);
    }, false);
    source.addEventListener('relay1', function (e) {
      console.log("relay1", e.data);
      document.getElementById("relay1").innerHTML = e.data;
      if (e.data == "ON") {
        document.getElementById("4").checked = true;
      }
      else if (e.data == "OFF") {
        document.getElementById("4").checked = false;
      }
    }, false);
    source.addEventListener('relay2', function (e) {
      console.log("relay2", e.data);
      document.getElementById("relay2").innerHTML = e.data;
      if (e.data == "ON") {
        document.getElementById("5").checked = true;
      }
      else if (e.data == "OFF") {
        document.getElementById("5").checked = false;
      }
    }, false);
    source.addEventListener('relay3', function (e) {
      console.log("relay3", e.data);
      document.getElementById("relay3").innerHTML = e.data;
      if (e.data == "ON") {
        document.getElementById("12").checked = true;
      }
      else if (e.data == "OFF") {
        document.getElementById("12").checked = false;
      }
    }, false);
    source.addEventListener('relay4', function (e) {
      console.log("relay4", e.data);
      document.getElementById("relay4").innerHTML = e.data;
      if (e.data == "ON") {
        document.getElementById("13").checked = true;
      }
      else if (e.data == "OFF") {
        document.getElementById("13").checked = false;
      }
    }, false);
  }
</script>
</html>
)rawliteral";

/* ------------------------------------------------------------------------- */
/*                                  FUNCTION                                 */
/* ------------------------------------------------------------------------- */

/* Handling the state of Relay */
String stateRelay(byte output)
{
  if (digitalRead(output))
  {
    return "OFF"; // Output now HIGH, mean turn-off Relay
  }
  else
  {
    return "ON"; // Output now LOW, mean turn-on Relay
  }
}

/* Handling the state of Slider Checkbox */
String stateCheckbox(byte output)
{
  if (digitalRead(output))
  {
    return ""; // Output now HIGH, mean turn-off Relay
  }
  else
  {
    return "checked"; // Output now LOW, mean turn-on Relay
  }
}

/* Replace placeholder with RTC values and state Relay */
String processor(const String &var)
{
  getRTCReadings(); // Update data from RTC

  DEBUG_PRINT("Var: ");
  DEBUG_PRINTLN(var);

  if (var == "TIME")
  {
    return timeRTC;
  }
  else if (var == "DATE")
  {
    return dateRTC;
  }
  else if (var == "RELAY")
  {
    statusRelay1 = stateRelay(RELAY_1);
    statusRelay2 = stateRelay(RELAY_2);
    statusRelay3 = stateRelay(RELAY_3);
    statusRelay4 = stateRelay(RELAY_4);

    String relays = "";
    relays += "<h3>RELAY 1 - <span id=\"relay1\">" + statusRelay1 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay1')\" id=\"" + String(RELAY_1) + "\" " + stateCheckbox(RELAY_1) + "><span class=\"slider\"></span></label>";
    relays += "<h3>RELAY 2 - <span id=\"relay2\">" + statusRelay2 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay2')\" id=\"" + String(RELAY_2) + "\" " + stateCheckbox(RELAY_2) + "><span class=\"slider\"></span></label>";
    relays += "<h3>RELAY 3 - <span id=\"relay3\">" + statusRelay3 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay3')\" id=\"" + String(RELAY_3) + "\" " + stateCheckbox(RELAY_3) + "><span class=\"slider\"></span></label>";
    relays += "<h3>RELAY 4 - <span id=\"relay4\">" + statusRelay4 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay4')\" id=\"" + String(RELAY_4) + "\" " + stateCheckbox(RELAY_4) + "><span class=\"slider\"></span></label>";
    return relays;
  }

  return String();
}

/* ------------------------------------------------------------------------- */

/* Detect button, catch the "Edge-Up" */
char detectButton()
{
  int btn = analogRead(BUTTON);

  if (btn >= BUTTON_1_MIN && btn <= BUTTON_4_MAX)
  {
    /* Only need read ADC, when "statusBtn" is FALSE
    ** This means that the button is not currently pressed
    ** Skip if "statusBtn" still TRUE
    */
    if (!statusBtn)
    {
      delay(10); // Skip Debounce
      btn = analogRead(BUTTON);

      if (btn >= BUTTON_1_MIN && btn <= BUTTON_1_MAX)
      {
        DEBUG_PRINTLN(F("Btn 1"));

        statusBtn = true;
        return '1';
      }
      else if (btn >= BUTTON_2_MIN && btn <= BUTTON_2_MAX)
      {
        DEBUG_PRINTLN(F("Btn 2"));

        statusBtn = true;
        return '2';
      }
      else if (btn >= BUTTON_3_MIN && btn <= BUTTON_3_MAX)
      {
        DEBUG_PRINTLN(F("Btn 3"));

        statusBtn = true;
        return '3';
      }
      else if (btn >= BUTTON_4_MIN && btn <= BUTTON_4_MAX)
      {
        DEBUG_PRINTLN(F("Btn 4"));

        statusBtn = true;
        return '4';
      }
    }
  }
  else
  {
    statusBtn = false;
  }
}

/* Get data from RTC */
void getRTCReadings()
{
  DateTime now = rtc.now();

  /* Buffer can be defined using following combinations:
  ** hh - the hour with a leading zero (00 to 23)
  ** mm - the minute with a leading zero (00 to 59)
  ** ss - the whole second with a leading zero where applicable (00 to 59)
  ** YYYY - the year as four digit number
  ** YY - the year as two digit number (00-99)
  ** MM - the month as number with a leading zero (01-12)
  ** MMM - the abbreviated English month name ('Jan' to 'Dec')
  ** DD - the day as number with a leading zero (01 to 31)
  ** DDD - the abbreviated English day name ('Mon' to 'Sun')
  */

  char timeBuf[] = "hh:mm:ss";
  timeRTC = now.toString(timeBuf);
  DEBUG_PRINTLN(timeRTC);

  char dateBuf[] = "DD/MM/YYYY";
  dateRTC = now.toString(dateBuf);
  DEBUG_PRINTLN(dateRTC);
}

/* ------------------------------------------------------------------------- */
/*                                    MAIN                                   */
/* ------------------------------------------------------------------------- */

void setup()
{
  /* To use the Debug feature */
  Serial.begin(115200);

  /* Initialize LED, and turn off it */
  pinMode(LED, OUTPUT);
  LED_OFF;

  /* Initialize Relay with initial default state */
  pinMode(RELAY_1, OUTPUT);
  TRIGGER_RELAY_1;
  pinMode(RELAY_2, OUTPUT);
  TRIGGER_RELAY_2;
  pinMode(RELAY_3, OUTPUT);
  TRIGGER_RELAY_3;
  pinMode(RELAY_4, OUTPUT);
  TRIGGER_RELAY_4;

  /* Initialize RTC */
  Wire.begin(SDA, SCL);
  rtc.begin();
  delay(500);
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //!!!DEBUG

  /* Setting the ESP8266 as an Access Point (AP) */
  DEBUG_PRINT(F("Setting AP (Access Point) ..."));
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  /* Note: by default, the access point IP address is 192.168.4.1 */
  IPAddress IP = WiFi.softAPIP();
  DEBUG_PRINT(F("AP IP address: "));
  DEBUG_PRINTLN(IP);

  /* Print ESP8266 Local IP Address */
  DEBUG_PRINTLN(WiFi.localIP());

  /* Route for root / web page (Handle Web Server)
  **
  ** c_str()
  ** -> https://www.arduino.cc/reference/en/language/variables/data-types/string/functions/c_str/
  */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              DEBUG_PRINTLN(F("--- WEB ---"));
              request->send_P(200, "text/html", index_html, processor); });

  /* Handle Web Server Events */
  events.onConnect([](AsyncEventSourceClient *client)
                   {
                    if (client->lastId())
                    {
                      DEBUG_PRINT(F("Client reconnected! Last message ID that it got is: "));
                      DEBUG_PRINTLN(client->lastId());
                    }
                    /* Send event with message "hello!", id current millis
                    ** And set reconnect delay to 0.1 second
                    */
                    client->send("hello!", NULL, millis(), 1000); });

  /* ----------------------------------------------------------------------- */

  /* Send a GET request to <ESP_IP>/time=<timeRTC.c_str()> */
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request)
            { DEBUG_PRINTLN(F("TIME"));
              request->send_P(200, "text/plain", timeRTC.c_str()); });

  /* Send a GET request to <ESP_IP>/date=<dateRTC.c_str()> */
  server.on("/date", HTTP_GET, [](AsyncWebServerRequest *request)
            { DEBUG_PRINTLN(F("DATE"));
              request->send_P(200, "text/plain", dateRTC.c_str()); });

  /* ----------------------------------------------------------------------- */

  /* Send a GET request to <ESP_IP>/relay1=<String(statusRelay[0])> */
  // server.on("/relay1", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send_P(200, "text/plain", String(statusRelay[0]).c_str()); });

  /* Send a GET request to <ESP_IP>/relay2=<String(statusRelay[1])> */
  // server.on("/relay2", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send_P(200, "text/plain", String(statusRelay[1]).c_str()); });

  /* Send a GET request to <ESP_IP>/relay3=<String(statusRelay[2])> */
  // server.on("/relay3", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send_P(200, "text/plain", String(statusRelay[2]).c_str()); });

  /* Send a GET request to <ESP_IP>/relay4=<String(statusRelay[3])> */
  // server.on("/relay4", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send_P(200, "text/plain", String(statusRelay[3]).c_str()); });

  /* ----------------------------------------------------------------------- */

  /* Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2> */
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String inputMessage1;
              String inputMessage2;

              /* GET value of "output" and "state" */
              if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2))
              {
                inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
                inputMessage2 = request->getParam(PARAM_INPUT_2)->value();

                switch (inputMessage1.toInt()) // Update for "statusRelay"
                {
                case RELAY_1:
                  (inputMessage2.toInt()) ? (statusRelay[0] = true) : (statusRelay[0] = false);
                  TRIGGER_RELAY_1;
                  break;
                case RELAY_2:
                  (inputMessage2.toInt()) ? (statusRelay[1] = true) : (statusRelay[1] = false);
                  TRIGGER_RELAY_2;
                  break;
                case RELAY_3:
                  (inputMessage2.toInt()) ? (statusRelay[2] = true) : (statusRelay[2] = false);
                  TRIGGER_RELAY_3;
                  break;
                case RELAY_4:
                  (inputMessage2.toInt()) ? (statusRelay[3] = true) : (statusRelay[3] = false);
                  TRIGGER_RELAY_4;
                  break;
                }
              }
              else
              {
                inputMessage1 = "No message sent";
                inputMessage2 = "No message sent";
              }

              DEBUG_PRINT(F("GPIO: "));
              DEBUG_PRINT(inputMessage1);
              DEBUG_PRINT(F(" - Set to: "));
              DEBUG_PRINTLN(inputMessage2);

              request->send(200, "text/plain", "OK"); });

  /* Start Server */
  server.addHandler(&events);
  server.begin();
}

/* ------------------------------------------------------------------------- */

void loop()
{
  if (millis() - checkBtn >= TIME_CHECK_BTN)
  {
    char relay = detectButton();
    switch (relay)
    {
    case '1':
      statusRelay[0] = !statusRelay[0];
      TRIGGER_RELAY_1;
      statusRelay1 = stateRelay(RELAY_1);

      /* Send Events to the Web Server */
      events.send("ping", NULL, millis());
      events.send(statusRelay1.c_str(), "relay1", millis());
      break;
    case '2':
      statusRelay[1] = !statusRelay[1];
      TRIGGER_RELAY_2;
      statusRelay2 = stateRelay(RELAY_2);

      /* Send Events to the Web Server */
      events.send("ping", NULL, millis());
      events.send(statusRelay2.c_str(), "relay2", millis());
      break;
    case '3':
      statusRelay[2] = !statusRelay[2];
      TRIGGER_RELAY_3;
      statusRelay3 = stateRelay(RELAY_3);

      /* Send Events to the Web Server */
      events.send("ping", NULL, millis());
      events.send(statusRelay3.c_str(), "relay3", millis());
      break;
    case '4':
      statusRelay[3] = !statusRelay[3];
      TRIGGER_RELAY_4;
      statusRelay4 = stateRelay(RELAY_4);

      /* Send Events to the Web Server */
      events.send("ping", NULL, millis());
      events.send(statusRelay4.c_str(), "relay4", millis());
      break;
    }

    checkBtn = millis();
  }

  /* ----------------------------------------------------------------------- */

  if (millis() - checkRTC >= TIME_CHECK_RTC)
  {
    getRTCReadings();

    checkRTC = millis();
  }
}
