#include <Arduino.h>

// Choose just 1 or none of the 2 below
//#define COMPILE_TIME_SETUP  // uncomment if initializing from compilation time
//#define NTP_TIME_SETUP // uncomment to initialize from NTP server

#if defined(COMPILE_TIME_SETUP) && defined(NTP_TIME_SETUP)
#error Cannot use both kinds of initialize
#endif

/*
   IMPORTANT The DS3231 is unaware of timezone and daylight savings time
   When initialized it saves the given date/time and carries on, correcting for end of month/year/leap 
   year but no more.
   NTP knows about epoch (number of seconds since 1/1/1970 00:00 PST) and can be taught about timeone ans
   DST so can thus format into localtime
*/
// CONNECTIONS:
// DS3231 SDA --> SDA
// DS3231 SCL --> SCL
// DS3231 VCC --> 3.3v or 5v
// DS3231 GND --> GND

/* for software wire use below
#include <SoftwareWire.h>  // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

SoftwareWire myWire(SDA, SCL);
RtcDS3231<SoftwareWire> Rtc(myWire);
 for software wire use above */

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */

#ifdef NTP_TIME_SETUP
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield or ESP32
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include "WiFiUdp.h"
#include "NTP.h"

char ssid[]     = "******";
char password[] = "********";

#define NTP_Update_Interval 15000  // msec
WiFiUDP wifiUdp;
NTP ntp(wifiUdp);

#endif

void printDateTime(const RtcDateTime& dt);  
char ntpdate[20],ntptime[20];

void setup () 
{
    Serial.begin(57600);

    //--------RTC SETUP ------------
    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
    
    Rtc.Begin();
    RtcDateTime settime;
#ifdef COMPILE_TIME_SETUP
    strcpy(ntpdate,__DATE__);
    strcpy(ntptime,__TIME__);
    Serial.print("Compile: " );
#elif defined(NTP_TIME_SETUP)
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting ...");
    delay(500);
    }
    Serial.println("Connected");  
    // set rules for switching daylight saving to daylight times
    ntp.ruleDST("IST", Last, Sat, Mar, 2, 180); // last sunday in march 2:00, timetone +180min (+2 GMT + 1h summertime offset)
    ntp.ruleSTD("IDT", Last, Sun, Oct, 2, 120); // last sunday in october 3:00, timezone +120min (+2 GMT)
//  ntp.ntpServer("ntp.ubuntu.com");
    ntp.updateInterval(NTP_Update_Interval);
    ntp.begin();
    Serial.println("start NTP");
    delay(NTP_Update_Interval);
    while (!ntp.update()) {
        Serial.println("Waiting for NTP update");
        delay(5000);
    }
    strcpy(ntpdate,ntp.formattedTime("%b %d %Y"));
    strcpy(ntptime,ntp.formattedTime("%H %M %S"));
    Serial.print("NTP: " );
#endif
#if defined(COMPILE_TIME_SETUP) || defined(NTP_TIME_SETUP)
    settime = RtcDateTime(ntpdate, ntptime);
    printDateTime(settime);
    Serial.println();
#endif
    if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            Serial.println("RTC lost confidence in the DateTime!");

            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue
        }
        Serial.println("Recompile with COMPILE_TIME_SETUP or NTP_TIME_SETUP enabled");
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }


#if defined(COMPILE_TIME_SETUP) || defined(NTP_TIME_SETUP)
    RtcDateTime now = Rtc.GetDateTime();
    if (now < settime) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(settime);
    }
    else if (now > settime) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
        Rtc.SetDateTime(settime);
    }
    else if (now == settime) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
#endif
    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

void loop () 
{
    if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) the battery on the device is low or even missing and the power line was disconnected
            Serial.println("RTC lost confidence in the DateTime!");
        }
    }

    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();

	RtcTemperature temp = Rtc.GetTemperature();
	temp.Print(Serial);
	// you may also get the temperature as a float and print it
    // Serial.print(temp.AsFloatDegC());
    Serial.println("C");

    delay(2000); // ten seconds
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}
