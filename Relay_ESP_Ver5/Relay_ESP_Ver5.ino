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
bool statusRelays[NUMBER_RELAYS] = {true, true, true, true};

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
/*                               VARIABLE TIMER                              */
/* ------------------------------------------------------------------------- */

/* To store time data to "trigger" the Relay of Timer */
byte hourAfter[NUMBER_RELAYS] = {0, 0, 0, 0};
byte minuteAfter[NUMBER_RELAYS] = {0, 0, 0, 0};
byte secondAfter[NUMBER_RELAYS] = {0, 0, 0, 0};

/**
 * Indicates the user setting the Timer to ON or OFF the Relay
 *
 * TRUE   : will OFF Relay
 * FALSE  : will ON Relay
 */
bool trigTimer[NUMBER_RELAYS] = {true, true, true, true};

/**
 * Indicates the current state of the Timer
 *
 * TRUE   : Timer is ON
 * FALSE  : Timer is OFF (default)
 */
bool statusTimer[NUMBER_RELAYS] = {false, false, false, false};

/* Calculate the time in the future to run the Timer (Unix Time Stamp) */
uint32_t flagTimer[NUMBER_RELAYS] = {0, 0, 0, 0};

/**
 * To update each Timer
 * To send to each CLIENT via "event" (SSE)
 */
String offTimer[NUMBER_RELAYS] = {"t1", "t2", "t3", "t4"};
String onTimer[NUMBER_RELAYS] = {"1", "2", "3", "4"};

/* ------------------------------------------------------------------------- */
/*                             VARIABLE CALENDAR                             */
/* ------------------------------------------------------------------------- */

/* To store time data to "trigger" the Relay of Calendar, (-1) mean "null" */
byte hourON[NUMBER_RELAYS] = {-1, -1, -1, -1};
byte minuteON[NUMBER_RELAYS] = {-1, -1, -1, -1};
byte hourOFF[NUMBER_RELAYS] = {-1, -1, -1, -1};
byte minuteOFF[NUMBER_RELAYS] = {-1, -1, -1, -1};

/**
 * Indicates the current state of the Calendar
 *
 * TRUE   : Calendar is ON
 * FALSE  : Calendar is OFF (default)
 */
bool statusCalendar[NUMBER_RELAYS] = {false, false, false, false};

/* ?, true = "Wait for Activation" */
bool flagCalendarON[NUMBER_RELAYS] = {false, false, false, false};
bool flagCalendarOFF[NUMBER_RELAYS] = {false, false, false, false};

/**
 * To update each Calendar
 * To send to each CLIENT via "event" (SSE)
 */
