unsigned long begin;
unsigned long end;

void setup()
{
  Serial.begin(115200);
  pinMode(9, OUTPUT);
}

void loop()
{
  begin = micros();
  digitalWrite(9, HIGH);
  end = micros();
  Serial.print("digitalWrite pin9 HIGH: ");
  Serial.println(end - begin);

  begin = micros();
  digitalWrite(9, LOW);
  end = micros();
  Serial.print("digitalWrite pin9 LOW: ");
  Serial.println(end - begin);

  // PortB --|--13|12|11|10|09|08

  begin = micros();
  PORTB |= B00000010; // Sets only D9 to HIGH
  end = micros();
  Serial.print("PortB pin9 HIGH: ");
  Serial.println(end - begin);

  begin = micros();
  PORTB &= !B00000010; // Sets only D9 to LOW (we use ! to invertt the byte)
  end = micros();
  Serial.print("PortB pin9 LOW: ");
  Serial.println(end - begin);

  delay(3000);
  Serial.println();
  Serial.println("- - - - -");
  Serial.println();
}