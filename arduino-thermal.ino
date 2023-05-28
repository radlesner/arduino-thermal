/*
	Used pins for Arduino Nano and Uno:
		D8 = LCD RS
		D9 = LCD E
		D5 = LCD Data (D4)
		D4 = LCD Data (D5)
		D3 = LCD Data (D6)
		D2 = LCD Data (D7)
		D7 = Temperature sensor pin
*/

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define VERSION "v0.2"

const int rs = 8,
					en = 6,
					d4 = 5,
					d5 = 4,
					d6 = 3,
					d7 = 2,
					tempSensorPin = 9;

int sensorCount = 0;

float tempC;

OneWire oneWire(tempSensorPin);	
DallasTemperature sensors(&oneWire);
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte degreeChar [8] = {
	0b00010,
	0b00101,
	0b00010,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
};

void degreeSerialSymbol() {
	if (Serial)
	{
		Serial.write(0xC2);
		Serial.write(0xB0);
	}
}

// ------------------------------------- MAIN -------------------------------------

void setup()
{
  sensors.begin();
  Serial.begin(9600);
  
  Serial.println("Scanning...");
  Serial.print("Found ");
  sensorCount = sensors.getDeviceCount();
  Serial.print(sensorCount, DEC);
  Serial.println(" devices.");
  Serial.println("");


	lcd.begin(16, 2);
	lcd.createChar(0, degreeChar);
	lcd.setCursor(0, 0);

	lcd.print("Arduino Thermal");
	lcd.setCursor(0, 1);
	lcd.print(VERSION);
	delay(2000);
	lcd.clear();

	lcd.setCursor(0, 0);
	lcd.print("T1: ---");

	lcd.setCursor(0, 1);
	lcd.print("T2: ---");
	delay(500);
}

void loop()
{ 
  sensors.requestTemperatures(); 
  
  for (int i = 0;  i < sensorCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(": ");
    tempC = sensors.getTempCByIndex(i);
    Serial.print(tempC);
		Serial.print(" ");
    degreeSerialSymbol();
    Serial.println("C");

		lcd.setCursor(4, i);
		lcd.print(tempC);
		lcd.write(byte(0));
		lcd.print("C");
  }
  
  Serial.println("");
	delay(500);
}