String offCalendar[NUMBER_RELAYS] = {"c1", "c2", "c3", "c4"};
String onCalendar[NUMBER_RELAYS] = {"1", "2", "3", "4"};

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
  <!-- <link rel="icon" href="data:,"> -->
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
      line-height: 1.75;
    }

    /* ------------------------------- HEADER ------------------------------ */

    .topnav {
      overflow: hidden;
      background-color: #2A3158;
      color: white;
      font-size: 1rem;
    }

    /* ----------------------------- CARD RELAY ---------------------------- */

    .content {
      padding: 20px;
    }

    .cards {
      /* max-width: 100%; */
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

    /* ---------------------------- SWITCH RELAY --------------------------- */

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
      background-color: #757090;
    }

    input:checked+.slider:before {
      -webkit-transform: translateX(52px);
      -ms-transform: translateX(52px);
      transform: translateX(52px);
    }

    /* ------------------------- BUTTON UPDATE TIME ------------------------ */

    .center {
      display: flex;
      justify-content: center;
      align-items: center;
    }

    .btnTime {
      /* height: 100%; */
      /* width: 100%; */
      background-color: #EAEAEA;
      color: black;
      cursor: pointer;
      font-size: 16px;
      padding: 16px 30px;
      border: none;
      border-radius: 5px;
      text-align: center;
    }

    /* Let try only use ".btnTime:hover" */
    /* Instead of using both ".btnTime:hover" and ".btnTime:active" */
    .btnTime:hover {
      background-color: #757090;
      color: white;
    }

    /* -------------------------- ADVANCED FEATURE ------------------------- */

    .collapsible {
      /* width: 100%; */
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
      background-color: #B195A5;
      color: white;
    }

    .collapsible:after {
      /* Unicode character for "plus" sign (+) */
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
      /* Unicode character for "minus" sign (-) */
      content: "\2212";
      color: white;
    }

    .contentAdv {
      max-height: 0;
      overflow: hidden;
      transition: max-height .8s ease-out;
      background-color: #EAEAEA;
    }

    /* --------------------------- CARD ADVANCED --------------------------- */

    .cardsAdv {
      /* max-width: 100%; */
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
      /* width: 100%; */
      background-color: #FFDD91;
      display: inline-block;
    }

    .textAdv {
      padding: 10px;
    }

    /* ------------------------------- INPUT ------------------------------- */

    p.adv {
      font-family: monospace;
    }

    input {
      border: none;
      border-bottom: 1px solid #B2B2B2;
      font-family: monospace;
      font-size: 1rem;
      text-align: center;
    }

    input[type=number] {
      filter: grayscale(1);

      /* 2REM (32px) + 0.25REM */
      width: 36px;
    }

    input[type=number]::-webkit-inner-spin-button,
    input[type=number]::-webkit-outer-spin-button {
      opacity: 1;
    }

    /* ---------------------------- INPUT TIMER ---------------------------- */

    input[type=radio] {
      filter: grayscale(1);
      cursor: pointer;

      /* IE 9 */
      /* -ms-transform: scale(1.2); */
      /* Chrome, Safari, Opera */
      /* -webkit-transform: scale(1.2);  */
      transform: scale(1.2);
    }

    /* --------------------------- INPUT CALENDAR -------------------------- */

    input[type=time] {
      filter: grayscale(1);

      /* 3*2REM (3*32px) + 1REM */
      width: 112px;
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

  <hr> <!-- --------------------------------------------------------------- -->

  <div class="content">
    <button class="collapsible" style="width: 100&#37;;">
      <h3>Advanced Settings:</h3>
    </button>
    <div class="contentAdv">
      <!-- ---------------------------------------------------------------- -->
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
              <!-- -------------------------------------------------------- -->
              <!-- <p class='adv'>
                . After <input type='number' id='th1' value='String(hourAfter[INDEX_R1])'>:<input type='number' id='tm1' value='String(minuteAfter[INDEX_R1])'>:<input type='number' id='ts1' value='String(secondAfter[INDEX_R1])'>
                <br>
                . Then ON <input type='radio' id='tOn1' name='timer1' value='0' stateTrigON(trigTimer[INDEX_R1])> or OFF <input type='radio' id='tOff1' name='timer1' value='1' stateTrigOFF(trigTimer[INDEX_R1])>
              </p>
              <hr>
              <label class='switch'>
                <input type='checkbox' onchange="toggleTimer(this, '1' )" id='t1' stateTimer(statusTimer[INDEX_R1])>
                <span class='slider'></span>
              </label> -->
              <!-- -------------------------------------------------------- -->
            </div>
          </div>
          <div class="cardAdv">
            <div class="titleAdv" style="width: 100&#37;;">
              <h3>CALENDAR</h3>
            </div>
            <div class="textAdv">
              %CALENDAR1%
              <!-- -------------------------------------------------------- -->
              <!-- <p class='adv'>
                . ON at <input type='time' id='cOn1' value='timeCalendar(hourON[INDEX_R1], minuteON[INDEX_R1]);'>
                <br>
                . OFF at <input type='time' id='cOff1' value='timeCalendar(hourOFF[INDEX_R1], minuteOFF[INDEX_R1]);'>
              </p>
              <hr>
              <label class='switch'>
                <input type='checkbox' onchange="toggleCalendar(this, '1')" id='c1' stateCalendar(statusCalendar[INDEX_R1])>
                <span class='slider'></span>
              </label> -->
              <!-- -------------------------------------------------------- -->
            </div>
          </div>
        </div>
      </div>
      <!-- ---------------------------------------------------------------- -->
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
      <!-- ---------------------------------------------------------------- -->
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
      <!-- ---------------------------------------------------------------- -->
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
      <!-- ---------------------------------------------------------------- -->
    </div>
  </div>

  <!-- -------------------------------------------------------------------- -->

  <script>
    /* --------------------------------------------------------------------- */
    /*                                  RTC                                  */
    /* --------------------------------------------------------------------- */

    /* Get data TIME from RTC */
    setInterval(function () {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("time").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/t", true);
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
      xhttp.open("GET", "/d", true);
      xhttp.send();
    }, 1000);

    /* --------------------------------------------------------------------- */

    /**
     * Update TIME and DATE for RTC, by send data to ESP
     * 
     * Ex:
     * "/rtc?Y=?&M=?&D=?&h=?&m=?&s=?"
     */
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
    /*                                 RELAY                                 */
    /* --------------------------------------------------------------------- */

    /**
     * Send data state of RELAY (HTTP GET) to ESP
     * 
     * Ex:
     * "/r?n=1&t=0" → On Relay 1
     * "/r?n=1&t=1" → Off Relay 1
     */
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

      /* ------------------------------------------------------------------- */

      source.addEventListener('t', function (e) {
        if (e.data[0] == "t") {
          document.getElementById(e.data).checked = false; // Turn-off Timer
        }
        else {
          document.getElementById("t" + e.data).checked = true; // Turn-on Timer
        }
      }, false);

      source.addEventListener('c', function (e) {
        if (e.data[0] == "c") {
          document.getElementById(e.data).checked = false; // Turn-off Calendar
        }
        else {
          document.getElementById("c" + e.data).checked = true; // Turn-on Calendar
        }
      }, false);
    };

    /* --------------------------------------------------------------------- */
    /*                           INTERFACE EFFECTS                           */
    /* --------------------------------------------------------------------- */

    /* To use the "Collapsible" feature */
    var coll = document.getElementsByClassName("collapsible");

    /* Show collapsible of "Advanced Settings" */
    coll[0].addEventListener("click", function () {
      this.classList.toggle("active");
      var content = this.nextElementSibling;
      if (content.style.maxHeight) {
        coll[0].style.removeProperty("background-color"); // Remove "Special" color
        //
        content.style.maxHeight = null;
      } else {
        coll[0].style.backgroundColor = "#757090"; // Display "Special" color for easy viewing
        //
        var height = 0;
        for (var i = 1; i < coll.length; i++) {
          height = height + coll[i].nextElementSibling.scrollHeight;
        }
        content.style.maxHeight = (content.scrollHeight + height) + "px";
      }
    });

    /* Show each Relay's setting part */
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

    /* Close "Advanced Settings" when the browser window is resized */
    // window.addEventListener("resize", function () {
    //   coll[0].style.removeProperty("background-color"); // Remove "Special" color
    //   //
    //   for (var i = 0; i < coll.length; i++) {
    //     coll[i].classList.remove("active");
    //     coll[i].nextElementSibling.style.maxHeight = null;
    //   }
    // });

    /* --------------------------------------------------------------------- */
    /*                                 TIMER                                 */
    /* --------------------------------------------------------------------- */

    /**
     * Send the info Timer (HTTP GET) to ESP
     * 
     * Ex:
     * "/ton?n=1&h=23&m=59&s=59&t=0"  → On Timer1 (Trig Relay 1 ON at 23:59:59)
     * "/tof?n=1"                     → Off Timer1
     */
    function toggleTimer(element, device) {
      var xhr = new XMLHttpRequest();
      if (element.checked) {
        var h = document.getElementById("th" + device).value;
        var m = document.getElementById("tm" + device).value;
        var s = document.getElementById("ts" + device).value;
        //
        var trig = "";
        if (document.getElementById("tOn" + device).checked) {
          trig = document.getElementById("tOn" + device).value;
        }
        else {
          trig = document.getElementById("tOff" + device).value;
        }
        //
        xhr.open("GET", "/ton?n=" + device + "&h=" + h + "&m=" + m + "&s=" + s + "&t=" + trig, true);
      }
      else {
        xhr.open("GET", "/tof?n=" + device, true);
      }
      xhr.send();
    };

    /* --------------------------------------------------------------------- */
    /*                                CALENDAR                               */
    /* --------------------------------------------------------------------- */

    /**
     * Send the info Calendar (HTTP GET) to ESP
     * 
     * Ex:
     * "/con?n=1&nH=13&nM=13&fH=7&fM=7" → On Calendar 1
     *                                  | Trig Relay 1 ON at 13:13 PM
     *                                  | Trig Relay 1 OFF at 07:07 AM
     * "/cof?n=1"                       → Off Calendar 1
     */
    function toggleCalendar(element, device) {
      var xhr = new XMLHttpRequest();
      if (element.checked) {
        var onH = document.getElementById("cOn" + device).value.slice(0, 2);
        var onM = document.getElementById("cOn" + device).value.slice(3, 5);
        var offH = document.getElementById("cOff" + device).value.slice(0, 2);
        var offM = document.getElementById("cOff" + device).value.slice(3, 5);
        //
        xhr.open("GET", "/con?n=" + device + "&nH=" + onH + "&nM=" + onM + "&fH=" + offH + "&fM=" + offM, true);
      }
      else {
        xhr.open("GET", "/cof?n=" + device, true);
      }
      xhr.send();
    };
  </script>
