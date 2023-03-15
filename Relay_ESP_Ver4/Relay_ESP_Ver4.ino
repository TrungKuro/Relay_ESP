/* ------------------------------------------------------------------------- */
/*                                  LIBRARY                                  */
/* ------------------------------------------------------------------------- */

#include "Control_Relay.h"

/* ------------------------------------------------------------------------- */
/*                              VARIABLE BUTTON                              */
/* ------------------------------------------------------------------------- */

/**
 * Indicates the current state of the Button
 *
 * TRUE   : Button is pressing
 * FALSE  : Button is releasing (default)
 */
bool statusBtn = false;

/* Frequency of Checking Button */
unsigned long checkBtn;

/* ------------------------------------------------------------------------- */
/*                               VARIABLE RELAY                              */
/* ------------------------------------------------------------------------- */

/**
 * Indicates the current state of the Relay
 *
 * TRUE   : Relay is OFF (default)
 * FALSE  : Relay is ON
 */
bool statusRelays[4] = {true, true, true, true};

/**
 * To update the value of each Relay
 * To send to each CLIENT via "event" (SSE)
 */
String onRelay = "ON";
String offRelay = "OFF";

/* ------------------------------------------------------------------------- */
/*                                VARIABLE RTC                               */
/* ------------------------------------------------------------------------- */

RTC_DS1307 rtc;

/**
 * To store data from RTC
 * And to send to each CLIENT via "event" (SSE)
 */
String timeRTC = "hh:mm:ss";
String dateRTC = "DD/MM/YYYY";

/* RTC update frequency */
unsigned long checkRTC;

/* ------------------------------------------------------------------------- */
/*                          WIFI - ACCESS POINT (AP)                         */
/* ------------------------------------------------------------------------- */

/**
 * By default, the "Access Point" IP address is 192.168.4.1
 * But here I set it to ...
 */
IPAddress local_IP(IP_A, IP_B, IP_C, IP_D);
IPAddress gateway(GATEWAY_A, GATEWAY_B, GATEWAY_C, GATEWAY_D);
IPAddress subnet(SUBNET_A, SUBNET_B, SUBNET_C, SUBNET_D);

/* ------------------------------------------------------------------------- */
/*                              WEB SERVER (ESP)                             */
/* ------------------------------------------------------------------------- */

/* Create AsyncWebServer object on port 80 */
AsyncWebServer server(80);

/* Create an Event Source on "/e" */
AsyncEventSource events(URL_EVENT);

/* ------------------------------------------------------------------------- */
/*                     WEB PAGE (HTML + CSS + JAVASCRIPT)                    */
/* ------------------------------------------------------------------------- */

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
      <!-- <div class='card'>
        <h3>RELAY 1 - <span id='relay1'>stateRelay(RELAY_1)</span></h3>
        <label class='switch'>
          <input type='checkbox' onchange="toggleCheckbox(this, 'relay1')" id='1' stateCheckbox(RELAY_1)>
          <span class='slider'></span>
        </label>
      </div>
      <div class='card'>
        <h3>RELAY 2 - <span id='relay2'>stateRelay(RELAY_2)</span></h3>
        <label class='switch'>
          <input type='checkbox' onchange="toggleCheckbox(this, 'relay2')" id='2' stateCheckbox(RELAY_2)>
          <span class='slider'></span>
        </label>
      </div>
      <div class='card'>
        <h3>RELAY 3 - <span id='relay3'>stateRelay(RELAY_3)</span></h3>
        <label class='switch'>
          <input type='checkbox' onchange="toggleCheckbox(this, 'relay3')" id='3' stateCheckbox(RELAY_3)>
          <span class='slider'></span>
        </label>
      </div>
      <div class='card'>
        <h3>RELAY 4 - <span id='relay4'>stateRelay(RELAY_4)</span></h3>
        <label class='switch'>
          <input type='checkbox' onchange="toggleCheckbox(this, 'relay4')" id='4' stateCheckbox(RELAY_4)>
          <span class='slider'></span>
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

    /* --------------------------------------------------------------------- */

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
    };

    /* --------------------------------------------------------------------- */

    /* Send data state of RELAY (HTTP GET) to ESP */
    function toggleCheckbox(element, relay) {
      var stateRelay = document.getElementById(relay); // Get ID of that Relay
      var xhr = new XMLHttpRequest();
      if (element.checked) {
        stateRelay.innerHTML = "ON"; // Display state of that Relay
        xhr.open("GET", "/r?n=" + element.id + "&t=0", true);
      }
      else {
        stateRelay.innerHTML = "OFF"; // Display state of that Relay
        xhr.open("GET", "/r?n=" + element.id + "&t=1", true);
      }
      xhr.send();
    };

    /* Web Server using Server-Sent Events */
    if (!!window.EventSource) {
      var source = new EventSource('/e');

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

      source.addEventListener("1", function (e) {
        document.getElementById("relay1").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("1").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("1").checked = false;
        }
      }, false);

      source.addEventListener("2", function (e) {
        document.getElementById("relay2").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("2").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("2").checked = false;
        }
      }, false);

      source.addEventListener("3", function (e) {
        document.getElementById("relay3").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("3").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("3").checked = false;
        }
      }, false);

      source.addEventListener("4", function (e) {
        document.getElementById("relay4").innerHTML = e.data;
        if (e.data == "ON") {
          document.getElementById("4").checked = true;
        }
        else if (e.data == "OFF") {
          document.getElementById("4").checked = false;
        }
      }, false);
    };
  </script>
