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
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <style>
    html {
      display: inline-block;
      margin: 0px auto;
      font-family: Arial;
      text-align: center;
    }

    p {
      font-size: 1.2rem;
    }

    body {
      margin: 0px auto;
      padding-bottom: 25px;
    }

    /* ------------------------------- HEADER ------------------------------ */

    .topnav {
      overflow: hidden;
      background-color: #00ABB3;
      color: white;
      font-size: 1rem;
    }

    /* -------------------------------- CARD ------------------------------- */

    .content {
      padding: 20px;
    }

    .cards {
      /* max-width: 100%; */
      margin: 0px auto;
      display: grid;
      grid-gap: 2rem;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    }

    .card {
      padding: 10px;
      background-color: white;
      box-shadow: 2px 2px 12px 1px rgba(178, 178, 178, .5);
    }

    /* ------------------------------- RELAY ------------------------------- */

    .switch {
      position: relative;
      display: inline-block;
      width: 120px;
      height: 68px;
      cursor: pointer;
    }

    .switch input {
      display: none;
    }

    .slider {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #B2B2B2;
      border-radius: 6px;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 52px;
      width: 52px;
      left: 8px;
      bottom: 8px;
      background-color: white;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 3px;
    }

    input:checked+.slider {
      background-color: #00ABB3;
    }

    input:checked+.slider:before {
      -webkit-transform: translateX(52px);
      -ms-transform: translateX(52px);
      transform: translateX(52px);
    }

    /* ------------------------------- BUTTON ------------------------------ */

    .center {
      display: flex;
      justify-content: center;
      align-items: center;
    }

    .btn {
      background-color: #EAEAEA;
      color: black;
      font-size: 16px;
      padding: 16px 30px;
      border: none;
      cursor: pointer;
      border-radius: 5px;
      text-align: center;
    }

    .btn:hover {
      background-color: #3C4048;
      color: white;
    }
  </style>
</head>

<body>
  <div class="topnav">
    <h1>Relay ESP8266 (RTC)</h1>
  </div>

  <!-- -------------------------------------------------------------------- -->

  <div class="content">
    <div class="cards" style="max-width: 100&#37;;">
      <div class="card">
        <p>TIME - <span id="time">%TIME%</span></p>
        <p>DATE - <span id="date">%DATE%</span></p>
      </div>
      <div class="center">
        <button class="btn" style="height: 100&#37;; width: 100&#37;;" onclick="updateTime()">
          <h3>Update Time</h3>
        </button>
      </div>
    </div>
  </div>

  <!-- -------------------------------------------------------------------- -->

  <div class="content">
    <div class="cards" style="max-width: 100&#37;;">
      %RELAY%
      <!-- ---------------------------------------------------------------- -->
      <!-- <div class="card">
        <h3>RELAY 1 - <span id="relay1">statusRelay1</span></h3>
        <label class="switch">
          <input type="checkbox" onchange="toggleCheckbox(this, 'relay1')" id='1' stateCheckbox(RELAY_1)>
          <span class="slider"></span>
        </label>
      </div>
      <div class="card">
        <h3>RELAY 2 - <span id="relay2">statusRelay2</span></h3>
        <label class="switch">
          <input type="checkbox" onchange="toggleCheckbox(this, 'relay2')" id='2' stateCheckbox(RELAY_2)>
          <span class="slider"></span>
        </label>
      </div>
      <div class="card">
        <h3>RELAY 3 - <span id="relay3">statusRelay3</span></h3>
        <label class="switch">
          <input type="checkbox" onchange="toggleCheckbox(this, 'relay3')" id='3' stateCheckbox(RELAY_3)>
          <span class="slider"></span>
        </label>
      </div>
      <div class="card">
        <h3>RELAY 4 - <span id="relay4">statusRelay4</span></h3>
        <label class="switch">
          <input type="checkbox" onchange="toggleCheckbox(this, 'relay4')" id='4' stateCheckbox(RELAY_4)>
          <span class="slider"></span>
        </label>
      </div> -->
      <!-- ---------------------------------------------------------------- -->
    </div>
  </div>

  <!-- -------------------------------------------------------------------- -->

  <script>
    /* Get data TIME from RTC */
    setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("time").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/time", true);
      xhttp.send();
    }, 1000);

    /* Get data DATE from RTC */
    setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("date").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/date", true);
      xhttp.send();
    }, 1000);

    /* Update TIME and DATE for RTC, by send data to ESP */
    function updateTime() {
      let text = "You want real-time updates for RTC?";
      if (confirm(text) == true) {
        const rtc = new Date();
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/rtc?Y=" + rtc.getFullYear()
          + "&M=" + (rtc.getMonth() + 1)
          + "&D=" + rtc.getDate()
          + "&h=" + rtc.getHours()
          + "&m=" + rtc.getMinutes()
          + "&s=" + rtc.getSeconds(), true);
        xhr.send();
      }
    }

    /* Send data state of RELAY (HTTP GET) to ESP */
    function toggleCheckbox(element, relay) {
      var stateRelay = document.getElementById(relay); // Get ID of that Relay
      var xhr = new XMLHttpRequest();
      if (element.checked) {
        stateRelay.innerHTML = "ON"; // Display state of that Relay
        xhr.open("GET", "/update?output=" + element.id + "&state=0", true);
      }
      else {
        stateRelay.innerHTML = "OFF"; // Display state of that Relay
        xhr.open("GET", "/update?output=" + element.id + "&state=1", true);
      }
      xhr.send();
    };

    /* Web Server using Server-Sent Events */
    if (!!window.EventSource) {
      var source = new EventSource('/events');

      /* ------------------------------------------------------------------- */

      source.addEventListener('open', function (e) {
        console.log("Events Connected");
      }, false);

      source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Events Disconnected");
        }
      }, false);

      /* ------------------------------------------------------------------- */

      source.addEventListener('relay1', function (e) {
        document.getElementById("relay1").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("1").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("1").checked = false;
        }
      }, false);

      source.addEventListener('relay2', function (e) {
        document.getElementById("relay2").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("2").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("2").checked = false;
        }
      }, false);

      source.addEventListener('relay3', function (e) {
        document.getElementById("relay3").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("3").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("3").checked = false;
        }
      }, false);

      source.addEventListener('relay4', function (e) {
        document.getElementById("relay4").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("4").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("4").checked = false;
        }
      }, false);
    }
  </script>
