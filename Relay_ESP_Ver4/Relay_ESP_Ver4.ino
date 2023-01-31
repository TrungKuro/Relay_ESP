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

#define NUMBER_RELAYS 4

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

bool statusRelay[NUMBER_RELAYS] = {
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

/* --------------------------------- Timer --------------------------------- */

byte hourAfter[NUMBER_RELAYS] = {
    0, 0, 0, 0};
byte minuteAfter[NUMBER_RELAYS] = {
    0, 0, 0, 0};
byte secondAfter[NUMBER_RELAYS] = {
    0, 0, 0, 0};

bool trigTimer[NUMBER_RELAYS] = {
    true, true, true, true}; // true = "OFF" ; false = "ON"

bool statusTimer[NUMBER_RELAYS] = {
    false, // Timer 1
    false, // Timer 2
    false, // Timer 3
    false  // Timer 4
};         // Turn-off all Timers

uint32_t flagTimer[NUMBER_RELAYS] = {
    0, 0, 0, 0};

/* -------------------------------- Calendar ------------------------------- */

/* (-1) mean "null" */
byte hourON[NUMBER_RELAYS] = {
    -1, -1, -1, -1};
byte hourOFF[NUMBER_RELAYS] = {
    -1, -1, -1, -1};
byte minuteON[NUMBER_RELAYS] = {
    -1, -1, -1, -1};
byte minuteOFF[NUMBER_RELAYS] = {
    -1, -1, -1, -1};

bool statusCalendar[NUMBER_RELAYS] = {
    false, // Calendar 1
    false, // Calendar 2
    false, // Calendar 3
    false  // Calendar 4
};         // Turn-off all Calendars

bool flagCalendarON[NUMBER_RELAYS] = {
    false, false, false, false}; // true = "Wait for Activation"

bool flagCalendarOFF[NUMBER_RELAYS] = {
    false, false, false, false}; // true = "Wait for Activation"

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
body {
margin: 0px auto;
padding-bottom: 5px;
}
p {
display: inline-block;
text-align: justify;
font-size: 1rem;
line-height: 175%;
}
.topnav {
overflow: hidden;
background-color: #00ABB3;
color: white;
font-size: 1rem;
}
.content {
padding: 20px;
}
.cards {
margin: 0px auto;
display: grid;
grid-gap: 1.5rem;
grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
}
.card {
padding: 10px;
background-color: white;
box-shadow: 2px 2px 12px 1px rgba(178, 178, 178, .5);
}
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
.center {
display: flex;
justify-content: center;
align-items: center;
}
.btnTime {
background-color: #EAEAEA;
color: black;
cursor: pointer;
font-size: 16px;
padding: 16px 30px;
border: none;
border-radius: 5px;
text-align: center;
}
.btnTime:hover,
.btnTime:active {
background-color: #3C4048;
color: white;
}
.collapsible {
background-color: #EAEAEA;
color: black;
cursor: pointer;
font-size: 16px;
padding: 18px;
border: none;
text-align: left;
outline: none;
display: flex;
justify-content: space-between;
align-items: center;
}
.active,
.collapsible:hover,
.collapsible:active {
background-color: #3C4048;
color: white;
}
.collapsible:after {
content: "\002B";
color: black;
font-size: 32px;
font-weight: bold;
float: right;
margin-left: 15px;
}
.collapsible:hover::after,
.collapsible:active::after {
color: white;
}
.active:after {
content: "\2212";
color: white;
}
.contentAdv {
max-height: 0;
overflow: hidden;
transition: max-height .8s ease-out;
background-color: #EAEAEA;
}
.cardsAdv {
display: grid;
grid-gap: 1.5rem;
grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
margin: 18px;
}
.cardAdv {
background-color: white;
box-shadow: 2px 2px 12px 1px rgba(60, 64, 72, .5);
}
.titleAdv {
background-color: #F3EFE0;
display: inline-block;
}
.textAdv {
padding: 10px;
}
input[type=number] {
width: 36px;
border: none;
border-bottom: 1px solid #B2B2B2;
font-family: Arial;
font-size: 1rem;
text-align: center;
}
input[type=number]::-webkit-inner-spin-button,
input[type=number]::-webkit-outer-spin-button {
opacity: 1;
}
input[type=radio]:after {
width: 15px;
height: 15px;
border-radius: 15px;
top: -3px;
left: -3px;
position: relative;
content: '';
display: inline-block;
visibility: visible;
cursor: pointer;
background-color: #EAEAEA;
border: 2px solid #B2B2B2;
}
input[type=radio]:checked:after {
background-color: #3C4048;
}
input[type=time] {
border: none;
border-bottom: 1px solid #B2B2B2;
font-family: Arial;
font-size: 1rem;
text-align: center;
}
</style>
</head>
<body>
<div class="topnav">
<h1>RELAY ESP8266 (RTC)</h1>
</div>
<div class="content">
<div class="cards" style="max-width: 100&#37;;">
<div class="card">
<p style="text-align: center;">
TIME - <span id="time">%TIME%</span>
<br>
DATE - <span id="date">%DATE%</span>
</p>
</div>
<div class="center">
<button class="btnTime" style="height: 100&#37;; width: 100&#37;;" onclick="updateTime()">
<h3>Update Time</h3>
</button>
</div>
</div>
</div>
<div class="content">
<div class="cards" style="max-width: 100&#37;;">
%RELAY%
</div>
</div>
<hr>
<div class="content">
<button class="collapsible" style="width: 100&#37;;">
<h3>Advanced Settings:</h3>
</button>
<div class="contentAdv">
<button class="collapsible" style="width: 100&#37;;">
<h3>Device 1</h3>
</button>
<div class="contentAdv">
<div class="cardsAdv" style="max-width: 100&#37;;">
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>TIMER</h3>
</div>
<div class="textAdv">
%TIMER1%
</div>
</div>
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>CALENDAR</h3>
</div>
<div class="textAdv">
%CALENDAR1%
</div>
</div>
</div>
</div>
<button class="collapsible" style="width: 100&#37;;">
<h3>Device 2</h3>
</button>
<div class="contentAdv">
<div class="cardsAdv" style="max-width: 100&#37;;">
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>TIMER</h3>
</div>
<div class="textAdv">
%TIMER2%
</div>
</div>
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>CALENDAR</h3>
</div>
<div class="textAdv">
%CALENDAR2%
</div>
</div>
</div>
</div>
<button class="collapsible" style="width: 100&#37;;">
<h3>Device 3</h3>
</button>
<div class="contentAdv">
<div class="cardsAdv" style="max-width: 100&#37;;">
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>TIMER</h3>
</div>
<div class="textAdv">
%TIMER3%
</div>
</div>
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>CALENDAR</h3>
</div>
<div class="textAdv">
%CALENDAR3%
</div>
</div>
</div>
</div>
<button class="collapsible" style="width: 100&#37;;">
<h3>Device 4</h3>
</button>
<div class="contentAdv">
<div class="cardsAdv" style="max-width: 100&#37;;">
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>TIMER</h3>
</div>
<div class="textAdv">
%TIMER4%
</div>
</div>
<div class="cardAdv">
<div class="titleAdv" style="width: 100&#37;;">
<h3>CALENDAR</h3>
</div>
<div class="textAdv">
%CALENDAR4%
</div>
</div>
</div>
</div>
</div>
</div>
<script>
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
source.addEventListener('timer', function (e) {
if (e.data[0] == "t") {
document.getElementById(e.data).checked = false;
}
else {
document.getElementById("t" + e.data).checked = true;
}
}, false);
source.addEventListener('calendar', function (e) {
if (e.data[0] == "c") {
document.getElementById(e.data).checked = false;
}
else {
document.getElementById("c" + e.data).checked = true;
}
}, false);
}
var coll = document.getElementsByClassName("collapsible");
coll[0].addEventListener("click", function () {
this.classList.toggle("active");
var content = this.nextElementSibling;
if (content.style.maxHeight) {
coll[0].style.removeProperty("background-color");
content.style.maxHeight = null;
} else {
coll[0].style.backgroundColor = "#00ABB3";
var height = 0;
for (var i = 1; i < coll.length; i++) {
height = height + coll[i].nextElementSibling.scrollHeight;
}
content.style.maxHeight = (content.scrollHeight + height) + "px";
}
});
for (var i = 1; i < coll.length; i++) {
coll[i].addEventListener("click", function () {
this.classList.toggle("active");
var content = this.nextElementSibling;
if (content.style.maxHeight) {
content.style.maxHeight = null;
} else {
content.style.maxHeight = content.scrollHeight + "px";
}
});
}
function toggleTimer(element, device) {
var xhr = new XMLHttpRequest();
if (element.checked) {
var h = document.getElementById("h" + device).value;
var m = document.getElementById("m" + device).value;
var s = document.getElementById("s" + device).value;
var trig = "";
if (document.getElementById("on" + device).checked) {
trig = document.getElementById("on" + device).value;
}
else {
trig = document.getElementById("off" + device).value;
}
xhr.open("GET", "/timerON?device=" + device + "&h=" + h + "&m=" + m + "&s=" + s + "&trig=" + trig, true);
}
else {
xhr.open("GET", "/timerOFF?device=" + device, true);
}
xhr.send();
};
function toggleCalendar(element, device) {
var xhr = new XMLHttpRequest();
if (element.checked) {
var onH = document.getElementById("calOn" + device).value.slice(0, 2);
var onM = document.getElementById("calOn" + device).value.slice(3, 5);
var offH = document.getElementById("calOff" + device).value.slice(0, 2);
var offM = document.getElementById("calOff" + device).value.slice(3, 5);
xhr.open("GET", "/calendarON?device=" + device + "&onH=" + onH + "&onM=" + onM + "&offH=" + offH + "&offM=" + offM, true);
}
else {
xhr.open("GET", "/calendarOFF?device=" + device, true);
}
xhr.send();
};
</script>
</body>
</html>
)rawliteral";

