#include <Arduino.h>
#include <ArduinoBLE.h>


void setup() {
  Serial.begin(115200);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }
  BLE.setLocalName("braccialetto");
  BLE.setDeviceName("ciccio");

  Serial.println("Bluetooth® Low Energy Central scan");
  // start advertising
  BLE.advertise();
  // start scanning for peripheral
}

void loop(){

}
