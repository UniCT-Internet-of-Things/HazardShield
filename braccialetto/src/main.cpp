#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"

BLECharacteristic *pTemperatureCharacteristic;
BLECharacteristic *pHeartBeatCharacteristic;
BLECharacteristic *pSaturationCharacteristic;


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer) {
    BLEDevice::startAdvertising();
    Serial.println("Device disconnected");
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Hai scritto: ");
    Serial.println(value.c_str());

  }
    
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  BLEDevice::init("Braccialetto");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTemperatureCharacteristic = pService->createCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pHeartBeatCharacteristic = pService->createCharacteristic(HEARTBEAT_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pSaturationCharacteristic = pService->createCharacteristic(SATURATION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pTemperatureCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pHeartBeatCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pSaturationCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pTemperatureCharacteristic->setValue("1");
  pHeartBeatCharacteristic->setValue("0");
  pSaturationCharacteristic->setValue("3");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined!");

}

void loop() {
  //crea valori casuali per temperatura
  int temp = random(35, 42);
  pTemperatureCharacteristic->setValue(String(temp).c_str());
  pTemperatureCharacteristic->notify();

  //crea valori casuali per heartbeat
  int hb = random(40, 200);
  pHeartBeatCharacteristic->setValue(String(hb).c_str());
  pHeartBeatCharacteristic->notify();

  //crea valori casuali per saturazione
  int sat = random(90, 100);
  pSaturationCharacteristic->setValue(String(sat).c_str());
  pSaturationCharacteristic->notify();

  delay(2000);
}