/* Used to store temporary strings of information */
String temp = "";

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

/* ------------------------------------------------------------------------- */

/* Handling the state of Radio ON Timer */
String stateTrigON(bool state)
{
  if (state)
  {
    return ""; // Mean not choose ON
  }
  else
  {
    return "checked"; // Mean choose ON
  }
}

/* Handling the state of Radio OFF Timer */
String stateTrigOFF(bool state)
{
  if (state)
  {
    return "checked"; // Mean choose OFF
  }
  else
  {
    return ""; // Mean not choose OFF
  }
}

/* Handling the state of Slider Timer */
String stateTimer(bool state)
{
  if (state)
  {
    return "checked"; // Mean turn on Timer
  }
  else
  {
    return ""; // Mean turn off Timer
  }
}

/* ------------------------------------------------------------------------- */

/* Handling the time ON of Calendar */
String timeON(byte addr)
{
  String on = "";

  if (hourON[addr] >= 10)
  {
    on += String(hourON[addr]) + ":";
  }
  else if (hourON[addr] >= 0)
  {
    on += "0" + String(hourON[addr]) + ":";
  }

  if (minuteON[addr] >= 10)
  {
    on += String(minuteON[addr]);
  }
  else if (minuteON[addr] >= 0)
  {
    on += "0" + String(minuteON[addr]);
  }

  return on;
}

