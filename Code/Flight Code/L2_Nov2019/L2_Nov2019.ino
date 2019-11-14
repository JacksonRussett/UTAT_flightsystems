// My libraries, source files are in their respective projects
#include <MS5611.h>
#include <leanSD.h>
// Other libraries
#include <MPU9250.h>
#include <TinyGPS.h>
#include <Wire.h>
#include <Adafruit_INA219.h>


//		Initialize all sensors
// Init MS5611 with I2C address
MS5611 myMS5611(0x77);
// Init SD card with SS pin
leanSDCard mySDCard(2);
// Init MPU9250 with I2C address
MPU9250 myMPU9250(Wire, 0x68);
// Init the NEO-6M GPS module
#define GPSSERIAL Serial1
TinyGPS myNEO6M;
// Init the INA219
Adafruit_INA219 myINA219;
// Radio
#define RADIOSERIAL Serial2


// Hypsometric formula: A formula for calculating altitude from pressure and temperature
// Replace this with whatever is appropriate, if you so wish
float getAltitude(float T, float P0) {
	return ((pow(1013.25 / P0, 0.19) - 1) * (T + 273.15)) / 0.0065;
}


//		Data collection stuff
// Data buffer
float data[128];
// Start block address
uint32_t blockAddress;


void setup() {
	// Serial for communicating with the host
	Serial.begin(115200);
	// I2C for MPU9250 and MS5611
	Wire.begin();
	// SPI for SD card
	SPI.begin();
	// Hardware serial lines for NEO-6M
	GPSSERIAL.begin(9600);
	// Serial for using the radio
	RADIOSERIAL.begin(9600);


	Serial.println("Starting sensors...");
	while (mySDCard.init() != 0) { delay(1000); }
	blockAddress = 8193; // Replace this 
	Serial.println("	SD card ready");
	while (myMS5611.init() != 0) { delay(1000); }
	Serial.println("	MS5611 ready");
	while (myMPU9250.begin() < 0) { delay(1000); }
	Serial.println("	MPU9250 ready");
	myINA219.begin(); // See if there's some way to catch errors here
	Serial.println("	INA219 ready");
}


void loop() {
	float lat = nanf("0.0");
	float lon = nanf("0.0");
	while (GPSSERIAL.available()) {
		if (myNEO6M.encode(GPSSERIAL.read())) {
			myNEO6M.f_get_position(&lat, &lon);
			// Send to radio
			char buff[10];
			RADIOSERIAL.write("\n");
			dtostrf(lat, 4, 6, buff);
			RADIOSERIAL.write(buff, 10);
			RADIOSERIAL.write("\t");
			dtostrf(lon, 4, 6, buff);
			RADIOSERIAL.write(buff, 10);
			RADIOSERIAL.write("\n");
		}
	}

	unsigned long t1 = micros();
	for (int i = 0; i < 126; i += 14) {
		// Get time from microcontroller
		unsigned long time = micros();
		data[i] = time;

		// Get pressure, temperature and altitude
		float T, P0;
		myMS5611.getTandP(&T, &P0);
		float h = getAltitude(T, P0);
		data[i + 1] = P0;
		data[i + 2] = T;
		data[i + 3] = h;

		// Get current and voltage
		// Voltage from bus and shunt calculated according to INA219 library formula
		float current = myINA219.getCurrent_mA();
		float voltage = myINA219.getBusVoltage_V() + (myINA219.getShuntVoltage_mV() / 1000);
		data[i + 4] = current;
		data[i + 5] = voltage;

		// Read MPU9250
		myMPU9250.readSensor();
		// Get acceleration
		float accelX = myMPU9250.getAccelX_mss();
		float accelY = myMPU9250.getAccelY_mss();
		float accelZ = myMPU9250.getAccelZ_mss();
		data[i + 6] = accelX;
		data[i + 7] = accelY;
		data[i + 8] = accelZ;
		// Get gyroscope
		float gyroX = myMPU9250.getGyroX_rads();
		float gyroY = myMPU9250.getGyroY_rads();
		float gyroZ = myMPU9250.getGyroZ_rads();
		data[i + 9] = gyroX;
		data[i + 10] = gyroY;
		data[i + 11] = gyroZ;
		
		// Get GPS coordinates: NaN is stored any time a sample is taken when the GPS receiver is still receiving a sentence
		data[i + 12] = lat;
		data[i + 13] = lon;
	}
	// Store the time taken to read 10 readings, for diagnostics
	data[126] = micros() - t1;
	// End of sector delimiter
	data[127] = 0xFFFFFFFF;

	// Write the sector
	mySDCard.writeSector(blockAddress, (byte*)data);
	// Increment the block address
	blockAddress++;

	// Write "." during downtime
	RADIOSERIAL.write(".");
}