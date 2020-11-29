//	Used pins:

//	PWM 2 = LCD Data
//	PWM 3 = LCD Data
//	PWM 4 = LCD Data
//	PWM 5 = LCD Data
//	PWM 7 = Temperature sensor (1Wire bus)
//	PWM 8 = LCD RS
//	PWM 9 = LCD E
//	PWM 10 = Ethernet shield
//	PWM 11 = Ethernet shield
//	PWM 12 = Ethernet shield
//	PWM 13 = Ethernet shield

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(172, 31, 227, 18);
EthernetServer server(80);

const int rs = 9, en = 8, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
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

OneWire ds(7);

//---------------------------------------- SETUP ----------------------------------------
void setup() {
	Serial.begin(9600);
	lcd.begin(16, 2);
	lcd.createChar(0, degreeChar);
	lcd.setCursor(0, 0);

	while (!Serial) {}// wait for serial port to connect. Needed for native USB port only

	// start the Ethernet connection and the server:
	Ethernet.begin(mac, ip);

	// Check for Ethernet hardware present
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		Serial.println("> eth: Ethernet shield was not found. Sorry, can't run without hardware. :(");
		while (true) {
			delay(1); // do nothing, no point running without Ethernet hardware
		}
		}
		if (Ethernet.linkStatus() == LinkOFF) {
		Serial.println("> eth: Ethernet cable is not connected.");
	}

	// start the server
	server.begin();
	Serial.print("> eth: server is at ");
	Serial.println(Ethernet.localIP());
}

//---------------------------------------- LOOP ----------------------------------------
void loop() {
	float celsius, fahrenheit;
	int16_t rawTemp = tempSensor();

	celsius = tempConvertCelsius((float)rawTemp);
	fahrenheit = tempConvertFahrenheit((float)rawTemp);

	EthernetClient client = server.available();
	if (client) {
		Serial.println("> eth: new client");
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) {
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close");	// the connection will be closed after completion of the response
					client.println("Refresh: 5");	// refresh the page automatically every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");

					client.print("Temperature: ");
					client.print(celsius);
					client.println("&#x2103; <br>");

					client.print("Temperature: ");
					client.print(fahrenheit);
					client.println("&#x2109;");

					client.println("</html>");
					break;
				}
				if (c == '\n') {
					// you're starting a new line
					currentLineIsBlank = true;
				} else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
		client.stop();
		Serial.println("> eth: client disconnected");
	}
}

int16_t tempSensor() {
	byte i;
	byte present = 0;
	byte type_s;
	byte data[12];
	byte addr[8];
	float celsius, fahrenheit;

	if ( !ds.search(addr)) {
		Serial.println("> temp: No more addresses.");
		Serial.println();
		ds.reset_search();
		delay(250);
		return 0;
	}

	Serial.print("> temp: ROM =");
	for( i = 0; i < 8; i++) {
		Serial.write(' ');
		Serial.print(addr[i], HEX);
	}

	if (OneWire::crc8(addr, 7) != addr[7]) {
			Serial.println("> temp: CRC is not valid!");
			return 0;
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
			return 0;
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

	for ( i = 0; i < 9; i++) {
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

	celsius = tempConvertCelsius((float)raw);
	fahrenheit = tempConvertFahrenheit((float)raw);

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

	return raw;
}

void degreeSerialSymbol() {
	if (Serial)
	{
		Serial.write(0xC2);
		Serial.write(0xB0);
	}
}

float tempConvertCelsius(int16_t rawTemp) {
	float tempCelsius = (float)rawTemp / 16.0;
	return tempCelsius;
}

float tempConvertFahrenheit(int16_t rawTemp) {
	float tempCelsius, tempFahrenheit;

	tempCelsius = (float)rawTemp / 16.0;
	tempFahrenheit = tempCelsius * 1.8 + 32.0;
	return tempFahrenheit;
}