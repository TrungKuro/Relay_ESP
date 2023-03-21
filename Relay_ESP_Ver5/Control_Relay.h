#ifndef _CONTROL_RELAY_H_
#define _CONTROL_RELAY_H_

/* ------------------------------------------------------------------------- */
/*                                  LIBRARY                                  */
/* ------------------------------------------------------------------------- */

#include "Arduino.h"
#include "Pin_Connect.h"

#include <ESP8266WiFi.h>

#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"

/* ------------------------------------------------------------------------- */
/*                              FUNCTION BUTTON                              */
/* ------------------------------------------------------------------------- */

/* Detect button, catch the "Edge-Up" */
char detectButton(bool *ptrStatusBtn);

/* ------------------------------------------------------------------------- */
/*                               FUNCTION TIMER                              */
/* ------------------------------------------------------------------------- */

/* Handling the state of Radio ON Timer */
String stateTrigON(bool state);

/* Handling the state of Radio OFF Timer */
String stateTrigOFF(bool state);

/* Handling the state of Slider Timer */
String stateTimer(bool state);

/* ------------------------------------------------------------------------- */

/* Calculating when to activate the Flags for Timer */
void calculateFlagTimer(byte addr, RTC_DS1307 *ptrRTC, byte h[], byte m[], byte s[], uint32_t flag[]);

/* ------------------------------------------------------------------------- */
/*                             FUNCTION CALENDAR                             */
/* ------------------------------------------------------------------------- */

/* Handling the time ON and time OFF of Calendar */
String timeCalendar(byte h, byte m);

/* Handling the state of Slider Calendar */
String stateCalendar(bool state);

/* ------------------------------------------------------------------------- */

/* Calculating when to activate the Flags for Calendar */
void calculateFlagCalendar(byte addr, RTC_DS1307 *ptrRTC, byte hON[], byte mON[], byte hOFF[], byte mOFF[], bool flagON[], bool flagOFF[]);

/* ------------------------------------------------------------------------- */
/*                               FUNCTION RELAY                              */
/* ------------------------------------------------------------------------- */

/* Handling the state of Relay */
String stateRelay(uint8_t output);

/* Handling the state of Slider Checkbox */
String stateCheckbox(uint8_t output);

/* ------------------------------------------------------------------------- */

#endif