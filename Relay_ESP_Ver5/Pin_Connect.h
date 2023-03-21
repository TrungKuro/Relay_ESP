#ifndef _PIN_CONNECT_H_
#define _PIN_CONNECT_H_

/* ------------------------------------------------------------------------- */
/*                DEBUG (uncomment to open the Debug function)               */
/* ------------------------------------------------------------------------- */

// #define ENABLE_DEBUG

#if defined(ENABLE_DEBUG)
#define Debug Serial
#define DEBUG_BEGIN(...) Debug.begin(__VA_ARGS__)
#define DEBUG_WRITE(...) Debug.write(__VA_ARGS__)
#define DEBUG_PRINT(...) Debug.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Debug.println(__VA_ARGS__)
#else
#define DEBUG_BEGIN(...)
#define DEBUG_WRITE(...)
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

/* ------------------------------------------------------------------------- */
/*                                   BUTTON                                  */
/* ------------------------------------------------------------------------- */

/**
 * Read ADC from pin "TOUT" (A0)
 * When no 'click' the button, the value voltage is 0V ~ 3ADC
 *
 *          | VDC | ADC | Real value | Range ± 0.05 VDC
 * Button 1 = 0.2 : 062 ~ 057 -5    -> [0.15 - 0.25] = [046 - 078]
 * Button 2 = 0.4 : 124 ~ 116 +8    -> [0.35 - 0.45] = [108 - 140]
 * Button 3 = 0.6 : 186 ~ 180 +6    -> [0.55 - 0.65] = [170 - 202]
 * Button 4 = 0.8 : 248 ~ 237 +11   -> [0.75 - 0.85] = [232 - 264]
 *
 * The ADC pin has a 10-bit resolution, which means you’ll get values between 0 and 1023
 * The ESP8266 ADC pin input voltage range is 0 to 1V if you’re using the bare chip
 */

#define BUTTON A0 //! TOUT (ADC)

#define BUTTON_1_MIN 46
#define BUTTON_1_MAX 78

#define BUTTON_2_MIN 108
#define BUTTON_2_MAX 140

#define BUTTON_3_MIN 170
#define BUTTON_3_MAX 202

#define BUTTON_4_MIN 232
#define BUTTON_4_MAX 264

#define DEBOUNCE 10 // Unit (ms)

#define TIME_CHECK_BTN 10 // Unit (ms)

#define NONE_BTN '0' // Detect none button
#define BTN_1 '1'    // Detect button 1
#define BTN_2 '2'    // Detect button 2
#define BTN_3 '3'    // Detect button 3
#define BTN_4 '4'    // Detect button 4

/* ------------------------------------------------------------------------- */
/*                                    LED                                    */
/* ------------------------------------------------------------------------- */

/**
 * Trigger HIGH to turn-on LED
 * Trigger LOW to turn-off LED
 */

#define LED 16 //! IO16 (GPIO16)

#define LED_ON digitalWrite(LED, HIGH)
#define LED_OFF digitalWrite(LED, LOW)

/* ------------------------------------------------------------------------- */
/*                                   RELAY                                   */
/* ------------------------------------------------------------------------- */

#define NUMBER_RELAYS 4
#define INDEX_R1 0
#define INDEX_R2 1
#define INDEX_R3 2
#define INDEX_R4 3

/**
 * Trigger LOW to turn-on Relay
 * Trigger HIGH to turn-off Relay
 *
 * Relay 1 - IO 04
 * Relay 2 - IO 05
 * Relay 3 - IO 12
 * Relay 4 - IO 13
 */

#define RELAY_1 4  //! IO4 (GPIO4)
#define RELAY_2 5  //! IO5 (GPIO5)
#define RELAY_3 12 //! IO12 (GPIO12 / MISO / PWM-R)
#define RELAY_4 13 //! IO13 (GPIO13 / MOSI / PWM-B)
//
#define RELAY_ON false // Turn-on Relay
#define RELAY_OFF true // Turn-off Relay

