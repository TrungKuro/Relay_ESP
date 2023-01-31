String inputData = "";

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(115200);

  Serial.println(F("-------------------------------------"));
  Serial.println(F("Enter commands below: ... example ..."));
  Serial.println(F("Pin 05 output HIGH = H:05"));
  Serial.println(F("Pin 13 output LOW = L:13"));
  Serial.println(F("Note: only for pin 4, 5, 12, 13, 16"));
  Serial.println(F("-------------------------------------"));
  Serial.println("");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(16, OUTPUT);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
}

// the loop function runs over and over again forever
void loop()
{
  // digitalWrite(16, HIGH); // turn the LED on (HIGH is the voltage level)
  // digitalWrite(4, HIGH);
  // digitalWrite(5, HIGH);
  // digitalWrite(12, HIGH);
  // digitalWrite(13, HIGH);
  // delay(2500); // wait for a second

  // digitalWrite(16, LOW); // turn the LED off by making the voltage LOW
  // digitalWrite(4, LOW);
  // digitalWrite(5, LOW);
  // digitalWrite(12, LOW);
  // digitalWrite(13, LOW);
  // delay(2500); // wait for a second

  Serial.println(F("-----------------"));
  Serial.println(F("Enter command ..."));
  Serial.println("");

  while (Serial.available() == 0)
  {
  }
  inputData = Serial.readString();
  int lengthOfData = inputData.length();
  Serial.println("-> " + inputData);

  if (inputData[1] == ':' && lengthOfData == 4)
  {
    int pin = inputData.substring(2).toInt();

    if (pin == 4 || pin == 5 || pin == 12 || pin == 13 || pin == 16)
    {
      if (inputData[0] == 'H')
      {
        digitalWrite(pin, HIGH);
      }
      else if (inputData[0] == 'L')
      {
        digitalWrite(pin, LOW);
      }
      else
      {
        Serial.println("");
      }
    }
    else
    {
      Serial.println("");
    }
  }
  else
  {
    Serial.println("");
  }

  inputData = "";
}
