/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com
*********/

// const int analogInPin = A0; // ESP8266 Analog Pin ADC0 = A0

// int sensorValue = 0; // value read from the pot

// void setup()
// {
//   // initialize serial communication at 115200
//   Serial.begin(115200);
// }

// void loop()
// {
//   // read the analog in value
//   sensorValue = analogRead(analogInPin);

//   // print the readings in the Serial Monitor
//   Serial.println(sensorValue);

//   delay(1000);
// }

/* -------------- DEBUG (uncomment to open the Debug function) ------------- */

#define ENABLE_DEBUG

#if defined(ENABLE_DEBUG)
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

/* --------------------------------- Button -------------------------------- */

/* Read ADC from pin "TOUT" (A0)
** When no 'click' the button, the value voltage is 0V ~ 3 ADC
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

bool stateBtn = false;

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

/* ------------------------------------------------------------------------- */
/*                                    MAIN                                   */
/* ------------------------------------------------------------------------- */

void setup()
{
  /* To use the Debug feature */
  Serial.begin(115200);
}

void loop()
{
  detectButton();
  delay(10);
}