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

void setup() {
	Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.createChar(0, degreeChar);
    lcd.setCursor(0, 0);
}

void loop() {
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float celsius, fahrenheit;

    if ( !ds.search(addr)) {
        ds.reset_search();
        delay(250);
        return;
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return;
    }
	Serial.println();

  // the first ROM byte indicates which chip
	switch (addr[0]) {
	case 0x10:
		type_s = 1;
		break;
	case 0x28:
		type_s = 0;
		break;
	case 0x22:
		type_s = 0;
		break;
	default:
		Serial.println("Device is not a DS18x20 family device.");
		return;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);

	delay(1000);

	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);

	for ( i = 0; i < 9; i++) {
		data[i] = ds.read();
	}


	int16_t raw = (data[1] << 8) | data[0];
	if (type_s) {
		raw = raw << 3; // 9 bit resolution default
		if (data[7] == 0x10) {
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
		}
	} else {
		byte cfg = (data[4] & 0x60);
		if (cfg == 0x00) raw = raw & ~7;	// 9 bit resolution, 93.75 ms
		else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
		else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
	}

	celsius = (float)raw / 16.0;
	fahrenheit = celsius * 1.8 + 32.0;

	Serial.print("> Temperature = ");
	Serial.print(celsius);
	Serial.print("°C, ");
	Serial.print(fahrenheit);
	Serial.print("°F");

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