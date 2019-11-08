/*
 Name:		leanSD.h
 Created:	11/3/2019 5:12:36 PM
 Author:	vedan
 Editor:	http://www.visualmicro.com
*/

#ifndef _leanSD_h
#define _leanSD_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SPI.h>

class leanSDCard {
public:
	leanSDCard(int SSPin);
	int init();
	int writeSector(uint32_t blockAddress, byte* data);


private:
	int _SSPin;

	void wait();
	void sendCommand(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4, uint8_t crc);
};

#endif