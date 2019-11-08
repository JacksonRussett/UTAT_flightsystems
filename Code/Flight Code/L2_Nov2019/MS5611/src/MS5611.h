/*
 Name:		MS5611.h
 Created:	11/3/2019 4:03:36 PM
 Author:	vedan
 Editor:	http://www.visualmicro.com
*/

#ifndef _MS5611_h
#define _MS5611_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>

class MS5611 {
public:
	MS5611(int address);
	int init();
	int getTandP(float* temp, float* pressure);
private:
	int _address;
	uint16_t _calibrationData[7];
};

#endif