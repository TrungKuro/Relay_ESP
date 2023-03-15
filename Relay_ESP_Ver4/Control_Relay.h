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
/*                         FUNCTION WEB SERVER (ESP)                         */
/* ------------------------------------------------------------------------- */

/* Handling the state of Relay */
String stateRelay(uint8_t output);

/* Handling the state of Slider Checkbox */
String stateCheckbox(uint8_t output);

/* Replace placeholder with RTC values and state Relay */
String processor(const String &var);

/* ------------------------------------------------------------------------- */

#endif