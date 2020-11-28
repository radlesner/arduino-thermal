#include <LiquidCrystal.h>
#include <OneWire.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
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

OneWire  ds(10);

//----------------------------------------
void setup() {
	Serial.begin(9600);
	lcd.begin(16, 2);
	lcd.createChar(0, degreeChar);
	lcd.setCursor(0, 0);
}

void loop() {
	int16_t rawTemp = tempSensor();
}
//----------------------------------------

int16_t tempSensor() {
	byte i;
	byte present = 0;
	byte type_s;
	byte data[12];
	byte addr[8];
	float celsius, fahrenheit;

	if ( !ds.search(addr)) {
		Serial.println("No more addresses.");
		Serial.println();
		ds.reset_search();
		delay(250);
		return 0;
  }

	Serial.print("ROM =");
	for( i = 0; i < 8; i++) {
		Serial.write(' ');
		Serial.print(addr[i], HEX);
	}

	if (OneWire::crc8(addr, 7) != addr[7]) {
			Serial.println("CRC is not valid!");
			return 0;
	}
	Serial.println();

	switch (addr[0]) {
		case 0x10:
			Serial.println("  Chip = DS18S20");
      		type_s = 1;
      		break;
		case 0x28:
			Serial.println("  Chip = DS18B20");
			type_s = 0;
			break;
		case 0x22:
			Serial.println("  Chip = DS1822");
			type_s = 0;
			break;
		default:
			Serial.println("Device is not a DS18x20 family device.");
			return 0;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);

	delay(1000);

	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);

	Serial.print("  Data = ");
	Serial.print(present, HEX);
	Serial.print(" ");

	for ( i = 0; i < 9; i++) {
		data[i] = ds.read();
		Serial.print(data[i], HEX);
		Serial.print(" ");
	}
	Serial.print(" CRC=");
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

	celsius = (float)raw / 16.0;
	fahrenheit = celsius * 1.8 + 32.0;

	Serial.print("  Temperature = ");
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

	return raw;
}

void degreeSerialSymbol() {
	if (Serial)
	{
		Serial.write(0xC2);
		Serial.write(0xB0);
	}
}