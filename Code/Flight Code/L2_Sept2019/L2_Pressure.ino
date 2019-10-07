unsigned int loopCount = 0;


// ===========================================
// ===               GPS SETUP             ===
// ===========================================
#include <TinyGPS++.h>
TinyGPSPlus gps;

// ====================================================
// ===               BAROMETER SETUP                ===
// ====================================================
#include <Wire.h>

// Memory

#include <MS5611.h>
MS5611 ms5611;
double temperature;
double altitude;


// ========================================================
// ===               ACCELEROMETER SETUP                ===
// ========================================================
#include "MPU9250.h"
#define I2Cclock 400000
#define I2Cport Wire
#define MPU9250_ADDRESS 0x68
MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);

float ax, ay, az;
float gx, gy, gz;
float ux, uy, uz;

//
//  SETUP UNCTIONS
//

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial1.begin(9600); // gps
  Serial2.begin(9600); // radio
  Wire.begin();

  // Power Up Long Beep
  smartDelay(1);
  Serial.println(("POWER UP"));
  
  // Initialize Baro by reading calibration parameters
  if(ms5611.begin()){
     // Barometer Connected Beep once
    smartDelay(1);
    Serial.println(("BARO OK"));
  } else {
    // Baro startup fail, no beep
    Serial.println(("BARO AIL"));
  }

  // Read the WHO_AM_I register, this is a good test of communication
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print(("MPU9250 I AM 0x"));
  Serial.print(c, HEX);
  Serial.print((" I should be 0x"));
  Serial.println(0x71, HEX);

    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);    
    myIMU.initMPU9250();

    // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.factoryMagCalibration);
    // Initialize device for active mode read of magnetometer
    Serial.println("AK8963 initialized for active data mode....");


    // Get sensor resolutions, only need to do this once
    myIMU.getAres();
    myIMU.getGres();
    myIMU.getMres();
    
        // The next call delays for 4 seconds, and then records about 15 seconds of
    // data to calculate bias and scale.
//    myIMU.magCalMPU9250(myIMU.magBias, myIMU.magScale);
    Serial.println("AK8963 mag biases (mG)");
    Serial.println(myIMU.magBias[0]);
    Serial.println(myIMU.magBias[1]);
    Serial.println(myIMU.magBias[2]);

    Serial.println("AK8963 mag scale (mG)");
    Serial.println(myIMU.magScale[0]);
    Serial.println(myIMU.magScale[1]);
    Serial.println(myIMU.magScale[2]);

      Serial.println("Magnetometer:");
      Serial.print("X-Axis sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[0], 2);
      Serial.print("Y-Axis sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[1], 2);
      Serial.print("Z-Axis sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[2], 2);


    smartDelay(500);

    // Used to have offset settting code here, is dodgy and not documented, so nope
  
// ===============================================
// ===               GPS STARTUP               ===
// ===============================================
  
  if (gps.charsProcessed() > 10){
    // GPS is connectedm Beep 4 times
    Serial.println(("GPS OK"));
  } else {
    // GPS not working, no beep
    
    Serial.println(("GPS AIL"));
  }


  Serial.println(("READY"));
}

void loop() {
  if (loopCount > 200) loopCount = 0;
  loopCount++;
  
  // Always read GPS first
  smartDelay(0);
  
  // Read Altitude
  altitude =  ms5611.getAltitude(ms5611.readPressure());
  smartDelay(0);
  

    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes; // - myIMU.accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes; // - myIMU.accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes; // - myIMU.accelBias[2];

    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0] * myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1] * myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2] * myIMU.gRes;

    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes
               * myIMU.factoryMagCalibration[0] - myIMU.magBias[0];
    myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes
               * myIMU.factoryMagCalibration[1] - myIMU.magBias[1];
    myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes
               * myIMU.factoryMagCalibration[2] - myIMU.magBias[2];

    ax = myIMU.ax;  
    ay = myIMU.ay;
    az = myIMU.az;
    
    gx = myIMU.gx;  
    gy = myIMU.gy;
    gz = myIMU.gz;
    
    ux = myIMU.mx;  
    uy = myIMU.my;
    uz = myIMU.mz;


  smartDelay(0);
  
  // Read Temperature
  temperature = ms5611.readTemperature();
  smartDelay(0);

  smartDelay(500);

   
    Serial.print(millis());
    Serial.print(",");
    smartDelay(0);

    Serial.print(temperature);
    Serial.print(("C,"));
    smartDelay(0);
    
    Serial.print(altitude);
    Serial.print(("m,"));
    smartDelay(0);
    
    Serial.print(("A,"));
    Serial.print(ax);
    Serial.print(",");
    Serial.print(ay);
    Serial.print(",");
    Serial.print(az);
    Serial.print(",");
    smartDelay(0);

    Serial.print(("R,"));
    Serial.print(gx);
    Serial.print(",");
    Serial.print(gy);
    Serial.print(",");
    Serial.print(gz);
    Serial.print(",");
    smartDelay(0);

    Serial.print(("M,"));
    Serial.print(ux);
    Serial.print(",");
    Serial.print(uy);
    Serial.print(",");
    Serial.print(uz);
    Serial.print(",");
    smartDelay(0);
    

    // Log GPS
    // GPS Time
    Serial.print(("GT,"));
    Serial.print(gps.date.year());
    Serial.print("/");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.year());
    Serial.print(" ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.print(gps.time.second());
    Serial.print(",");
    smartDelay(0);


    // GPS Position
    Serial.print(("GL,"));
    if (gps.location.isValid()) 
    {
      Serial.print(("**,**,"));
    } else {
      Serial.print(gps.location.lat(), 6);
      Serial.print(",");
      Serial.print(gps.location.lng(), 6);
      Serial.print(",");
    }
    smartDelay(0);

    Serial.println();


    Serial2.print(millis());
    Serial2.print(",");
    smartDelay(0);

    Serial2.print(temperature);
    Serial2.print(("C,"));
    smartDelay(0);
    
    Serial2.print(altitude);
    Serial2.print(("m,"));
    smartDelay(0);
    
    Serial2.print(("A,"));
    Serial2.print(ax);
    Serial2.print(",");
    Serial2.print(ay);
    Serial2.print(",");
    Serial2.print(az);
    Serial2.print(",");
    smartDelay(0);

    Serial2.print(("R,"));
    Serial2.print(gx);
    Serial2.print(",");
    Serial2.print(gy);
    Serial2.print(",");
    Serial2.print(gz);
    Serial2.print(",");
    smartDelay(0);

    Serial2.print(("M,"));
    Serial2.print(ux);
    Serial2.print(",");
    Serial2.print(uy);
    Serial2.print(",");
    Serial2.print(uz);
    Serial2.print(",");
    smartDelay(0);
    

    // Log GPS
    // GPS Time
    Serial2.print(("GT,"));
    Serial2.print(gps.date.year());
    Serial2.print("/");
    Serial2.print(gps.date.month());
    Serial2.print("/");
    Serial2.print(gps.date.year());
    Serial2.print(" ");
    Serial2.print(gps.time.hour());
    Serial2.print(":");
    Serial2.print(gps.time.minute());
    Serial2.print(":");
    Serial2.print(gps.time.second());
    Serial2.print(",");
    smartDelay(0);


    // GPS Position
    Serial2.print(("GL,"));
    if (gps.location.isValid()) 
    {
      Serial2.print(("**,**,"));
    } else {
      Serial2.print(gps.location.lat(), 6);
      Serial2.print(",");
      Serial2.print(gps.location.lng(), 6);
      Serial2.print(",");
    }
    smartDelay(0);

    Serial2.println();
    
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}
