#include <SPI.h>
#include <SD.h>
#include <Audio.h>
#include <MPU9250.h>
#include <Wire.h>
#include <MS5611.h>


// MPU9250 Multisensor, I2C address 0x68
MPU9250 myMPU9250(Wire, 0x68);
// MS5611 barometer, I2C address 0x77 by pulling CSB low
MS5611 myMS5611(0x77);
// MAX4466 Microphone analog input pin, file and buffer
int myMAX4466pin = 14;
// Sparkfun level-shifting microSD breakout pins
int cardDetectPin = 9;
int chipSelectPin = 10;
int flushFreq = 50;
// LM35DZ analog input pin
int myLM35DZpin = 15;
// Data file
File dataFile;


void setup() {
	Serial.begin(57600);
	SPI.begin();
	Wire.begin();
	
	// Init sensor, with 1 second delay in case of fail
	// 1. MPU9250
	while (myMPU9250.begin() < 0) {
		Serial.println("MPU9250 init failed, retrying...");
		delay(1000); 
	}
	Serial.println("MPU9250 ready");
	// 2. MS5611
	while (myMS5611.init() != 0) {
		Serial.println("MS5611 init failed, retrying...");
		delay(1000);
	}
	Serial.println("MS5611 ready");
	// 3. MAX4466
	pinMode(myMAX4466pin, INPUT);
	// 4. SD card	
	pinMode(cardDetectPin, INPUT);
	while (!digitalRead(cardDetectPin)) {
		Serial.println("SD card not detected. Please insert card.");
		delay(1000);
	}
	while (!SD.begin(chipSelectPin)) { 
		Serial.println("SD card init failed... retrying...");
		delay(1000); }
	Serial.println("SD card ready.");
	// 5. LM35DZ
	pinMode(myLM35DZpin, INPUT);


	// Speed up SPI clock to make SD card writes faster
	SPI.setClockDivider(SPI_CLOCK_DIV4);

	// Open data file
	dataFile = SD.open("DATAFILE.TXT", FILE_WRITE);
	if (!dataFile) {
		Serial.println("File creation failed.");
		return;
	}
	Serial.println("File ready.");

	// Header of csv file
	dataFile.println("Time,9250_x,9250_y,9250_z,5611_t,5611_p,LM35_t,MAX_aud");
}

void loop() {
	for (int i = 0; i < flushFreq; i++) {
		// Collect data from sensors
		float time = millis();
		myMPU9250.readSensor();
		float MPU9250_accelX = myMPU9250.getAccelX_mss();
		float MPU9250_accelY = myMPU9250.getAccelY_mss();
		float MPU9250_accelZ = myMPU9250.getAccelZ_mss();
		float MS5611_temperature, MS5611_pressure;
		myMS5611.getTandP(&MS5611_temperature, &MS5611_pressure);
		int MAX4466_audio = analogRead(myMAX4466pin);
		float LM35DZ_temperature = ((analogRead(myLM35DZpin) / 1024.0) * 5000.0) / 10.0;

		// Concatenate sensor values together to form string
		/*String data = String(time) + String(",");
		data = String(MPU9250_accelX) + String(",") + String(MPU9250_accelY) + String(",") + String(MPU9250_accelZ) + String(",");
		data += String(MS5611_temperature) + String(",") + String(MS5611_pressure) + String(",");
		data += String(LM35DZ_temperature) + String(",") + String(MAX4466_audio) + String("\n");*/

		// Write data to file (Probably a faster way to do this)
		dataFile.print(time); dataFile.print(",");
		dataFile.print(MPU9250_accelX); dataFile.print(",");
		dataFile.print(MPU9250_accelY); dataFile.print(",");
		dataFile.print(MPU9250_accelZ); dataFile.print(",");
		dataFile.print(MS5611_temperature); dataFile.print(",");
		dataFile.print(MS5611_pressure); dataFile.print(",");
		dataFile.print(LM35DZ_temperature); dataFile.print(",");
		dataFile.println(MAX4466_audio);
	}
	// Flush SD buffer to file.
	dataFile.flush();
}