</body>

</html>
)rawliteral";

/* ------------------------------------------------------------------------- */
/*                                   CONFIG                                  */
/* ------------------------------------------------------------------------- */

void setup()
{
  /* To use the Debug feature */
  DEBUG_BEGIN(115200);

  /* Initialize LED, and turn off it */
  pinMode(LED, OUTPUT);
  LED_OFF;

  /* Initialize Relay with initial default state */
  pinMode(RELAY_1, OUTPUT);
  TRIG_RELAY_1;
  pinMode(RELAY_2, OUTPUT);
  TRIG_RELAY_2;
  pinMode(RELAY_3, OUTPUT);
  TRIG_RELAY_3;
  pinMode(RELAY_4, OUTPUT);
  TRIG_RELAY_4;

  /* Initialize RTC */
  Wire.begin(SDA, SCL);
  rtc.begin();

  /* Setting the ESP8266 as an Access Point (AP) */
  DEBUG_PRINT(F("Setting soft-AP configuration ... "));
  if (WiFi.softAPConfig(local_IP, gateway, subnet))
    DEBUG_PRINTLN(F("Ready"));
  else
    DEBUG_PRINTLN(F("Failed!"));
  DEBUG_PRINT(F("Setting soft-AP ... "));
  if (WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, CHANNEL, SSID_HIDDEN, MAX_CONNECTION))
    DEBUG_PRINTLN(F("Ready"));
  else
    DEBUG_PRINTLN(F("Failed!"));
  DEBUG_PRINT(F("Soft-AP IP address = "));
  IPAddress IP = WiFi.softAPIP();
  DEBUG_PRINTLN(IP);

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
                Y = F("No message sent");
                M = F("No message sent");
                D = F("No message sent");
                h = F("No message sent");
                m = F("No message sent");
                s = F("No message sent");
              }

              DEBUG_PRINT(h); DEBUG_PRINT(F(":")); DEBUG_PRINT(m); DEBUG_PRINT(F(":")); DEBUG_PRINTLN(s);
              DEBUG_PRINT(D); DEBUG_PRINT(F("/")); DEBUG_PRINT(M); DEBUG_PRINT(F("/")); DEBUG_PRINTLN(Y);

              request->send(200, "text/plain", "OK"); });

  /* ----------------------------------------------------------------------- */

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
   *
   * Change:
   * <ESP_IP>/r?n=<mes1>&t=<mes2>
   */
  server.on(URL_RELAY, HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String mes1;
              String mes2;

              /* GET value of "numerical" and "triggered" */
              if (request->hasParam(PARAM_NUM) && request->hasParam(PARAM_TRIG))
              {
                mes1 = request->getParam(PARAM_NUM)->value();
                mes2 = request->getParam(PARAM_TRIG)->value();

                switch (mes1.toInt()) // Update for "statusRelay"
                {
                case 1:
                {
                  (mes2.toInt()) ? (statusRelays[0] = true) : (statusRelays[0] = false);
                  TRIG_RELAY_1;
                  /* Send Events to the Web Server */
                  if (statusRelays[0])
                    events.send(offRelay.c_str(), EVENT_RELAY_1, millis());
                  else
                    events.send(onRelay.c_str(), EVENT_RELAY_1, millis());
                  break;
                }
                case 2:
                {
                  (mes2.toInt()) ? (statusRelays[1] = true) : (statusRelays[1] = false);
                  TRIG_RELAY_2;
                  /* Send Events to the Web Server */
                  if (statusRelays[1])
                    events.send(offRelay.c_str(), EVENT_RELAY_2, millis());
                  else
                    events.send(onRelay.c_str(), EVENT_RELAY_2, millis());
                  break;
                }
                case 3:
                {
                  (mes2.toInt()) ? (statusRelays[2] = true) : (statusRelays[2] = false);
                  TRIG_RELAY_3;
                  /* Send Events to the Web Server */
                  if (statusRelays[2])
                    events.send(offRelay.c_str(), EVENT_RELAY_3, millis());
                  else
                    events.send(onRelay.c_str(), EVENT_RELAY_3, millis());
                  break;
                }
                case 4:
                {
                  (mes2.toInt()) ? (statusRelays[3] = true) : (statusRelays[3] = false);
                  TRIG_RELAY_4;
                  /* Send Events to the Web Server */
                  if (statusRelays[3])
                    events.send(offRelay.c_str(), EVENT_RELAY_4, millis());
                  else
                    events.send(onRelay.c_str(), EVENT_RELAY_4, millis());
                  break;
                }
                }
              }
              else
              {
                mes1 = F("No message sent");
                mes2 = F("No message sent");
              }

              DEBUG_PRINT(F("GPIO: "));
              DEBUG_PRINT(mes1);
              DEBUG_PRINT(F(" - Set to: "));
              DEBUG_PRINTLN(mes2);

              request->send(200, "text/plain", "OK"); });

  /* Start Server */
  server.addHandler(&events);
  server.begin();
}

