/*
	Used pins for Arduino Nano:
		D9 = Temperature sensor pin
		D8 = LCD RS
		D7 = LCD RW (Currently not user)
		D6 = LCD E
		D5 = LCD Data (D4)
		D4 = LCD Data (D5)
		D3 = LCD Data (D6)
		D2 = LCD Data (D7)
*/

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define VERSION "button-feature"

const int
	lcdButtonPin = 10,
	tempSensorPin = 9,
	rs = 8,
	en = 6,
	d4 = 5,
	d5 = 4,
	d6 = 3,
	d7 = 2;

int
	sensorCount = 0,
	lcdButtonState = 0,
	lcdButtonLastState = 0,
	lcdButtonIndex = 0,
	tempIndex = 0;

float	tempBuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0, };

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

// ----------------------------------------------------------------- MAIN -----------------------------------------------------------------

void setup()
{
	pinMode(lcdButtonPin, INPUT_PULLUP);

	// ----- Serial settings -----
  sensors.begin();
  Serial.begin(9600);

	// Scan DSB18B20 sensors
  Serial.println("> Scanning...");
  sensorCount = sensors.getDeviceCount();
	Serial.print("> ");
  Serial.print(sensorCount, DEC);
	Serial.print("found ");
  Serial.println(" devices.");
  Serial.println("");

	// ----- LCD settings -----
	lcd.begin(16, 2);
	lcd.createChar(0, degreeChar);

	// Initial screen output
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
	lcdButtonState = digitalRead(lcdButtonPin);
  sensors.requestTemperatures();

  for (int i = 0;  i < sensorCount;  i++)
  {
		tempBuffer[i] = sensors.getTempCByIndex(i);

		// Serial temperature output
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(tempBuffer[i]);
		Serial.print(" ");
    degreeSerialSymbol();
    Serial.println("C");

		// LCD temperature output
		// lcd.setCursor(4, i);
		// lcd.print(tempBuffer[i]);
		// lcd.write(byte(0));
		// lcd.print("C");
  }

	if (lcdButtonState != lcdButtonLastState)
	{
		if (lcdButtonState == LOW)
		{
			tempIndex = (tempIndex + 1) % 4;

			lcd.clear();
			// lcd.setCursor(0, 0);
			// lcd.print(tempIndex);

			if (tempIndex == 0)
			{
				lcd.setCursor(0, 0);
				lcd.print("T1: ---");
				lcd.print(tempBuffer[0]);
				lcd.setCursor(0, 1);
				lcd.print("T2: ---");
			} else if (tempIndex == 1)
			{
				lcd.setCursor(0, 0);
				lcd.print("T3: ---");
				lcd.setCursor(0, 1);
				lcd.print("T4: ---");
			} else if (tempIndex == 2)
			{
				lcd.setCursor(0, 0);
				lcd.print("T5: ---");
				lcd.setCursor(0, 1);
				lcd.print("T6: ---");
			} else if (tempIndex == 3)
			{
				lcd.setCursor(0, 0);
				lcd.print("T7: ---");
				lcd.setCursor(0, 1);
				lcd.print("T8: ---");
			}



			// LCD temperature output
			// lcd.setCursor(4, 0);
			// lcd.print(tempBuffer[tempIndex]);
			// lcd.write(byte(0));
			// lcd.print("C");
		}
		delay(50);
	}

	lcdButtonLastState = lcdButtonState;
  Serial.println("");
	delay(500);
}