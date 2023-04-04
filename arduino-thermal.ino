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

#define VERSION "v0.1"

const int rs = 8,
					en = 6,
					d4 = 5,
					d5 = 4,
					d6 = 3,
					d7 = 2,
					tempSensorPin = 9;

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

OneWire ds(tempSensorPin);

void degreeSerialSymbol() {
	if (Serial)
	{
		Serial.write(0xC2);
		Serial.write(0xB0);
	}
}

float tempConvertCelsius(float rawTemp) {
	float tempCelsius = rawTemp / 16.0;
	return tempCelsius;
}

float tempConvertFahrenheit(float rawTemp) {
	float tempCelsius, tempFahrenheit;

	tempCelsius = rawTemp / 16.0;
	tempFahrenheit = tempCelsius * 1.8 + 32.0;
	return tempFahrenheit;
}

// ------------------------------------- MAIN -------------------------------------

void setup() {
  Serial.begin(9600);

	lcd.begin(16, 2);
	lcd.createChar(0, degreeChar);
	lcd.setCursor(0, 0);

	lcd.print("arduino-thermal");
	lcd.setCursor(0, 1);
	lcd.print(VERSION);
	delay(2000);
	lcd.clear();
}


void loop() {
	byte present = 0;
	byte type_s;
	byte data[12];
	byte addr[8];
	float celsius, fahrenheit;

	if (!ds.search(addr)) {
		Serial.println("> temp: No more addresses.");
		Serial.println();
		ds.reset_search();
		delay(250);
		return ;
	}

	Serial.print("> temp: ROM =");
	for(byte i = 0; i < 8; i++) {
		Serial.write(' ');
		Serial.print(addr[i], HEX);
	}

	if (OneWire::crc8(addr, 7) != addr[7]) {
			Serial.println("> temp: CRC is not valid!");
			return ;
	}
	Serial.println();

	switch (addr[0]) {
		case 0x10:
			Serial.println("> temp: Chip = DS18S20");
				type_s = 1;
				break;
		case 0x28:
			Serial.println("> temp: Chip = DS18B20");
			type_s = 0;
			break;
		case 0x22:
			Serial.println("> temp: Chip = DS1822");
			type_s = 0;
			break;
		default:
			Serial.println("> temp: Device is not a DS18x20 family device.");
			return ;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);

	delay(1000);

	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);

	Serial.print("> temp: Data = ");
	Serial.print(present, HEX);
	Serial.print(" ");

	for (byte i = 0; i < 9; i++) {
		data[i] = ds.read();
		Serial.print(data[i], HEX);
		Serial.print(" ");
	}
	Serial.print("> temp: CRC=");
	Serial.print(OneWire::crc8(data, 8), HEX);
	Serial.println();

	int16_t raw = (data[1] << 8) | data[0];
	if (type_s) {
		raw = raw << 3;
		if (data[7] == 0x10) {

			raw = (raw & 0xFFF0) + 12 - data[6];
		}
	} else {
		byte cfg = (data[4] & 0x60);

		if (cfg == 0x00) raw = raw & ~7;
		else if (cfg == 0x20) raw = raw & ~3;
		else if (cfg == 0x40) raw = raw & ~1;
	}

	celsius = tempConvertCelsius(static_cast<float>(raw));
	fahrenheit = tempConvertFahrenheit(static_cast<float>(raw));

	Serial.print("> temp: Temperature = ");
	Serial.print(celsius);
	degreeSerialSymbol();
	Serial.print("C, ");
	Serial.print(fahrenheit);
	degreeSerialSymbol();
	Serial.println("F");

	lcd.setCursor(0, 0);
	lcd.print("Temp: ");
	lcd.print(celsius);
	lcd.write(byte(0));
	lcd.print("C");

	lcd.setCursor(0, 1);
	lcd.print("Temp: ");
	lcd.print(fahrenheit);
	lcd.write(byte(0));
	lcd.print("F");
}