/* Handling the time OFF of Calendar */
String timeOFF(byte addr)
{
  String off = "";

  if (hourOFF[addr] >= 10)
  {
    off += String(hourOFF[addr]) + ":";
  }
  else if (hourOFF[addr] >= 0)
  {
    off += "0" + String(hourOFF[addr]) + ":";
  }

  if (minuteOFF[addr] >= 10)
  {
    off += String(minuteOFF[addr]);
  }
  else if (minuteOFF[addr] >= 0)
  {
    off += "0" + String(minuteOFF[addr]);
  }

  return off;
}

/* Handling the state of Slider Calendar */
String stateCalendar(bool state)
{
  if (state)
  {
    return "checked"; // Mean turn on Calendar
  }
  else
  {
    return ""; // Mean turn off Calendar
  }
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

  /* -------------------- Timer check is enabled or not? ------------------- */

  /* Timer 1 */
  if (statusTimer[0])
  {
    if (now.unixtime() >= flagTimer[0])
    {
      statusTimer[0] = false; // Clear Flag

      statusRelay[0] = trigTimer[0];
      TRIGGER_RELAY_1;
      statusRelay1 = stateRelay(RELAY_1);

      /* Send Events to "sync" with other Clients */
      events.send(statusRelay1.c_str(), "relay1", millis());

      /* Send Events to the Web Server */
      temp = "t1";
      events.send(temp.c_str(), "timer", millis()); // Turn-off Timer 1 !!!DEBUG
    }
  }

  /* Timer 2 */
  if (statusTimer[1])
  {
    //
  }

  /* Timer 3 */
  if (statusTimer[2])
  {
    //
  }

  /* Timer 4 */
  if (statusTimer[3])
  {
    //
  }

  /* ------------------ Calendar check is enabled or not? ------------------ */

  /* Calendar 1 */
  if (statusCalendar[0])
  {
    if (flagCalendarON[0])
    {
      if (now.hour() == hourON[0] && now.minute() == minuteON[0])
      {
        flagCalendarON[0] = false; // Clear Flag

        statusRelay[0] = false; // Turn-on Relay
        TRIGGER_RELAY_1;
        statusRelay1 = stateRelay(RELAY_1);

        /* Send Events to "sync" with other Clients */
        events.send(statusRelay1.c_str(), "relay1", millis());
      }
    }

    if (flagCalendarOFF[0])
    {
      if (now.hour() == hourOFF[0] && now.minute() == minuteOFF[0])
      {
        flagCalendarOFF[0] = false; // Clear Flag

        statusRelay[0] = true; // Turn-off Relay
        TRIGGER_RELAY_1;
        statusRelay1 = stateRelay(RELAY_1);

        /* Send Events to "sync" with other Clients */
        events.send(statusRelay1.c_str(), "relay1", millis());
      }
    }

    if (!flagCalendarON[0] && !flagCalendarOFF[0])
    {
      statusCalendar[0] = false;

      /* Send Events to the Web Server */
      temp = "c1";
      events.send(temp.c_str(), "calendar", millis()); // Turn-off Calendar 1 !!!DEBUG
    }
  }

  /* Calendar 2 */
  if (statusCalendar[1])
  {
    //
  }

  /* Calendar 3 */
  if (statusCalendar[2])
  {
    //
  }

  /* Calendar 4 */
  if (statusCalendar[3])
  {
    //
  }
}

