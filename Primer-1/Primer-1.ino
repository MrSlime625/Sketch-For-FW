/*
 ** MOSI -  D7
 ** MISO -  D6
 ** SCK -   D5
 ** CS -    D8

 ** SDA -   D1
 ** SCL -   D2

 DHT11 -    D3
 DS18B20 -  D4
 SD -       D8
*/


#include <SPI.h>
#include <SD.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTTYPE DHT22

#include <DS_raw.h>
#include <microDS18B20.h>
#include <microOneWire.h>

#include "RTClib.h"


// Pins
const uint8_t pinChipSelectSD = D8;
const uint8_t pinDHT = D3;
const uint8_t pinLED = D0;

// Files
String fileLogs = "Logs.txt";
String fileData = "sheetData.txt";

// Sensors
RTC_DS1307 rtc; // RTC module DS1307
MicroDS18B20<D4> ds; // Thermometer DS18B20
DHT dht(pinDHT, DHTTYPE); // Thermometer/Hygrometer DHT22



void setup() {
  Serial.begin(115200);

  // LED setup ----------------------------------------------------------------------------------------------
  pinMode(pinLED, OUTPUT);

  // DS18B20 setup ------------------------------------------------------------------------------------------
  ds.setResolution(12);
  delay(1000);

  // DHT22 setup --------------------------------------------------------------------------------------------
  dht.begin();

  // RTC setup ----------------------------------------------------------------------------------------------
  if (!rtc.begin()) {
    Serial.println("DS3231 не найден");
    LedIndication(2);
  } else {
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set the time of the RTC module when it is first turned on
  }

  // SD Card setup ------------------------------------------------------------------------------------------
  Serial.print(F("Инициализация модуля SD card "));
  if (!SD.begin(pinChipSelectSD)) {
    Serial.println(F(" - ошибка или SD карта не вставлена"));
    LedIndication(2);
    return;
  } else {
    Serial.println(F(" - прошла успешно."));
    Serial.print(F("Log-фаил: "));
    Serial.println(fileLogs);
  }
  delay(3000);
}

void loop() {

  // RTC Read -----------------------------------------------------------------------------------------------
  DateTime now = rtc.now();

  String nowData = String(now.day()) + F(".") + String(now.month()) + F(".") + String(now.year()) + F(" ") + String(now.hour()) + F(":") + String(now.minute()) + F(":") + String(now.second()) + F(" ");
  
  if (now.year() != 2024) {
    Serial.println(F("Модуль RTC отвалился"));
    LedIndication(2);
  }

  // DHT22 Read ---------------------------------------------------------------------------------------------
  float tempDHT = dht.readTemperature();
  float humDHT = dht.readHumidity();

  String tempHumDHT22 = F("DHT22-t: ") + String(tempDHT) + F("°C DHT22-h: ") + String(humDHT) + F("%\n");

  if (isnan(tempDHT) || isnan(humDHT)) {
    Serial.println(F("Датчик DHT22 при опросе вернул Nan"));
    LedIndication(2);
  }

  // DS18B20 Read -------------------------------------------------------------------------------------------
  ds.requestTemp();
  delay(980);

  float temperatureDS18B20 = ds.getTemp();

  String tempDS18B20 = F("DS18B20: ") + String(temperatureDS18B20) + F("°C ");

  // FC-28 Read ---------------------------------------------------------------------------------------------
  int16_t humidityFC28 = analogRead(A0);
  int8_t humFC28Procent = map(humidityFC28, 250, 1024, 100, 0);

  String humFC28 = F("FC-28: ") + String(humFC28Procent) + F("% ");

  // Write to SD --------------------------------------------------------------------------------------------
  if (ds.readTemp()) {
    String resultStrokeLog = nowData + tempDS18B20 + humFC28 + tempHumDHT22;
    String resultStrokeData = String(now.day()) + F(",") + String(now.month()) + F(",") + String(now.year()) + F(",") + String(now.hour()) + F(",") + String(now.minute()) + F(",") + String(now.second()) + F(",") + String(tDS18B20) + F(",") + String(solFC28) + F(",") + String(solFC28Procent) + F(",") + String(tempDHT) + F(",") + String(humDHT) + F("\n");
    
    Serial.println(resultStrokeLog);

    WriteToSD(fileLogs, resultStrokeLog);
    WriteToSD(fileData, resultStrokeData);
  } else {
    Serial.println(F("Датчик DS18B20 не читается"));
    LedIndication(2);
  }

  // Led ----------------------------------------------------------------------------------------------------
  if (humFC28Procent <= 20) {
    LedIndication(1);
  } else {
    LedIndication(0);
  }
  delay(3000);
}

void WriteToSD(String fileName, String content) {
  // Write File ---------------------------------------------------------------------------------------------
  File fileOnSD = SD.open(fileName, FILE_WRITE);

  if (fileOnSD) {
    fileOnSD.print(content);
    fileOnSD.close();
  } else {
    Serial.println(F("Не могу записать в файл."));
    LedIndication(2);
  }
  delay(100);
}

void LedIndication(uint8_t state) {
  /* State:
  0 - all is ok
  1 - pour
  2 - error
  */
  switch (state) {
    case 0:
      digitalWrite(pinLED, LOW);
      break;
    case 1:
      digitalWrite(pinLED, HIGH);
      break;
    case 2:
      for (;;) {
        digitalWrite(pinLED, HIGH);
        delay(100);
        digitalWrite(pinLED, LOW);
        delay(100);
      }
      break;
  }
}