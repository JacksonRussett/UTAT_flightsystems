/*
 Name:		MS5611.cpp
 Created:	11/3/2019 4:03:36 PM
 Author:	vedan
 Editor:	http://www.visualmicro.com
*/

#include "MS5611.h"
#include <Wire.h>


MS5611::MS5611(int address) {
	_address = address;
}

int MS5611::init() {
	// Reset
	Wire.beginTransmission(_address);
	Wire.write(0x1E);
	if (Wire.endTransmission() != 0) {
		Serial.println("MS5611 reset failed");
		return -1;
	}
	delay(5);


	// Read PROM data
	for (int i = 0; i < 6; i++) {
		// Send address of PROM to read
		Wire.beginTransmission(_address);
		Wire.write(0xA0 | (i + 1) << 1 | 0x00);
		Wire.endTransmission();
		// Read received bytes from PROM
		if (Wire.requestFrom(_address, 2)) {
			uint16_t h1 = Wire.read();
			uint16_t h2 = Wire.read();
			uint16_t c = h1 << 8 | h2;
			_calibrationData[i + 1] = c;
		}
		else {
			Serial.println("PROM read failed");
			return -1;
		}
	}

	return 0;
}

int MS5611::getTandP(float* temp, float* pressure) {
	uint32_t D1, D2;

	// Read D1 OSR=256
	Wire.beginTransmission(_address);
	Wire.write(0x40);
	Wire.endTransmission();
	delay(1);
	// Query ADC to get D1 back
	Wire.beginTransmission(_address);
	Wire.write(0x00);
	Wire.endTransmission();
	// Convert the 24-bit value from the ADC to a single integer
	if (Wire.requestFrom(_address, 3)) {
		byte high = Wire.read();
		byte mid = Wire.read();
		byte low = Wire.read();
		D1 = high << 16 | mid << 8 | low;
	}
	else {
		Serial.println("D1 read failed");
		return -1;
	}

	// Read D2 OSR=256
	Wire.beginTransmission(_address);
	Wire.write(0x50);
	Wire.endTransmission();
	delay(1);
	// Query ADC to get D2 back
	Wire.beginTransmission(_address);
	Wire.write(0x00);
	Wire.endTransmission();
	// Convert the 24-bit value from the ADC to a single integer
	if (Wire.requestFrom(_address, 3)) {
		byte high = Wire.read();
		byte mid = Wire.read();
		byte low = Wire.read();
		D2 = high << 16 | mid << 8 | low;
	}
	else {
		Serial.println("D2 read failed");
		return -1;
	}


	// Calculate temperature
	int32_t dT = D2 - _calibrationData[5] * pow(2, 8);
	int32_t TEMP = 2000 + dT * _calibrationData[6] / pow(2, 23);
	*temp = TEMP / 100.0;

	// Calculate pressure
	int64_t OFF = _calibrationData[2] * pow(2, 16) + (_calibrationData[4] * dT) / pow(2, 7);
	int64_t SENS = _calibrationData[1] * pow(2, 15) + (_calibrationData[3] * dT) / pow(2, 8);
	int32_t P = (D1 * SENS / pow(2, 21) - OFF) / pow(2, 15);
	*pressure = P / 100.0;

	return 0;
}