/* ------------------------------------------------------------------------- */

/* Run first and only once every time you visit or reload the site
   Replace placeholder with RTC values and state Relay */
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
  else if (var == "TIMER1")
  {
    String timer = "";
    timer += "<p>. After <input type=\"number\" id=\"h1\" value=\"" + String(hourAfter[0]);
    timer += "\" >H:<input type=\"number\" id=\"m1\" value=\"" + String(minuteAfter[0]);
    timer += "\" >M:<input type=\"number\" id=\"s1\" value=\"" + String(secondAfter[0]);
    timer += "\" >S<br>. Then ON <input type=\"radio\" id=\"on1\" name=\"timer1\" value=\"0\" " + stateTrigON(trigTimer[0]);
    timer += "> or OFF <input type=\"radio\" id=\"off1\" name=\"timer1\" value=\"1\" " + stateTrigOFF(trigTimer[0]);
    timer += "></p><hr><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleTimer(this, '1')\" id=\"t1\" " + stateTimer(statusTimer[0]);
    timer += "><span class=\"slider\"></span></label>";
    return timer;
  }
  else if (var == "CALENDAR1")
  {
    String calendar = "";
    calendar += "<p>. ON at <input type=\"time\" id=\"calOn1\" value=\"" + timeON(0);
    calendar += "\"><br>. OFF at <input type=\"time\" id=\"calOff1\" value=\"" + timeOFF(0);
    calendar += "\"></p><hr><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCalendar(this, '1')\" id=\"c1\" " + stateCalendar(statusCalendar[0]);
    calendar += "><span class=\"slider\"></span></label>";
    return calendar;
  }
  else if (var == "TIMER2")
  {
    return "."; //!!!DEBUG
  }
  else if (var == "CALENDAR2")
  {
    return "."; //!!!DEBUG
  }
  else if (var == "TIMER3")
  {
    return "."; //!!!DEBUG
  }
  else if (var == "CALENDAR3")
  {
    return "."; //!!!DEBUG
  }
  else if (var == "TIMER4")
  {
    return "."; //!!!DEBUG
  }
  else if (var == "CALENDAR4")
  {
    return "."; //!!!DEBUG
  }

  return String();
}