/**
 * Every time, the user manipulates the control on the CLIENT's interface
 * |
 * When the slider is RED, means the RELAY is ON (output is LOW)
 * When the slider is GRAY, means the RELAY is OFF (output is HIGH)
 * |
 * When the user "inverts" the slider state
 * CLIENT will send an HTTP GET request to the SERVER (ESP)
 * By URL path ...
 * |
 * When ESP receives a URL, it has info to know what to do
 *
 * **********************************************
 *
 * Original : /update?output=[?]&state=[?]
 * Change   : /relay?numerical=[?]&triggered=[?]
 *          |
 *          numerical = {'1', '2', '3', '4'}
 *          |         ~ relay1, relay2, relay3, relay4
 *          |
 *          triggered = {'0', '1'}
 *          |         ~ LOW, HIGH
 *          |         ~ false, true
 *          |         ~ ON Relay, OFF Relay
 *          |
 * Shorten  : /r?n=[?]&t=[?]
 */
#define URL_RELAY "/r" // relay
#define PARAM_NUM "n"  // numerical
#define PARAM_TRIG "t" // triggered

/**
 * Enable Relay according to its state variable value
 * Based on the variable "statusRelays[?]"
 */
#define TRIG_RELAY_1 digitalWrite(RELAY_1, statusRelays[0])
#define TRIG_RELAY_2 digitalWrite(RELAY_2, statusRelays[1])
#define TRIG_RELAY_3 digitalWrite(RELAY_3, statusRelays[2])
#define TRIG_RELAY_4 digitalWrite(RELAY_4, statusRelays[3])

/* ------------------------------------------------------------------------- */
/*                                   TIMER                                   */
/* ------------------------------------------------------------------------- */

/**
 * Original : /timerON?device=[?]&h=[?]&m=[?]&s=[?]&trig=[?]
 * Change   : /timerOn?numerical=[?]&hour=[?]&minute=[?]&second=[?]&triggered=[?]
 *          |
 *          numerical = {'1', '2', '3', '4'}
 *          |         ~ timer1, timer2, timer3, timer4
 *          |
 *          triggered = {'0', '1'}
 *          |         ~ ON Relay, OFF Relay
 *          |
 * Shorten  : /ton?n=[?]&h=[?]&m=[?]&s=[?]&t=[?]
 */
#define URL_ON_TIMER "/ton" // timerOn
#define PARAM_NUM "n"       // numerical
#define PARAM_HOUR "h"      // hour
#define PARAM_MINUTE "m"    // minute
#define PARAM_SECOND "s"    // second
#define PARAM_TRIG "t"      // triggered

/**
 * Original : /timerOFF?device=[?]
 * Change   : /timerOff?numerical=[?]
 *          |
 *          numerical = {'1', '2', '3', '4'}
 *          |         ~ timer1, timer2, timer3, timer4
 *          |
 * Shorten  : /tof?n=[?]
 */
#define URL_OFF_TIMER "/tof" // timerOff
#define PARAM_NUM "n"        // numerical

/* ------------------------------------------------------------------------- */
/*                                  CALENDAR                                 */
/* ------------------------------------------------------------------------- */

/**
 * Original : /calendarON?device=[?]&onH=[?]&onM=[?]&offH=[?]&offM=[?]
 * Change   : /calendarOn?numerical=[?]&onHour=[?]&onMinute=[?]&offHour=[?]&offMinute=[?]
 *          |
 *          numerical = {'1', '2', '3', '4'}
 *          |         ~ calendar1, calendar2, calendar3, calendar4
 *          |
 * Shorten  : /con?n=[?]&nH=[?]&nM=[?]&fH=[?]&fM=[?]
 */
#define URL_ON_CALENDAR "/con" // calendarOn
#define PARAM_NUM "n"          // numerical
#define PARAM_ON_HOUR "nH"     // onHour
#define PARAM_ON_MINUTE "nM"   // onMinute
#define PARAM_OFF_HOUR "fH"    // offHour
#define PARAM_OFF_MINUTE "fM"  // offMinute

/**
 * Original : /calendarOFF?device=[?]
 * Change   : /calendarOff?numerical=[?]
 *          |
 *          numerical = {'1', '2', '3', '4'}
 *          |         ~ calendar1, calendar2, calendar3, calendar4
 *          |
 * Shorten  : /cof?n=[?]
 */
#define URL_OFF_CALENDAR "/cof" // calendarOff
#define PARAM_NUM "n"           // numerical

