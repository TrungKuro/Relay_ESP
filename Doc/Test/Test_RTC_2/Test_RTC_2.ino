#include <Wire.h>
#include <SPI.h>

#include "RTClib.h"

RTC_DS1307 rtc;

String timeRTC = "";
String dateRTC = "";

void setup()
{
  Serial.begin(115200);
  Wire.begin(2, 14); // Chan 2 (SDA), 14 (SCL) I2C cua ESP8266

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }

  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}
void loop()
{
  DateTime now = rtc.now();

  // char buf1[] = "hh:mm";
  // Serial.println(now.toString(buf1));

  // char buf2[] = "YYMMDD-hh:mm:ss";
  // Serial.println(now.toString(buf2));

  // char buf3[] = "Today is DDD, MMM DD YYYY";
  // Serial.println(now.toString(buf3));

  // char buf4[] = "MM-DD-YYYY";
  // Serial.println(now.toString(buf4));

  char timeBuf[] = "hh:mm:ss";
  timeRTC = now.toString(timeBuf);
  Serial.println(timeRTC);

  char dateBuf[] = "DD/MM/YYYY";
  dateRTC = now.toString(dateBuf);
  Serial.println(dateRTC);

  delay(1000);
}