/* ------------------------------------------------------------------------- */

/* Calculating when to activate the Flags for Timer */
void calculateFlagTimer(byte addr)
{
  DateTime now = rtc.now();

  /* Calculate a date which is ? hours, ? minutes, and ? seconds into the future */
  DateTime future(now + TimeSpan(0, hourAfter[addr], minuteAfter[addr], secondAfter[addr]));
  flagTimer[addr] = future.unixtime();
}

/* Calculating when to activate the Flags for Calendar */
void calculateFlagCalendar(byte addr)
{
  DateTime now = rtc.now();

  /* Calendar ON */
  flagCalendarON[addr] = false;
  if (now.hour() < hourON[addr])
  {
    flagCalendarON[addr] = true;
  }
  else if (now.hour() == hourON[addr] && now.minute() < minuteON[addr])
  {
    flagCalendarON[addr] = true;
  }

  /* Calendar OFF */
  flagCalendarOFF[addr] = false;
  if (now.hour() < hourOFF[addr])
  {
    flagCalendarOFF[addr] = true;
  }
  else if (now.hour() == hourOFF[addr] && now.minute() < minuteOFF[addr])
  {
    flagCalendarOFF[addr] = true;
  }
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
                case 1: // Relay 1
                  (inputMessage2.toInt()) ? (statusRelay[0] = true) : (statusRelay[0] = false);
                  TRIGGER_RELAY_1;
                  statusRelay1 = stateRelay(RELAY_1);

                  /* Send Events to "sync" with other Clients */
                  events.send(statusRelay1.c_str(), "relay1", millis());
                  break;
                case 2: // Relay 2
                  (inputMessage2.toInt()) ? (statusRelay[1] = true) : (statusRelay[1] = false);
                  TRIGGER_RELAY_2;
                  statusRelay2 = stateRelay(RELAY_2);

                  /* Send Events to "sync" with other Clients */
                  events.send(statusRelay2.c_str(), "relay2", millis());
                  break;
                case 3: // Relay 3
                  (inputMessage2.toInt()) ? (statusRelay[2] = true) : (statusRelay[2] = false);
                  TRIGGER_RELAY_3;
                  statusRelay3 = stateRelay(RELAY_3);

                  /* Send Events to "sync" with other Clients */
                  events.send(statusRelay3.c_str(), "relay3", millis());
                  break;
                case 4: // Relay 4
                  (inputMessage2.toInt()) ? (statusRelay[3] = true) : (statusRelay[3] = false);
                  TRIGGER_RELAY_4;
                  statusRelay4 = stateRelay(RELAY_4);

                  /* Send Events to "sync" with other Clients */
                  events.send(statusRelay4.c_str(), "relay4", millis());
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

  /* ----------------------------------------------------------------------- */

  /* Send a GET request to <ESP_IP>/timerON?device=<device>&h=<h>&m=<m>&s=<s>&trig=<trig> */
  server.on("/timerON", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device, h, m, s, trig;
              
              /* GET value of "device", "h", "m", "s", "trig" */
              if (request->hasParam("device") && request->hasParam("h") && request->hasParam("m") && request->hasParam("s") && request->hasParam("trig"))
              {
                device = request->getParam("device")->value();
                h = request->getParam("h")->value();
                m = request->getParam("m")->value();
                s = request->getParam("s")->value();
                trig = request->getParam("trig")->value();

                byte addr = device.toInt() - 1;

                hourAfter[addr] = h.toInt();
                minuteAfter[addr] = m.toInt();
                secondAfter[addr] = s.toInt();
                trigTimer[addr] = trig.toInt();

                statusTimer[addr] = true; // Enable Timer
                calculateFlagTimer(addr);

                temp = device;
                events.send(temp.c_str(), "timer", millis()); // Turn-on Timer !!!DEBUG
              }
              else
              {
                device = "No message sent";
                h = "No message sent";
                m = "No message sent";
                s = "No message sent";
                trig = "No message sent";
              }

              DEBUG_PRINT("Timer "); DEBUG_PRINT(device); DEBUG_PRINT(" ON - trig "); DEBUG_PRINTLN(trig);
              DEBUG_PRINT(h); DEBUG_PRINT(":"); DEBUG_PRINT(m); DEBUG_PRINT(":"); DEBUG_PRINTLN(s);
              
              request->send(200, "text/plain", "OK"); });

  /* Send a GET request to <ESP_IP>/timerOFF?device=<device> */
  server.on("/timerOFF", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device;
              
              /* GET value of "device" */
              if (request->hasParam("device"))
              {
                device = request->getParam("device")->value();
                statusTimer[device.toInt() - 1] = false; // Disable Timer

                temp = "t" + device;
                events.send(temp.c_str(), "timer", millis()); // Turn-off Timer !!!DEBUG
              }
              else
              {
                device = "No message sent";
              }

              DEBUG_PRINT("Timer "); DEBUG_PRINT(device); DEBUG_PRINTLN(" OFF");

              request->send(200, "text/plain", "OK"); });

  /* ----------------------------------------------------------------------- */

  /* Send a GET request to <ESP_IP>/calendarON?device=<device>&onH=<onH>&onM=<onM>&offH=<offH>&offM=<offM> */
  server.on("/calendarON", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device, onH, onM, offH, offM;

              /* GET value of "device", "onH", "onM", "offH", "offM" */
              if (request->hasParam("device") && request->hasParam("onH") && request->hasParam("onM") && request->hasParam("offH") && request->hasParam("offM"))
              {
                device = request->getParam("device")->value();
                onH = request->getParam("onH")->value();
                onM = request->getParam("onM")->value();
                offH = request->getParam("offH")->value();
                offM = request->getParam("offM")->value();

                byte addr = device.toInt() - 1;

                hourON[addr] = onH.toInt();
                minuteON[addr] = onM.toInt();
                hourOFF[addr] = offH.toInt();
                minuteOFF[addr] = offM.toInt();

                statusCalendar[addr] = true; // Enable Calendar
                calculateFlagCalendar(addr);

                temp = device;
                events.send(temp.c_str(), "calendar", millis()); // Turn-on Calendar !!!DEBUG
              }
              else
              {
                device = "No message sent";
                onH = "No message sent";
                onM = "No message sent";
                offH = "No message sent";
                offM = "No message sent";
              }

              DEBUG_PRINT("Calendar "); DEBUG_PRINT(device); DEBUG_PRINTLN(" ON");
              DEBUG_PRINT("on = "); DEBUG_PRINT(onH); DEBUG_PRINT(":"); DEBUG_PRINTLN(onM);
              DEBUG_PRINT("off = "); DEBUG_PRINT(offH); DEBUG_PRINT(":"); DEBUG_PRINTLN(offM);

              request->send(200, "text/plain", "OK"); });

  /* Send a GET request to <ESP_IP>/calendarOFF?device=<device> */
  server.on("/calendarOFF", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device;

              /* GET value of "device" */
              if (request->hasParam("device"))
              {
                device = request->getParam("device")->value();
                statusCalendar[device.toInt() - 1] = false; // Disable Calendar

                temp = "c" + device;
                events.send(temp.c_str(), "calendar", millis()); // Turn-off Calendar !!!DEBUG
              }
              else
              {
                device = "No message sent";
              }

              DEBUG_PRINT("Calendar "); DEBUG_PRINT(device); DEBUG_PRINTLN(" OFF");

              request->send(200, "text/plain", "OK"); });

  /* ----------------------------------------------------------------------- */

  /* Start Server */
  server.addHandler(&events);
  server.begin();
}

/* ------------------------------------------------------------------------- */

void loop()
{
  /* ----------------------------- Check Button ---------------------------- */

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

  /* --------------------- Check RTC (Timer / Calendar) -------------------- */

  if (millis() - checkRTC >= TIME_CHECK_RTC)
  {
    getRTCReadings();

    checkRTC = millis();
  }
}
