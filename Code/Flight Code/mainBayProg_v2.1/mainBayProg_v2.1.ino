#include <SPI.h>
#include <Wire.h>

void setup() {
  //Initialize serial connections

  
}

void loop() {
  // Main loop to collect/transmit telemetry data

  //Collect the data

  //Store data

  //Send Data

}

void getGPSCoords() {
  //Parse serial data from the GPS module
  
}

void readPressure() {
  //Read registers in the MS5611 module
  
}

void readIMU() {
  //Read registers in the MPU-9250 module
  
}

void readFlightComps() {
  //Parse serial data from the two stratologger flight computers
  
}

void writeToSD(data packet) {
  //Record telemetry packets to the microSD card
  
}

void sendToRadio(data packet) {
  //Send the telemetry packets as serial data to the radio
  
}
