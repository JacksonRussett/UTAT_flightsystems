/*
 Name:		leanSD.cpp
 Created:	11/3/2019 5:12:36 PM
 Author:	vedan
 Editor:	http://www.visualmicro.com
*/

#include "leanSD.h"


leanSDCard::leanSDCard(int SSPin) {
	_SSPin = SSPin;
}

void leanSDCard::sendCommand(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4, uint8_t crc) {
	// Send command
	SPI.transfer(cmd);
	SPI.transfer(arg1);
	SPI.transfer(arg2);
	SPI.transfer(arg3);
	SPI.transfer(arg4);
	SPI.transfer(crc);
	// Response wait cycle
	SPI.transfer(0xFF);
}

void leanSDCard::wait() {
	for (int i = 0; i < 5; i++) {
		SPI.transfer(0xFF);
	}
}

int leanSDCard::init() {
	// Set the Slave Select pin
	pinMode(_SSPin, OUTPUT);
	// Slow down clock for initialization
	SPI.setClockDivider(SPI_CLOCK_DIV128);

	// Send 80 bits to get tell the SD card the rate of transfer
	for (int i = 0; i < 10; i++) {
		SPI.transfer(0xFF);
	}

	// CMD0
	sendCommand(0x40, 0x00, 0x00, 0x00, 0x00, 0x95);
	// R1 response
	if (SPI.transfer(0xFF) != 0x01) {
		Serial.println("CMD0 failed");
		return -1;
	};
	wait();

	// CMD8
	sendCommand(0x48, 0x00, 0x00, 0x01, 0xAA, 0x87);
	// R7 response
	SPI.transfer(0xFF);
	SPI.transfer(0xFF);
	SPI.transfer(0xFF);
	byte voltage = SPI.transfer(0xFF);
	byte echo = SPI.transfer(0xFF);
	if (voltage != 0x01 || echo != 0xAA) {
		Serial.println("CMD8 failed");
		return -1;
	}

	// CMD55 and ACMD41 repeat
	unsigned long start = millis();
	while (millis() - start < 5000) {
		// CMD55
		sendCommand(0x77, 0x00, 0x00, 0x00, 0x00, 0x65);
		wait();

		// ACMD41
		sendCommand(0x69, 0x40, 0x00, 0x00, 0x00, 0x77);
		if (SPI.transfer(0xFF) == 0x00) {
			// Speed up clock for fast writes
			SPI.setClockDivider(SPI_CLOCK_DIV16);
			wait();

			return 0;
		}
		wait();
	}
	
	Serial.println("CMD55/ACMD41 failed");
	return -1;
}

int leanSDCard::writeSector(uint32_t blockAddress, byte* data) {
	// CMD24 Single block write
	sendCommand(0x58, 
		(blockAddress & 0xFF000000) >> 24,
		(blockAddress & 0x00FF0000) >> 16,
		(blockAddress & 0x0000FF00) >> 8,
		(blockAddress & 0x000000FF) >> 0,
	0x95);
	// R1 response
	SPI.transfer(0xFF);
	
	// Send start token
	SPI.transfer(0xFE);
	// Send data
	for (int i = 0; i < 512; i++) {
		SPI.transfer(data[i]);
	}
	// Send dummy CRC
	SPI.transfer(0x95);
	SPI.transfer(0x95);
	
	// Response token
	byte response = ((SPI.transfer(0xFF)) & 0x0E) >> 1;
	if (response != 2) {
		Serial.println("Write error.");
		return -1;
	}

	return 0;
}