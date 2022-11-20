#include <Wire.h>
#include <SPI.h>

#include "RTClib.h"
RTC_DS1307 RTC;
void setup()
{
  Wire.begin(2, 14); // Chan 2 (SDA), 14 (SCL) I2C cua ESP8266
  RTC.begin();       // Khoi dong RTC
  delay(500);
  // Dong bo thoi gian voi may tinh
  RTC.adjust(DateTime(__DATE__, __TIME__));
  delay(1000);
  Serial.begin(115200); // Khoi dong serial port de lay du lieu
}
void loop()
{
  DateTime now = RTC.now(); // Thoi gian = thoi gian RTC hien tai
  // In thời gian
  Serial.print(now.year(), DEC); // Năm
  Serial.print('/');
  Serial.print(now.month(), DEC); // Tháng
  Serial.print('/');
  Serial.print(now.day(), DEC); // Ngày
  Serial.print(' ');
  Serial.print(now.hour(), DEC); // Giờ
  Serial.print(':');
  Serial.print(now.minute(), DEC); // Phút
  Serial.print(':');
  Serial.print(now.second(), DEC); // Giây
  Serial.println();
  delay(1000); // Delay
}

/* ------------------------------------------------------------------------- */

// // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
// #include <Wire.h>
// #include <SPI.h>
// #include "RTClib.h"

// RTC_DS1307 rtc;

// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// void setup()
// {
//   // Wire.begin(2, 14); // Chan SDA, SCL I2C cua ESP8266
//   Wire.begin();

//   rtc.begin(); // Khoi dong RTC
//   delay(500);

//   // When time needs to be set on a new device, or after a power loss, the
//   // following line sets the RTC to the date & time this sketch was compiled
//   // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//   // This line sets the RTC with an explicit date & time, for example to set
//   // January 21, 2014 at 3am you would call:
//   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

//   // When time needs to be re-set on a previously configured device, the
//   // following line sets the RTC to the date & time this sketch was compiled
//   // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//   // This line sets the RTC with an explicit date & time, for example to set
//   // January 21, 2014 at 3am you would call:
//   rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

//   delay(1000);
//   Serial.begin(115200); // Khoi dong serial port de lay du lieu
// }

// void loop()
// {
//   DateTime now = rtc.now();

//   Serial.print(now.year(), DEC);
//   Serial.print('/');
//   Serial.print(now.month(), DEC);
//   Serial.print('/');
//   Serial.print(now.day(), DEC);
//   Serial.print(" (");
//   Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
//   Serial.print(") ");
//   Serial.print(now.hour(), DEC);
//   Serial.print(':');
//   Serial.print(now.minute(), DEC);
//   Serial.print(':');
//   Serial.print(now.second(), DEC);
//   Serial.println();

//   Serial.print(" since midnight 1/1/1970 = ");
//   Serial.print(now.unixtime());
//   Serial.print("s = ");
//   Serial.print(now.unixtime() / 86400L);
//   Serial.println("d");

//   // calculate a date which is 7 days, 12 hours, 30 minutes, and 6 seconds into the future
//   DateTime future(now + TimeSpan(7, 12, 30, 6));

//   Serial.print(" now + 7d + 12h + 30m + 6s: ");
//   Serial.print(future.year(), DEC);
//   Serial.print('/');
//   Serial.print(future.month(), DEC);
//   Serial.print('/');
//   Serial.print(future.day(), DEC);
//   Serial.print(' ');
//   Serial.print(future.hour(), DEC);
//   Serial.print(':');
//   Serial.print(future.minute(), DEC);
//   Serial.print(':');
//   Serial.print(future.second(), DEC);
//   Serial.println();

//   Serial.println();
//   delay(3000);
// }

/* ------------------------------------------------------------------------- */

// #include <Wire.h>

// const byte DS1307 = 0x68;
// const byte NumberOfFields = 7;
// int second, minute, hour, day, wday, month, year;

// void setup()
// {
//   Wire.begin(2, 14);
//   setTime(1, 0, 0, 1, 1, 1, 18); // 1:00:00  1-1-2018
//   Serial.begin(9600);
// }
// void loop()
// {
//   readDS1307();
//   digitalClockDisplay();
//   delay(1000);
// }
// void readDS1307()
// {
//   Wire.beginTransmission(DS1307);
//   Wire.write((byte)0x00);
//   Wire.endTransmission();
//   Wire.requestFrom(DS1307, NumberOfFields);
//   second = bcd2dec(Wire.read() & 0x7f);
//   minute = bcd2dec(Wire.read());
//   hour = bcd2dec(Wire.read() & 0x3f);
//   wday = bcd2dec(Wire.read());
//   day = bcd2dec(Wire.read());
//   month = bcd2dec(Wire.read());
//   year = bcd2dec(Wire.read());
//   year += 2000;
// }
// int bcd2dec(byte num)
// {
//   return ((num / 16 * 10) + (num % 16));
// }
// int dec2bcd(byte num)
// {
//   return ((num / 10 * 16) + (num % 10));
// }
// void digitalClockDisplay()
// {
//   Serial.print(hour);
//   printDigits(minute);
//   printDigits(second);
//   Serial.print(" ");
//   Serial.print(day);
//   Serial.print(" ");
//   Serial.print(month);
//   Serial.print(" ");
//   Serial.print(year);
//   Serial.println();
// }

// void printDigits(int digits)
// {
//   Serial.print(":");
//   if (digits < 10)
//     Serial.print('0');
//   Serial.print(digits);
// }
// void setTime(byte hr, byte min, byte sec, byte wd, byte d, byte mth, byte yr)
// {
//   Wire.beginTransmission(DS1307);
//   Wire.write(byte(0x00));
//   Wire.write(dec2bcd(sec));
//   Wire.write(dec2bcd(min));
//   Wire.write(dec2bcd(hr));
//   Wire.write(dec2bcd(wd));
//   Wire.write(dec2bcd(d));
//   Wire.write(dec2bcd(mth));
//   Wire.write(dec2bcd(yr));
//   Wire.endTransmission();
// }