/* ------------------------------------------------------------------------- */
/*                                    RTC                                    */
/* ------------------------------------------------------------------------- */

/**
 * SCL (DS1307) - IO 14
 * SDA (DS1307) - IO 02
 */
#define SCL 14 //! IO14 (GPIO14 / SCL / CLK)
#define SDA 2  //! IO2 (GPIO2 / SDA / U1TXD)

#define TIME_CHECK_RTC 1000 // Unit(ms)

/**
 * Original : /time
 * Original : /date
 * Original : /rtc
 */
#define URL_TIME "/t" // time
#define URL_DATE "/d" // date
#define URL_RTC "/rtc"

/* ------------------------------------------------------------------------- */
/*                          WIFI - ACCESS POINT (AP)                         */
/* ------------------------------------------------------------------------- */

/**
 * IP address of the soft access point:
 * "local_IP" = (IP_A).(IP_B).(IP_C).(IP_D)
 */
#define IP_A 192
#define IP_B 168
#define IP_C 1
#define IP_D 11

/**
 * Gateway IP address:
 * "gateway" = (GATEWAY_A).(GATEWAY_B).(GATEWAY_C).(GATEWAY_D)
 */
#define GATEWAY_A 192
#define GATEWAY_B 168
#define GATEWAY_C 1
#define GATEWAY_D 1

/**
 * Subnet mask:
 * "subnet" = (SUBNET_A).(SUBNET_B).(SUBNET_C).(SUBNET_D)
 */
#define SUBNET_A 255
#define SUBNET_B 255
#define SUBNET_C 255
#define SUBNET_D 0

/**
 * Replace with your network credentials
 *
 * SSID     : Maximum of 31 characters
 * PASSWORD : Minimum of 8 characters, maximum 63 characters
 *            If not specified, the access point will be open
 *
 * CHANNEL        : Wi-Fi channel number (1-13). Default is 1
 * SSID_HIDDEN    : If set to "true" will hide SSID
 * MAX_CONNECTION : The maximum number of stations
 *                  That can simultaneously be connected to the soft-AP
 *                  Can be set from 0 to 8, but defaults to 4
 *                  |
 * Once the max number has been reached
 * Any other station that wants to connect
 * Will be forced to wait until an already connected station disconnects
 */
#define WIFI_SSID "Relay ESP"
#define WIFI_PASSWORD "Demo123#"
#define CHANNEL 1
#define SSID_HIDDEN false //! To test fast
#define MAX_CONNECTION 4

/* ------------------------------------------------------------------------- */
/*                              WEB SERVER (ESP)                             */
/* ------------------------------------------------------------------------- */

/**
 * The CLIENT initiates the SSE connection
 * And the SERVER (ESP) uses the "event source protocol"
 * On the /events URL to send updates to the CLIENT
 * |
 * From now on, every time ESP has new data
 * And wants to send to each CLIENT
 * |
 * ESP sends data in the form of events
 * With the following names (addEventListener) to the CLIENT
 *
 * **********************************************
 *
 * Original : /events ... (addEventListener) - for CLIENT (WEB)
 *          |             (events.send) - for SERVER (ESP)
 *          |             |
 *          |             relay1
 *          |             relay2
 *          |             relay3
 *          |             relay4
 *          |             |
 * Add New  :             timer
 *                        calendar
 *
 * Shorten  : /e ... 1  → "ON"/"OFF"
 *                   2  → "ON"/"OFF"
 *                   3  → "ON"/"OFF"
 *                   4  → "ON"/"OFF"
 *                   |
 *                   t  → ("t1"/"1") , ("t2"/"2") , ("t3"/"3") , ("t4"/"4")
 *                   c  → ("c1"/"1") , ("c2"/"2") , ("c3"/"3") , ("c4"/"4")
 */
#define URL_EVENT "/e" // events
//
#define EVENT_RELAY_1 "1" // relay1
#define EVENT_RELAY_2 "2" // relay2
#define EVENT_RELAY_3 "3" // relay3
#define EVENT_RELAY_4 "4" // relay4
//
#define EVENT_TIMER "t"    // For all Timers
#define EVENT_CALENDAR "c" // For all Calendars

/* ------------------------------------------------------------------------- */

#endif
