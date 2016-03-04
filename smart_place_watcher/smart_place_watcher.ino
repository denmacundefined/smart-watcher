// define section
#define DHTPIN 9
#define DHTTYPE DHT11
#define DISPLAYSCLKPIN 2
#define DISPLAYDNPIN 3
#define DISPLAYDCPIN 4
#define DISPLAYRSTPIN 5
#define DISPLAYSCEPIN 6
#define DISPLAYLEDPIN 7
#define BUTTONPIN1 A7
#define BUTTONPIN2 A3
#define GASPIN A2
#define VIBROPIN A1
#define VIBROGROUNDPIN 13
#define FLAMEPIN A0
#define LIGHTPIN A6
#define BUZZERPIN 10
#define DEBUGTXPIN 12
#define DEBUGRXPIN 11
#define EDITPIN 8
#define MINLIMITFORWORK 22
#define SIGNALTIME 50
#define SHOWDISPLAYCOUNT 5
#define SHOWEDITCOUNT 10

// include section
#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

// global variable section
RTC_DS1307 rtc;
Adafruit_PCD8544 display = Adafruit_PCD8544(DISPLAYSCLKPIN, DISPLAYDNPIN, DISPLAYDCPIN, DISPLAYSCEPIN, DISPLAYRSTPIN);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
SoftwareSerial DEBUG(DEBUGRXPIN, DEBUGTXPIN);
byte DisplayIndex = 0;
int EditModeValue = 0;
boolean EditMode = false;
int TimeConfig[5];

