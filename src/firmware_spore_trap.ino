#include <Adafruit_SHT31.h>
#include <DS3231.h>
#include <Wire.h>
#include <SPI.h>
#include <XBee.h>
#include <SD.h>
#include <math.h>
#include <string.h>

using namespace std;

int RTCAddress = 0x68;
int HTSAddress = 0x44;
int motorPin1 = 5;
int motorPin2 = 6;
int motorSpeed = 10; 
int batteryVoltage = A0;
int vaporSensor = A1;

const int stringLen = 50;

XBee xbee = XBee();

uint8_t payload[stringLen] = {0};

Adafruit_SHT31 sht31 = Adafruit_SHT31();

DS3231 clock;

const int chipSelect = 4;
const int sw = 3;
int t = 0;

XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x00000000);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

bool century = false;
bool h12Flag;
bool pmFlag;

void setup()
{
  pinMode(motorPin2, OUTPUT);
  pinMode(motorSpeed, OUTPUT);

  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);

  Serial.begin(9600);
  Wire.begin();
  xbee.setSerial(Serial);

  pinMode(sw, INPUT);
  pinMode(vaporSensor, INPUT);
  pinMode(batteryVoltage, INPUT);

  while (!Serial)
    delay(1);

  //Serial.print("Initializing SD card...");
  while (!SD.begin(chipSelect))
  {
    //Serial.println("Card failed, or not present");
    delay(500);
  }
  Serial.println("card initialized.");
  if (!sht31.begin(HTSAddress))
  {
   //Serial.println("Couldn't find SHT31");
  }

}

void loop()
{
  String dataString = "";
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  float v = analogRead(vaporSensor) * (5.0 / 1023.0);
  float b = analogRead(batteryVoltage) * (5.0 / 1023.0);

  dataString += "d1,t";
  dataString += String(t);
  dataString += ",h";
  dataString += String(h);
  dataString += ",v";
  dataString += String(v);
  dataString += ",b";
  dataString += String(b);
  dataString += ",";
  dataString += String(clock.getDate());
  dataString += '/';
  dataString += String(clock.getMonth(century));
  dataString += '/';
  dataString += String(clock.getYear());
  dataString += ",";
  dataString += String(clock.getHour(h12Flag, pmFlag));
  dataString += ':';
  dataString += String(clock.getMinute());
  dataString += ':';
  dataString += String(clock.getSecond());

  for(int i = (stringLen-1); i >= 0; i--){
    payload[i] = dataString[i] & 0xFF;
  }
  //Serial.println(dataString);

  xbee.send(zbTx);

  if (digitalRead(sw) == LOW)
  {
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    if (dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
    }
    else
    {
      //Serial.println("error opening datalog.txt");
    }

    t++;
  }

  delay(5000);
}