</body>

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
    relays += "<div class=\"card\"><h3>RELAY 1 - <span id=\"relay1\">" + statusRelay1 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay1')\" id='1' " + stateCheckbox(RELAY_1) + "><span class=\"slider\"></span></label></div>";
    relays += "<div class=\"card\"><h3>RELAY 2 - <span id=\"relay2\">" + statusRelay2 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay2')\" id='2' " + stateCheckbox(RELAY_2) + "><span class=\"slider\"></span></label></div>";
    relays += "<div class=\"card\"><h3>RELAY 3 - <span id=\"relay3\">" + statusRelay3 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay3')\" id='3' " + stateCheckbox(RELAY_3) + "><span class=\"slider\"></span></label></div>";
    relays += "<div class=\"card\"><h3>RELAY 4 - <span id=\"relay4\">" + statusRelay4 + "</span></h3><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this, 'relay4')\" id='4' " + stateCheckbox(RELAY_4) + "><span class=\"slider\"></span></label></div>";
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
                    ** And set reconnect delay to 1 second
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

  /* Send a GET request to <ESP_IP>/rtc?Y=<Y>&M=<M>&D=<D>&h=<h>&m=<m>&s=<s> */
  server.on("/rtc", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String Y, M, D, h, m, s;

              /* GET value of "Y", "M", "D", "h", "m", "s" */
              if (request->hasParam("Y") && request->hasParam("M") && request->hasParam("D") && request->hasParam("h") && request->hasParam("m") && request->hasParam("s"))
              {
                Y = request->getParam("Y")->value();
                M = request->getParam("M")->value();
                D = request->getParam("D")->value();
                h = request->getParam("h")->value();
                m = request->getParam("m")->value();
                s = request->getParam("s")->value();

                rtc.adjust(DateTime(Y.toInt(), M.toInt(), D.toInt(), h.toInt(), m.toInt(), s.toInt()));
              }
              else
              {
                Y = "No message sent";
                M = "No message sent";
                D = "No message sent";
                h = "No message sent";
                m = "No message sent";
                s = "No message sent";
              }

              DEBUG_PRINT(h); DEBUG_PRINT(":"); DEBUG_PRINT(m); DEBUG_PRINT(":"); DEBUG_PRINTLN(s);
              DEBUG_PRINT(D); DEBUG_PRINT("/"); DEBUG_PRINT(M); DEBUG_PRINT("/"); DEBUG_PRINTLN(Y);

              request->send(200, "text/plain", "OK"); });

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
                case 1:
                  (inputMessage2.toInt()) ? (statusRelay[0] = true) : (statusRelay[0] = false);
                  TRIGGER_RELAY_1;
                  //
                  statusRelay1 = stateRelay(RELAY_1);
                  events.send(statusRelay1.c_str(), "relay1", millis()); // Send Events to the Web Server
                  break;
                case 2:
                  (inputMessage2.toInt()) ? (statusRelay[1] = true) : (statusRelay[1] = false);
                  TRIGGER_RELAY_2;
                  //
                  statusRelay2 = stateRelay(RELAY_2);
                  events.send(statusRelay2.c_str(), "relay2", millis()); // Send Events to the Web Server
                  break;
                case 3:
                  (inputMessage2.toInt()) ? (statusRelay[2] = true) : (statusRelay[2] = false);
                  TRIGGER_RELAY_3;
                  //
                  statusRelay3 = stateRelay(RELAY_3);
                  events.send(statusRelay3.c_str(), "relay3", millis()); // Send Events to the Web Server
                  break;
                case 4:
                  (inputMessage2.toInt()) ? (statusRelay[3] = true) : (statusRelay[3] = false);
                  TRIGGER_RELAY_4;
                  //
                  statusRelay4 = stateRelay(RELAY_4);
                  events.send(statusRelay4.c_str(), "relay4", millis()); // Send Events to the Web Server
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
      events.send(statusRelay1.c_str(), "relay1", millis());
      break;
    case '2':
      statusRelay[1] = !statusRelay[1];
      TRIGGER_RELAY_2;
      statusRelay2 = stateRelay(RELAY_2);

      /* Send Events to the Web Server */
      events.send(statusRelay2.c_str(), "relay2", millis());
      break;
    case '3':
      statusRelay[2] = !statusRelay[2];
      TRIGGER_RELAY_3;
      statusRelay3 = stateRelay(RELAY_3);

      /* Send Events to the Web Server */
      events.send(statusRelay3.c_str(), "relay3", millis());
      break;
    case '4':
      statusRelay[3] = !statusRelay[3];
      TRIGGER_RELAY_4;
      statusRelay4 = stateRelay(RELAY_4);

      /* Send Events to the Web Server */
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