// config section
void setup() {
  
  //debug
  DEBUG.begin(9600);
  
  // set pin type 
  pinMode(BUTTONPIN1, INPUT);
  pinMode(BUTTONPIN2, INPUT);
  pinMode(GASPIN, INPUT);
  pinMode(VIBROPIN, INPUT);
  pinMode(FLAMEPIN, INPUT);
  pinMode(DISPLAYLEDPIN, OUTPUT);
  pinMode(LIGHTPIN, INPUT);
  pinMode(EDITPIN, INPUT);
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(VIBROGROUNDPIN, OUTPUT);
  
  // init vibrosensor
  digitalWrite(VIBROGROUNDPIN, LOW);
  
  // init display
  display.begin();
   
  // init temp
  dht.begin();

  // init barometr
  if (! bmp.begin()) {
    DEBUG.println("Could not find BMP085 sensor");
    while (1);
  }
  
  // init time 
  if (! rtc.begin()) {
    DEBUG.println("Could not find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    DEBUG.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } 

  // init wifi
  Serial.begin(115200);
  Serial.setTimeout(5000);
  Serial.println("AT+RST");
  delay(1000);
  DEBUG.println(Serial.readString());
  /*if (Serial.find("ready")) {
    DEBUG.print("WiFi - Module is ready");
  }else{
    DEBUG.print("Module dosn't respond.");
    while(1);
  }*/
  
}

// main section
void loop() { 

  // get data from sensors
    float humidity = dht.readHumidity() - EEPROM.read(6);
    float temp = dht.readTemperature() - EEPROM.read(5);
    float tempFromBarometr = bmp.readTemperature();
    long pressure = bmp.readPressure();
    int altitude = bmp.readAltitude();
    int light = analogRead(LIGHTPIN);
    int gas = analogRead(GASPIN);
    int flame = analogRead(FLAMEPIN);
    int vibro = analogRead(VIBROPIN);
    boolean editButton = digitalRead(EDITPIN);
    byte contrast = EEPROM.read(3);
    DateTime now = rtc.now();
    int year = now.year();
    byte month = now.month();
    byte day = now.day();
    byte hour = now.hour();
    byte minute = now.minute();
    byte second = now.second();
    
  // start functions
    InitEditMode(editButton, SHOWEDITCOUNT);
    if (! EditMode) {
      CheckButtons(EEPROM.read(4), analogRead(BUTTONPIN1), analogRead(BUTTONPIN2), SHOWDISPLAYCOUNT, 0);
    }
    DisplayLedPowerOn(light, EEPROM.read(0));
    PowerOnSignalExpression(gas, EEPROM.read(1), SIGNALTIME, true);
    PowerOnSignalExpression(flame, EEPROM.read(2), SIGNALTIME, false);

  // start display and set view
    display.clearDisplay();
    contrast = (contrast < MINLIMITFORWORK) ? MINLIMITFORWORK : contrast;
    display.setContrast(contrast);
    display.setTextColor(BLACK);
    if (EditMode) {
      if (DisplayIndex == 0) {
          DefaultTempleteEditTime(2000, 3000, "Виберiть рiк:", 0);
      }
      if (DisplayIndex == 1) {
          DefaultTempleteEditTime(1, 12, "Виберiть мiсяць:", 1);
      }
      if (DisplayIndex == 2) {
          DefaultTempleteEditTime(1, 31, "Виберiть день:", 2);
      }
      if (DisplayIndex == 3) {
          DefaultTempleteEditTime(0, 23, "Виберiть годину:", 3);
      }
      if (DisplayIndex == 4) {
          DefaultTempleteEditTime(0, 59, "Виберiть хвилину:", 4);
      }
      if (DisplayIndex == 5) {
          rtc.adjust(DateTime(TimeConfig[0], TimeConfig[1], TimeConfig[2], TimeConfig[3], TimeConfig[4], 0)); //year, month, day, hour, minute
          DefaultTempleteEditEpprom(0, 255, "Виберiть лiмiт освiтлення:", 0);
      }
      if (DisplayIndex == 6) {
          DefaultTempleteEditEpprom(0, 255, "Виберiть лiмiт газу:", 1);
      }
      if (DisplayIndex == 7) {
          DefaultTempleteEditEpprom(0, 255, "Виберiть лiмiт вогню:", 2);
      }
      if (DisplayIndex == 8) {
          DefaultTempleteEditEpprom(MINLIMITFORWORK, 100, "Виберiть контраст:", 3);
      }
      if (DisplayIndex == 9) {
          DefaultTempleteEditEpprom(MINLIMITFORWORK, 255, "Виберiть вiдстань реагування кнопок:", 4);
      }
      if (DisplayIndex == 10) {
          DefaultTempleteEditEpprom(0, 255, "Виберiть декремент температури:", 5);
      }
      if (DisplayIndex == 11) {
          DefaultTempleteEditEpprom(0, 255, "Виберiть  декремент вологостi:", 6);
      }
    } else {    
      if (DisplayIndex == 0) {
          ShowTime(year, month, day, hour, minute, second);
      }
      if (DisplayIndex == 1) {
          DefaultTempleteShow("Температура:", temp, " градусiв", "Вологiсть: ", humidity, " процентiв");
      }
      if (DisplayIndex == 2) {
          DefaultTempleteShow("Тиск:", pressure, " паск.", "Висота: ", altitude, " метрiв");
      }
      if (DisplayIndex == 3) {
          DefaultTempleteShow("Освiтлення:", light, " число", "Температура з барометра: ", tempFromBarometr, " градусiв");
      }
      if (DisplayIndex == 4) {
          DefaultTempleteShow("Гази:", gas, " число", "Вогонь: ", flame, "  число");
      }
      if (DisplayIndex == 5) {
          DefaultTempleteShow("Вiбрацiя:", vibro, "  число", "", 0, "");
      }
    }
    display.display();
}

// function section
void InitEditMode (boolean value, byte Limit) {
  if (!value) {
    if (EditMode) {
      if (DisplayIndex > Limit) {
        DisplayIndex = 0;
        EditMode = false;
      } else {
        DisplayIndex++;
      }    
    } else {
      EditMode = true;
      DisplayIndex = 0;
    }
  }
}

void CheckButtons (byte DefaultLimit, int Decrement, int Increment, byte MaxLimit, byte TimeDelay) {
  DefaultLimit = (DefaultLimit < MINLIMITFORWORK) ? MINLIMITFORWORK : DefaultLimit;
  if (EditMode) {
    if ((Decrement > DefaultLimit)) {
      EditModeValue--;
      delay(TimeDelay);
    } 
    if ((Increment > DefaultLimit)) {
      EditModeValue++;
      delay(TimeDelay);
    }
  } else {
    if ((Decrement > DefaultLimit) && (DisplayIndex > 0)) {
      DisplayIndex--;
      delay(TimeDelay);
    } 
    if ((Increment > DefaultLimit) && (DisplayIndex < MaxLimit)) {
      DisplayIndex++;
      delay(TimeDelay);
    }
  }
}

void PowerOnSignalExpression (int value, int checkValue, byte interval, boolean more) {
  boolean validation;
  if (more) {
      validation = (value > checkValue) ? true : false;
  } else {
      validation = (value < checkValue) ? true : false;
  }
  if (validation) {
    analogWrite (BUZZERPIN, 255);
    delay (interval);
    analogWrite (BUZZERPIN, 0);
  }
}

void DisplayLedPowerOn (int light, byte DefaultLimit) {
  if (light < DefaultLimit) {
    digitalWrite(DISPLAYLEDPIN, HIGH);
  } else {
    digitalWrite(DISPLAYLEDPIN, LOW);
  }  
}

// views
void ShowTime (int year, byte month, byte day, byte hour, byte minute, byte second) {   
   display.setTextSize(1);
   display.setCursor(14, 5);
   display.println(day, DEC);
   display.print("/");
   display.print(month, DEC);
   display.print("/");
   display.print(year, DEC);
   display.setCursor(12, 25);
   display.setTextSize(2);
   if (hour < 10) {
     display.print("0");
   }
   display.print(hour, DEC);
   display.print("-");
   if (minute < 10) {
     display.print("0");
   }
   display.print(minute, DEC);
   display.setTextSize(1);
   display.setCursor(35, 22);
   display.print(second, DEC);
}

void DefaultTempleteShow (char caption1[], long value1, char text1[], char caption2[], long value2, char text2[]) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print(caption1);
  display.setCursor(10, 10);
  display.print(value1);
  display.print(text1);
  if (value2 != 0) {
    display.setCursor(0, 22);
    display.print(caption2);
    display.setCursor(10, 39);
    display.print(value2);
    display.print(text2);
  }
}

//edit mode
void DefaultTempleteEditEpprom (byte minLimit, byte maxLimit, char text[], int epprom ) {
    EditModeValue = EEPROM.read(epprom);
    CheckButtons(EEPROM.read(4), analogRead(BUTTONPIN1), analogRead(BUTTONPIN2), SHOWDISPLAYCOUNT, 0);
    if ((EditModeValue < minLimit) or (EditModeValue > maxLimit)) {
       EditModeValue = minLimit;
    } 
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(text);
    display.setCursor(22, 30);
    display.setTextSize(2);
    display.print(EditModeValue);
    EEPROM.write(epprom, EditModeValue);
}

void DefaultTempleteEditTime (int minLimit, int maxLimit, char text[], byte index ) {
    CheckButtons(EEPROM.read(4), analogRead(BUTTONPIN1), analogRead(BUTTONPIN2), SHOWDISPLAYCOUNT, 0);
    if ((EditModeValue < minLimit) or (EditModeValue > maxLimit)) {
       EditModeValue = minLimit;
    } 
    TimeConfig[index] = EditModeValue;
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(text);
    display.setCursor(20, 25);
    display.setTextSize(2);
    display.print(TimeConfig[index]);
}
