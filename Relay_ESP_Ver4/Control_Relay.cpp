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
/*                         FUNCTION WEB SERVER (ESP)                         */
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

  return String();
}

/* ------------------------------------------------------------------------- */
