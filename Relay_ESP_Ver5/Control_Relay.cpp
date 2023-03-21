#include "Control_Relay.h"

/* ------------------------------------------------------------------------- */
/*                              FUNCTION BUTTON                              */
/* ------------------------------------------------------------------------- */

/* Detect button, catch the "Edge-Up" */
char detectButton(bool *ptrStatusBtn)
{
  int btn = analogRead(BUTTON);

  if (btn >= BUTTON_1_MIN && btn <= BUTTON_4_MAX)
  {
    /* Only need read ADC, when "statusBtn" is FALSE
    ** This means that the button is not currently pressed
    ** Skip if "statusBtn" still TRUE
    */
    if (!(*ptrStatusBtn))
    {
      delay(DEBOUNCE); // Skip Debounce (if have)
      btn = analogRead(BUTTON);

      if (btn >= BUTTON_1_MIN && btn <= BUTTON_1_MAX)
      {
        DEBUG_PRINTLN(F("Btn 1"));

        *ptrStatusBtn = true;
        return BTN_1;
      }
      else if (btn >= BUTTON_2_MIN && btn <= BUTTON_2_MAX)
      {
        DEBUG_PRINTLN(F("Btn 2"));

        *ptrStatusBtn = true;
        return BTN_2;
      }
      else if (btn >= BUTTON_3_MIN && btn <= BUTTON_3_MAX)
      {
        DEBUG_PRINTLN(F("Btn 3"));

        *ptrStatusBtn = true;
        return BTN_3;
      }
      else if (btn >= BUTTON_4_MIN && btn <= BUTTON_4_MAX)
      {
        DEBUG_PRINTLN(F("Btn 4"));

        *ptrStatusBtn = true;
        return BTN_4;
      }
      else
      {
        *ptrStatusBtn = false;
        return NONE_BTN; // Caused by Debounce
      }
    }
    else
    {
      return NONE_BTN; // Button being held (Hold Click)
    }
  }
  else
  {
    *ptrStatusBtn = false;
    return NONE_BTN; // Button is being released
  }
}

/* ------------------------------------------------------------------------- */
/*                               FUNCTION TIMER                              */
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

/* Calculating when to activate the Flags for Timer */
void calculateFlagTimer(byte addr, RTC_DS1307 *ptrRTC, byte h[], byte m[], byte s[], uint32_t flag[])
{
  DateTime now = ptrRTC->now();

  /* Calculate a date which is ? hours, ? minutes, and ? seconds into the future */
  DateTime future(now + TimeSpan(0, h[addr], m[addr], s[addr]));
  flag[addr] = future.unixtime();
}

/* ------------------------------------------------------------------------- */
/*                             FUNCTION CALENDAR                             */
/* ------------------------------------------------------------------------- */

/* Handling the time ON and time OFF of Calendar */
String timeCalendar(byte h, byte m)
{
  String timeCal = "";

  if (h >= 10)
  {
    timeCal += String(h) + ":";
  }
  else if (h >= 0)
  {
    timeCal += "0" + String(h) + ":";
  }

  if (m >= 10)
  {
    timeCal += String(m);
  }
  else if (m >= 0)
  {
    timeCal += "0" + String(m);
  }

  return timeCal;
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

/* Calculating when to activate the Flags for Calendar */
void calculateFlagCalendar(byte addr, RTC_DS1307 *ptrRTC, byte hON[], byte mON[], byte hOFF[], byte mOFF[], bool flagON[], bool flagOFF[])
{
  DateTime now = ptrRTC->now();

  /* Calendar ON */
  if (now.hour() < hON[addr])
  {
    flagON[addr] = true;
  }
  else if (now.hour() == hON[addr] && now.minute() < mON[addr])
  {
    flagON[addr] = true;
  }
  else
  {
    flagON[addr] = false;
  }

  /* Calendar OFF */
  if (now.hour() < hOFF[addr])
  {
    flagOFF[addr] = true;
  }
  else if (now.hour() == hOFF[addr] && now.minute() < mOFF[addr])
  {
    flagOFF[addr] = true;
  }
  else
  {
    flagOFF[addr] = false;
  }
}

/* ------------------------------------------------------------------------- */
/*                               FUNCTION RELAY                              */
/* ------------------------------------------------------------------------- */

/* Handling the state of Relay */
String stateRelay(uint8_t output)
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
String stateCheckbox(uint8_t output)
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
