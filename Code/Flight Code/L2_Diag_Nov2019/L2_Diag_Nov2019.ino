#include <MS5611.h>
#include <MPU9250.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <TinyGPS++.h>
#include <SD.h>


//		Initialize all sensors
// Init MS5611 with I2C address
MS5611 myMS5611(0x77);
// Init MPU9250 with I2C address
MPU9250 myMPU9250(Wire, 0x68);
// Init the NEO-6M GPS module
#define GPSSERIAL Serial1
TinyGPSPlus myNEO6M;
// Init the INA219
Adafruit_INA219 myINA219;

// SD card stuff
Sd2Card card;
SdVolume volume;
SdFile root;
const int checkSD = 2;
const int chipSelect = 3;


// Hypsometric formula: A formula for calculating altitude from pressure and temperature
// Replace this with whatever is appropriate, if you so wish
float getAltitude(float T, float P0) {
	return ((pow(1013.25 / P0, 0.19) - 1) * (T + 273.15)) / 0.0065;
}

void setup() {
	// Serial for communicating with the host
	Serial2.begin(9600);
	// I2C for MPU9250, MS5611, and INA219
	Wire.begin();
	// SPI for SD card
	SPI.begin();
	// Hardware serial lines for NEO-6M
	GPSSERIAL.begin(9600);

  delay(2500);

  //Check SD card
  Serial2.println("Checking SD Card...");
  if(digitalRead(checkSD)) {
    Serial2.print("\nInitializing SD card...");
    // we'll use the initialization code from the utility libraries
    // since we're just testing if the card is working!
    if (!card.init(SPI_HALF_SPEED, chipSelect)) {
      Serial2.println("initialization failed. Things to check:");
      Serial2.println("* is a card inserted?");
      Serial2.println("* is your wiring correct?");
      Serial2.println("* did you change the chipSelect pin to match your shield or module?");
      return;
    } else {
     Serial2.println("Wiring is correct and a card is present.");
    }
  } else {
    Serial2.println("No SD card detected");
  }


	Serial2.println("\nStarting sensors...");
	while (myMS5611.init() != 0) { delay(1000); }
	Serial2.println(" MS5611 ready");
	while (myMPU9250.begin() < 0) { delay(1000); }
	Serial2.println(" MPU9250 ready");
	myINA219.begin(); // See if there's some way to catch errors here
	Serial2.println(" INA219 ready\n");

  
  smartDelay(2000);
}


void loop() {
  // Get current and voltage
    // Voltage from bus and shunt calculated according to INA219 library formula
    float current = myINA219.getCurrent_mA()/1000;
    float voltage = myINA219.getBusVoltage_V() + (myINA219.getShuntVoltage_mV() / 1000);
    Serial2.println("Power Supply - Bat Volt: " +String(voltage) + " Current: " + String(current, 3) + " Power: " + String(current * voltage, 3));
    
		// Get pressure, temperature and altitude
		float T, P0;
		myMS5611.getTandP(&T, &P0);
		float h = getAltitude(T, P0);
		Serial2.println("Barometer - T: " +String(T) + " P: " + String(P0) + " Alt: " + String(h));

		// Read MPU9250
		myMPU9250.readSensor();
		// Get acceleration
		float accelX = myMPU9250.getAccelX_mss();
		float accelY = myMPU9250.getAccelY_mss();
		float accelZ = myMPU9250.getAccelZ_mss();
		// Get gyroscope
		float gyroX = myMPU9250.getGyroX_rads();
		float gyroY = myMPU9250.getGyroY_rads();
		float gyroZ = myMPU9250.getGyroZ_rads();
		Serial2.println("IMU - aX: " + String(accelX) + " aY: " + String(accelY) + " aZ: " + String(accelZ) + " gX: " + String(gyroX) + " gY: " + String(gyroY) + " gZ: " + String(gyroZ));

		// Get GPS coordinates: NaN is stored any time a sample is taken when the GPS receiver is still receiving a sentence
		float lat = nanf("0.0");
		float lng = nanf("0.0");
    if (millis() > 5000 && myNEO6M.charsProcessed() < 10) {
      Serial2.println(F("No GPS data received: check wiring"));
    } else {
			//lat = myNEO6M.location.lat();
      //lng = myNEO6M.location.lng();
      lat = myNEO6M.location.lat();
      lng = myNEO6M.location.lng();
		}
		Serial2.println("GPS Coords: " + String(lat, 10) + ", " + String(lng, 10) + "\n");

    smartDelay(3000);
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial1.available())
      myNEO6M.encode(Serial1.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial2.print('*');
    Serial2.print(' ');
  }
  else
  {
    Serial2.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial2.print(' ');
  }
  smartDelay(0);
}