</body>

</html>
)rawliteral";

/* ------------------------------------------------------------------------- */
/*                         FUNCTION WEB SERVER (ESP)                         */
/* ------------------------------------------------------------------------- */

/* Replace placeholder with RTC values and state Relay */
String processor(const String &var)
{
  DEBUG_PRINT(F("Var: "));
  DEBUG_PRINTLN(var);

  if (var == "TIME")
  {
    return F("--:--:--");
  }
  else if (var == "DATE")
  {
    return F("--/--/----");
  }
  else if (var == "RELAY")
  {
    String relays = "";
    relays += "<div class='card'><h3>RELAY 1 - <span id='relay1'>" + stateRelay(RELAY_1) + "</span></h3><label class='switch'><input type='checkbox' onchange=\"toggleCheckbox(this, 'relay1')\" id='1' " + stateCheckbox(RELAY_1) + "><span class='slider'></span></label></div>";
    relays += "<div class='card'><h3>RELAY 2 - <span id='relay2'>" + stateRelay(RELAY_2) + "</span></h3><label class='switch'><input type='checkbox' onchange=\"toggleCheckbox(this, 'relay2')\" id='2' " + stateCheckbox(RELAY_2) + "><span class='slider'></span></label></div>";
    relays += "<div class='card'><h3>RELAY 3 - <span id='relay3'>" + stateRelay(RELAY_3) + "</span></h3><label class='switch'><input type='checkbox' onchange=\"toggleCheckbox(this, 'relay3')\" id='3' " + stateCheckbox(RELAY_3) + "><span class='slider'></span></label></div>";
    relays += "<div class='card'><h3>RELAY 4 - <span id='relay4'>" + stateRelay(RELAY_4) + "</span></h3><label class='switch'><input type='checkbox' onchange=\"toggleCheckbox(this, 'relay4')\" id='4' " + stateCheckbox(RELAY_4) + "><span class='slider'></span></label></div>";
    return relays;
  }
  else if (var == "TIMER1")
  {
    String timer1 = "";
    timer1 += "<p class='adv'>. After <input type='number' id='th1' value='" + String(hourAfter[INDEX_R1]);
    timer1 += "'>:<input type='number' id='tm1' value='" + String(minuteAfter[INDEX_R1]);
    timer1 += "'>:<input type='number' id='ts1' value='" + String(secondAfter[INDEX_R1]);
    timer1 += "'><br>. Then ON <input type='radio' id='tOn1' name='timer1' value='0' " + stateTrigON(trigTimer[INDEX_R1]);
    timer1 += "> or OFF <input type='radio' id='tOff1' name='timer1' value='1' " + stateTrigOFF(trigTimer[INDEX_R1]);
    timer1 += "></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleTimer(this, '1')\" id='t1' " + stateTimer(statusTimer[INDEX_R1]);
    timer1 += "><span class='slider'></span></label>";
    return timer1;
  }
  else if (var == "CALENDAR1")
  {
    String calendar1 = "";
    calendar1 += "<p class='adv'>. ON at <input type='time' id='cOn1' value='" + timeCalendar(hourON[INDEX_R1], minuteON[INDEX_R1]);
    calendar1 += "'><br>. OFF at <input type='time' id='cOff1' value='" + timeCalendar(hourOFF[INDEX_R1], minuteOFF[INDEX_R1]);
    calendar1 += "'></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleCalendar(this, '1')\" id='c1' " + stateCalendar(statusCalendar[INDEX_R1]);
    calendar1 += "><span class='slider'></span></label>";
    return calendar1;
  }
  else if (var == "TIMER2")
  {
    String timer2 = "";
    timer2 += "<p class='adv'>. After <input type='number' id='th2' value='" + String(hourAfter[INDEX_R2]);
    timer2 += "'>:<input type='number' id='tm2' value='" + String(minuteAfter[INDEX_R2]);
    timer2 += "'>:<input type='number' id='ts2' value='" + String(secondAfter[INDEX_R2]);
    timer2 += "'><br>. Then ON <input type='radio' id='tOn2' name='timer2' value='0' " + stateTrigON(trigTimer[INDEX_R2]);
    timer2 += "> or OFF <input type='radio' id='tOff2' name='timer2' value='1' " + stateTrigOFF(trigTimer[INDEX_R2]);
    timer2 += "></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleTimer(this, '2')\" id='t2' " + stateTimer(statusTimer[INDEX_R2]);
    timer2 += "><span class='slider'></span></label>";
    return timer2;
  }
  else if (var == "CALENDAR2")
  {
    String calendar2 = "";
    calendar2 += "<p class='adv'>. ON at <input type='time' id='cOn2' value='" + timeCalendar(hourON[INDEX_R2], minuteON[INDEX_R2]);
    calendar2 += "'><br>. OFF at <input type='time' id='cOff2' value='" + timeCalendar(hourOFF[INDEX_R2], minuteOFF[INDEX_R2]);
    calendar2 += "'></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleCalendar(this, '2')\" id='c2' " + stateCalendar(statusCalendar[INDEX_R2]);
    calendar2 += "><span class='slider'></span></label>";
    return calendar2;
  }
  else if (var == "TIMER3")
  {
    String timer3 = "";
    timer3 += "<p class='adv'>. After <input type='number' id='th3' value='" + String(hourAfter[INDEX_R3]);
    timer3 += "'>:<input type='number' id='tm3' value='" + String(minuteAfter[INDEX_R3]);
    timer3 += "'>:<input type='number' id='ts3' value='" + String(secondAfter[INDEX_R3]);
    timer3 += "'><br>. Then ON <input type='radio' id='tOn3' name='timer3' value='0' " + stateTrigON(trigTimer[INDEX_R3]);
    timer3 += "> or OFF <input type='radio' id='tOff3' name='timer3' value='1' " + stateTrigOFF(trigTimer[INDEX_R3]);
    timer3 += "></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleTimer(this, '3')\" id='t3' " + stateTimer(statusTimer[INDEX_R3]);
    timer3 += "><span class='slider'></span></label>";
    return timer3;
  }
  else if (var == "CALENDAR3")
  {
    String calendar3 = "";
    calendar3 += "<p class='adv'>. ON at <input type='time' id='cOn3' value='" + timeCalendar(hourON[INDEX_R3], minuteON[INDEX_R3]);
    calendar3 += "'><br>. OFF at <input type='time' id='cOff3' value='" + timeCalendar(hourOFF[INDEX_R3], minuteOFF[INDEX_R3]);
    calendar3 += "'></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleCalendar(this, '3')\" id='c3' " + stateCalendar(statusCalendar[INDEX_R3]);
    calendar3 += "><span class='slider'></span></label>";
    return calendar3;
  }
  else if (var == "TIMER4")
  {
    String timer4 = "";
    timer4 += "<p class='adv'>. After <input type='number' id='th4' value='" + String(hourAfter[INDEX_R4]);
    timer4 += "'>:<input type='number' id='tm4' value='" + String(minuteAfter[INDEX_R4]);
    timer4 += "'>:<input type='number' id='ts4' value='" + String(secondAfter[INDEX_R4]);
    timer4 += "'><br>. Then ON <input type='radio' id='tOn4' name='timer4' value='0' " + stateTrigON(trigTimer[INDEX_R4]);
    timer4 += "> or OFF <input type='radio' id='tOff4' name='timer4' value='1' " + stateTrigOFF(trigTimer[INDEX_R4]);
    timer4 += "></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleTimer(this, '4')\" id='t4' " + stateTimer(statusTimer[INDEX_R4]);
    timer4 += "><span class='slider'></span></label>";
    return timer4;
  }
  else if (var == "CALENDAR4")
  {
    String calendar4 = "";
    calendar4 += "<p class='adv'>. ON at <input type='time' id='cOn4' value='" + timeCalendar(hourON[INDEX_R4], minuteON[INDEX_R4]);
    calendar4 += "'><br>. OFF at <input type='time' id='cOff4' value='" + timeCalendar(hourOFF[INDEX_R4], minuteOFF[INDEX_R4]);
    calendar4 += "'></p><hr><label class='switch'><input type='checkbox' onchange=\"toggleCalendar(this, '4')\" id='c4' " + stateCalendar(statusCalendar[INDEX_R4]);
    calendar4 += "><span class='slider'></span></label>";
    return calendar4;
  }

  return String();
}

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

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/time=<timeRTC.c_str()>
   *
   * Change:
   * <ESP_IP>/t=<timeRTC.c_str()>
   */
  server.on(URL_TIME, HTTP_GET, [](AsyncWebServerRequest *request)
            { DEBUG_PRINTLN(F("TIME"));
              request->send_P(200, "text/plain", timeRTC.c_str()); });

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/date=<dateRTC.c_str()>
   *
   * Change:
   * <ESP_IP>/d=<dateRTC.c_str()>
   */
  server.on(URL_DATE, HTTP_GET, [](AsyncWebServerRequest *request)
            { DEBUG_PRINTLN(F("DATE"));
              request->send_P(200, "text/plain", dateRTC.c_str()); });

  /* ----------------------------------------------------------------------- */

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/rtc?Y=<Y>&M=<M>&D=<D>&h=<h>&m=<m>&s=<s>
   */
  server.on(URL_RTC, HTTP_GET, [](AsyncWebServerRequest *request)
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

                switch (mes1.toInt()) // Update for "statusRelays[]"
                {
                case 1:
                {
                  (mes2.toInt()) ? (statusRelays[INDEX_R1] = true) : (statusRelays[INDEX_R1] = false);
                  TRIG_RELAY_1;
                  /* Send Events to the Web Server */
                  if (statusRelays[INDEX_R1])
                    events.send(offRelay.c_str(), EVENT_RELAY_1, millis());
                  else
                    events.send(onRelay.c_str(), EVENT_RELAY_1, millis());
                  break;
                }
                case 2:
                {
                  (mes2.toInt()) ? (statusRelays[INDEX_R2] = true) : (statusRelays[INDEX_R2] = false);
                  TRIG_RELAY_2;
                  /* Send Events to the Web Server */
                  if (statusRelays[INDEX_R2])
                    events.send(offRelay.c_str(), EVENT_RELAY_2, millis());
                  else
                    events.send(onRelay.c_str(), EVENT_RELAY_2, millis());
                  break;
                }
                case 3:
                {
                  (mes2.toInt()) ? (statusRelays[INDEX_R3] = true) : (statusRelays[INDEX_R3] = false);
                  TRIG_RELAY_3;
                  /* Send Events to the Web Server */
                  if (statusRelays[INDEX_R3])
                    events.send(offRelay.c_str(), EVENT_RELAY_3, millis());
                  else
                    events.send(onRelay.c_str(), EVENT_RELAY_3, millis());
                  break;
                }
                case 4:
                {
                  (mes2.toInt()) ? (statusRelays[INDEX_R4] = true) : (statusRelays[INDEX_R4] = false);
                  TRIG_RELAY_4;
                  /* Send Events to the Web Server */
                  if (statusRelays[INDEX_R4])
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

  /* ----------------------------------------------------------------------- */

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/timerON?device=<device>&h=<h>&m=<m>&s=<s>&trig=<trig>
   *
   * Change:
   * <ESP_IP>/ton?n=<device>&h=<h>&m=<m>&s=<s>&t=<trig>
   */
  server.on(URL_ON_TIMER, HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device, h, m, s, trig;
              
              /* GET value of "device", "h", "m", "s", "trig" */
              if (request->hasParam(PARAM_NUM) 
              && request->hasParam(PARAM_HOUR) 
              && request->hasParam(PARAM_MINUTE) 
              && request->hasParam(PARAM_SECOND) 
              && request->hasParam(PARAM_TRIG))
              {
                device = request->getParam(PARAM_NUM)->value();
                h = request->getParam(PARAM_HOUR)->value();
                m = request->getParam(PARAM_MINUTE)->value();
                s = request->getParam(PARAM_SECOND)->value();
                trig = request->getParam(PARAM_TRIG)->value();

                byte addr = device.toInt() - 1;

                hourAfter[addr] = h.toInt();
                minuteAfter[addr] = m.toInt();
                secondAfter[addr] = s.toInt();
                trigTimer[addr] = trig.toInt();

                statusTimer[addr] = true; // Enable Timer
                calculateFlagTimer(addr, &rtc, hourAfter, minuteAfter, secondAfter, flagTimer);

                /* Send Events to the Web Server */
                events.send(onTimer[addr].c_str(), EVENT_TIMER, millis());
              }
              else
              {
                device = F("No message sent");
                h = F("No message sent");
                m = F("No message sent");
                s = F("No message sent");
                trig = F("No message sent");
              }

              DEBUG_PRINT(F("Timer ")); DEBUG_PRINT(device);
              DEBUG_PRINT(F(" ON - Trig ")); DEBUG_PRINT(trig);
              DEBUG_PRINT(F(" - ")); DEBUG_PRINT(h);
              DEBUG_PRINT(F(":")); DEBUG_PRINT(m);
              DEBUG_PRINT(F(":")); DEBUG_PRINTLN(s);
              
              request->send(200, "text/plain", "OK"); });

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/timerOFF?device=<device>
   *
   * Change:
   * <ESP_IP>/tof?n=<device>
   */
  server.on(URL_OFF_TIMER, HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device;
              
              /* GET value of "device" */
              if (request->hasParam(PARAM_NUM))
              {
                device = request->getParam(PARAM_NUM)->value();

                byte addr = device.toInt() - 1;
                statusTimer[addr] = false; // Disable Timer

                /* Send Events to the Web Server */
                events.send(offTimer[addr].c_str(), EVENT_TIMER, millis());
              }
              else
              {
                device = F("No message sent");
              }

              DEBUG_PRINT(F("Timer ")); DEBUG_PRINT(device); DEBUG_PRINTLN(F(" OFF"));

              request->send(200, "text/plain", "OK"); });

  /* ----------------------------------------------------------------------- */

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/calendarON?device=<device>&onH=<onH>&onM=<onM>&offH=<offH>&offM=<offM>
   *
   * Change:
   * <ESP_IP>/con?n=<device>&nH=<onH>&nM=<onM>&fH=<offH>&fM=<offM>
   */
  server.on(URL_ON_CALENDAR, HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device, onH, onM, offH, offM;

              /* GET value of "device", "onH", "onM", "offH", "offM" */
              if (request->hasParam(PARAM_NUM) 
              && request->hasParam(PARAM_ON_HOUR) 
              && request->hasParam(PARAM_ON_MINUTE) 
              && request->hasParam(PARAM_OFF_HOUR) 
              && request->hasParam(PARAM_OFF_MINUTE))
              {
                device = request->getParam(PARAM_NUM)->value();
                onH = request->getParam(PARAM_ON_HOUR)->value();
                onM = request->getParam(PARAM_ON_MINUTE)->value();
                offH = request->getParam(PARAM_OFF_HOUR)->value();
                offM = request->getParam(PARAM_OFF_MINUTE)->value();

                byte addr = device.toInt() - 1;

                hourON[addr] = onH.toInt();
                minuteON[addr] = onM.toInt();
                hourOFF[addr] = offH.toInt();
                minuteOFF[addr] = offM.toInt();

                statusCalendar[addr] = true; // Enable Calendar
                calculateFlagCalendar(addr, &rtc, hourON, minuteON, hourOFF, minuteOFF, flagCalendarON, flagCalendarOFF);

                /* Send Events to the Web Server */
                events.send(onCalendar[addr].c_str(), EVENT_CALENDAR, millis());
              }
              else
              {
                device = F("No message sent");
                onH = F("No message sent");
                onM = F("No message sent");
                offH = F("No message sent");
                offM = F("No message sent");
              }

              DEBUG_PRINT(F("Calendar ")); DEBUG_PRINT(device);
              DEBUG_PRINT(F(" ON - Trig On = ")); DEBUG_PRINT(onH); DEBUG_PRINT(F(":")); DEBUG_PRINT(onM);
              DEBUG_PRINT(F(" - Trig Off = ")); DEBUG_PRINT(offH); DEBUG_PRINT(F(":")); DEBUG_PRINTLN(offM);

              request->send(200, "text/plain", "OK"); });

  /**
   * Send a GET request to ...
   *
   * Original:
   * <ESP_IP>/calendarOFF?device=<device>
   *
   * Change:
   * <ESP_IP>/cof?n=<device>
   */
  server.on(URL_OFF_CALENDAR, HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String device;

              /* GET value of "device" */
              if (request->hasParam(PARAM_NUM))
              {
                device = request->getParam(PARAM_NUM)->value();
                
                byte addr = device.toInt() - 1;
                statusCalendar[addr] = false; // Disable Calendar

                /* Send Events to the Web Server */
                events.send(offCalendar[addr].c_str(), EVENT_CALENDAR, millis());
              }
              else
              {
                device = F("No message sent");
              }

              DEBUG_PRINT(F("Calendar ")); DEBUG_PRINT(device); DEBUG_PRINTLN(F(" OFF"));

              request->send(200, "text/plain", "OK"); });

  /* ----------------------------------------------------------------------- */

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
      statusRelays[INDEX_R1] = !statusRelays[INDEX_R1];
      TRIG_RELAY_1;
      /* Send Events to the Web Server */
      if (statusRelays[INDEX_R1])
        events.send(offRelay.c_str(), EVENT_RELAY_1, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_1, millis());
      break;
    }
    case BTN_2:
    {
      statusRelays[INDEX_R2] = !statusRelays[INDEX_R2];
      TRIG_RELAY_2;
      /* Send Events to the Web Server */
      if (statusRelays[INDEX_R2])
        events.send(offRelay.c_str(), EVENT_RELAY_2, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_2, millis());
      break;
    }
    case BTN_3:
    {
      statusRelays[INDEX_R3] = !statusRelays[INDEX_R3];
      TRIG_RELAY_3;
      /* Send Events to the Web Server */
      if (statusRelays[INDEX_R3])
        events.send(offRelay.c_str(), EVENT_RELAY_3, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_3, millis());
      break;
    }
    case BTN_4:
    {
      statusRelays[INDEX_R4] = !statusRelays[INDEX_R4];
      TRIG_RELAY_4;
      /* Send Events to the Web Server */
      if (statusRelays[INDEX_R4])
        events.send(offRelay.c_str(), EVENT_RELAY_4, millis());
      else
        events.send(onRelay.c_str(), EVENT_RELAY_4, millis());
      break;
    }
    }

    /* --------------------------------------------------------------------- */

    checkBtn = millis();
  }

  /* ----------------------------------------------------------------------- */

  if (millis() - checkRTC >= TIME_CHECK_RTC)
  {
    /* ------------------------- Get data from RTC ------------------------- */

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

    /* --------------- Check if any Timer is enabled or not? --------------- */

    /* Timer 1 */
    if (statusTimer[INDEX_R1])
    {
      if (now.unixtime() >= flagTimer[INDEX_R1])
      {
        statusTimer[INDEX_R1] = false; // Disable Timer 1
        statusRelays[INDEX_R1] = trigTimer[INDEX_R1];
        TRIG_RELAY_1;
        /* Send Events to the Web Server */
        if (statusRelays[INDEX_R1])
          events.send(offRelay.c_str(), EVENT_RELAY_1, millis());
        else
          events.send(onRelay.c_str(), EVENT_RELAY_1, millis());
        //
        events.send(offTimer[INDEX_R1].c_str(), EVENT_TIMER, millis());
      }
    }

    /* Timer 2 */
    if (statusTimer[INDEX_R2])
    {
      if (now.unixtime() >= flagTimer[INDEX_R2])
      {
        statusTimer[INDEX_R2] = false; // Disable Timer 2
        statusRelays[INDEX_R2] = trigTimer[INDEX_R2];
        TRIG_RELAY_2;
        /* Send Events to the Web Server */
        if (statusRelays[INDEX_R2])
          events.send(offRelay.c_str(), EVENT_RELAY_2, millis());
        else
          events.send(onRelay.c_str(), EVENT_RELAY_2, millis());
        //
        events.send(offTimer[INDEX_R2].c_str(), EVENT_TIMER, millis());
      }
    }

    /* Timer 3 */
    if (statusTimer[INDEX_R3])
    {
      if (now.unixtime() >= flagTimer[INDEX_R3])
      {
        statusTimer[INDEX_R3] = false; // Disable Timer 3
        statusRelays[INDEX_R3] = trigTimer[INDEX_R3];
        TRIG_RELAY_3;
        /* Send Events to the Web Server */
        if (statusRelays[INDEX_R3])
          events.send(offRelay.c_str(), EVENT_RELAY_3, millis());
        else
          events.send(onRelay.c_str(), EVENT_RELAY_3, millis());
        //
        events.send(offTimer[INDEX_R3].c_str(), EVENT_TIMER, millis());
      }
    }

    /* Timer 4 */
    if (statusTimer[INDEX_R4])
    {
      if (now.unixtime() >= flagTimer[INDEX_R4])
      {
        statusTimer[INDEX_R4] = false; // Disable Timer 4
        statusRelays[INDEX_R4] = trigTimer[INDEX_R4];
        TRIG_RELAY_4;
        /* Send Events to the Web Server */
        if (statusRelays[INDEX_R4])
          events.send(offRelay.c_str(), EVENT_RELAY_4, millis());
        else
          events.send(onRelay.c_str(), EVENT_RELAY_4, millis());
        //
        events.send(offTimer[INDEX_R4].c_str(), EVENT_TIMER, millis());
      }
    }

    /* -------------- Check if any Calendar is enabled or not. ------------- */

    /* Calendar 1 */
    if (statusCalendar[INDEX_R1])
    {
      if (flagCalendarON[INDEX_R1])
      {
        if (now.hour() == hourON[INDEX_R1] && now.minute() == minuteON[INDEX_R1])
        {
          flagCalendarON[INDEX_R1] = false; // Clear Flag Calendar ON
          statusRelays[INDEX_R1] = RELAY_ON;
          TRIG_RELAY_1;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R1])
            events.send(offRelay.c_str(), EVENT_RELAY_1, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_1, millis());
        }
      }

      if (flagCalendarOFF[INDEX_R1])
      {
        if (now.hour() == hourOFF[INDEX_R1] && now.minute() == minuteOFF[INDEX_R1])
        {
          flagCalendarOFF[INDEX_R1] = false; // Clear Flag Calendar OFF
          statusRelays[INDEX_R1] = RELAY_OFF;
          TRIG_RELAY_1;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R1])
            events.send(offRelay.c_str(), EVENT_RELAY_1, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_1, millis());
        }
      }

      if (!flagCalendarON[INDEX_R1] && !flagCalendarOFF[INDEX_R1])
      {
        statusCalendar[INDEX_R1] = false; // Disable Calendar 1
        /* Send Events to the Web Server */
        events.send(offCalendar[INDEX_R1].c_str(), EVENT_CALENDAR, millis());
      }
    }

    /* Calendar 2 */
    if (statusCalendar[INDEX_R2])
    {
      if (flagCalendarON[INDEX_R2])
      {
        if (now.hour() == hourON[INDEX_R2] && now.minute() == minuteON[INDEX_R2])
        {
          flagCalendarON[INDEX_R2] = false; // Clear Flag Calendar ON
          statusRelays[INDEX_R2] = RELAY_ON;
          TRIG_RELAY_2;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R2])
            events.send(offRelay.c_str(), EVENT_RELAY_2, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_2, millis());
        }
      }

      if (flagCalendarOFF[INDEX_R2])
      {
        if (now.hour() == hourOFF[INDEX_R2] && now.minute() == minuteOFF[INDEX_R2])
        {
          flagCalendarOFF[INDEX_R2] = false; // Clear Flag Calendar OFF
          statusRelays[INDEX_R2] = RELAY_OFF;
          TRIG_RELAY_2;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R2])
            events.send(offRelay.c_str(), EVENT_RELAY_2, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_2, millis());
        }
      }

      if (!flagCalendarON[INDEX_R2] && !flagCalendarOFF[INDEX_R2])
      {
        statusCalendar[INDEX_R2] = false; // Disable Calendar 2
        /* Send Events to the Web Server */
        events.send(offCalendar[INDEX_R2].c_str(), EVENT_CALENDAR, millis());
      }
    }

    /* Calendar 3 */
    if (statusCalendar[INDEX_R3])
    {
      if (flagCalendarON[INDEX_R3])
      {
        if (now.hour() == hourON[INDEX_R3] && now.minute() == minuteON[INDEX_R3])
        {
          flagCalendarON[INDEX_R3] = false; // Clear Flag Calendar ON
          statusRelays[INDEX_R3] = RELAY_ON;
          TRIG_RELAY_3;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R3])
            events.send(offRelay.c_str(), EVENT_RELAY_3, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_3, millis());
        }
      }

      if (flagCalendarOFF[INDEX_R3])
      {
        if (now.hour() == hourOFF[INDEX_R3] && now.minute() == minuteOFF[INDEX_R3])
        {
          flagCalendarOFF[INDEX_R3] = false; // Clear Flag Calendar OFF
          statusRelays[INDEX_R3] = RELAY_OFF;
          TRIG_RELAY_3;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R3])
            events.send(offRelay.c_str(), EVENT_RELAY_3, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_3, millis());
        }
      }

      if (!flagCalendarON[INDEX_R3] && !flagCalendarOFF[INDEX_R3])
      {
        statusCalendar[INDEX_R3] = false; // Disable Calendar 3
        /* Send Events to the Web Server */
        events.send(offCalendar[INDEX_R3].c_str(), EVENT_CALENDAR, millis());
      }
    }

    /* Calendar 4 */
    if (statusCalendar[INDEX_R4])
    {
      if (flagCalendarON[INDEX_R4])
      {
        if (now.hour() == hourON[INDEX_R4] && now.minute() == minuteON[INDEX_R4])
        {
          flagCalendarON[INDEX_R4] = false; // Clear Flag Calendar ON
          statusRelays[INDEX_R4] = RELAY_ON;
          TRIG_RELAY_4;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R4])
            events.send(offRelay.c_str(), EVENT_RELAY_4, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_4, millis());
        }
      }

      if (flagCalendarOFF[INDEX_R4])
      {
        if (now.hour() == hourOFF[INDEX_R4] && now.minute() == minuteOFF[INDEX_R4])
        {
          flagCalendarOFF[INDEX_R4] = false; // Clear Flag Calendar OFF
          statusRelays[INDEX_R4] = RELAY_OFF;
          TRIG_RELAY_4;
          /* Send Events to the Web Server */
          if (statusRelays[INDEX_R4])
            events.send(offRelay.c_str(), EVENT_RELAY_4, millis());
          else
            events.send(onRelay.c_str(), EVENT_RELAY_4, millis());
        }
      }

      if (!flagCalendarON[INDEX_R4] && !flagCalendarOFF[INDEX_R4])
      {
        statusCalendar[INDEX_R4] = false; // Disable Calendar 4
        /* Send Events to the Web Server */
        events.send(offCalendar[INDEX_R4].c_str(), EVENT_CALENDAR, millis());
      }
    }

    /* --------------------------------------------------------------------- */

    checkRTC = millis();
  }
}