/* ------------------------------------------------------------------------- */
/*                                    MAIN                                   */
/* ------------------------------------------------------------------------- */

void loop()
{
  if (millis() - checkBtn >= TIME_CHECK_BTN)
  {
    switch (detectButton(&statusBtn))
    {
    case BTN_1:
    {
      statusRelays[0] = !statusRelays[0];
      TRIG_RELAY_1;
      /* Send Events to the Web Server */
      if (statusRelays[0])
        events.send(offRelay.c_str(), EVENT_RELAY_1, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_1, millis());
      break;
    }
    case BTN_2:
    {
      statusRelays[1] = !statusRelays[1];
      TRIG_RELAY_2;
      /* Send Events to the Web Server */
      if (statusRelays[1])
        events.send(offRelay.c_str(), EVENT_RELAY_2, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_2, millis());
      break;
    }
    case BTN_3:
    {
      statusRelays[2] = !statusRelays[2];
      TRIG_RELAY_3;
      /* Send Events to the Web Server */
      if (statusRelays[2])
        events.send(offRelay.c_str(), EVENT_RELAY_3, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_3, millis());
      break;
    }
    case BTN_4:
    {
      statusRelays[3] = !statusRelays[3];
      TRIG_RELAY_4;
      /* Send Events to the Web Server */
      if (statusRelays[3])
        events.send(offRelay.c_str(), EVENT_RELAY_4, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_4, millis());
      break;
    }
    }

    checkBtn = millis();
  }

  /* ----------------------------------------------------------------------- */

  if (millis() - checkRTC >= TIME_CHECK_RTC)
  {
    /* Get data from RTC */
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

    timeRTC = F("hh:mm:ss");
    timeRTC = now.toString(&timeRTC[0]);
    DEBUG_PRINTLN(timeRTC);

    dateRTC = F("DD/MM/YYYY");
    dateRTC = now.toString(&dateRTC[0]);
    DEBUG_PRINTLN(dateRTC);

    checkRTC = millis();
  }
}
