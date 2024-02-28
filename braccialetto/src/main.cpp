#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <EloquentTinyML.h>
#include "Neural.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define NUMBER_OF_INPUTS 5
#define NUMBER_OF_OUTPUTS 1
#define TENSOR_ARENA_SIZE 2*1024
Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS,NUMBER_OF_OUTPUTS,TENSOR_ARENA_SIZE> ml;



#define SERVICE_UUID_FOR_SEX_AND_AGE "4fafc201-1fb5-459e-8fcc-c5c9c331914j"
#define SEX_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a1"
#define AGE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a2"
#define DEAD_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a5"
#define RUN_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a6"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"
#define COLESTEROL_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a3"
#define SUGAR_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a4"

BLECharacteristic *pTemperatureCharacteristic;
BLECharacteristic *pHeartBeatCharacteristic;
BLECharacteristic *pSaturationCharacteristic;
BLECharacteristic *pSexCharacteristic;
BLECharacteristic *pAgeCharacteristic;
BLECharacteristic *pColesterolCharacteristic;
BLECharacteristic *pSugarCharacteristic;
BLECharacteristic *pDeadCharacteristic;
BLECharacteristic *pRunCharacteristic;

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
  pinMode(LED_BUILTIN, OUTPUT); 

  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  delay(2000);         // wait two seconds for initializing
  oled.clearDisplay(); // clear display

  ml.begin(Neural);
  
  Serial.println("Starting BLE work!");
  BLEDevice::init("Braccialetto");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLEService *pServiceForSexAndAge = pServer->createService(SERVICE_UUID_FOR_SEX_AND_AGE);
  pTemperatureCharacteristic = pService->createCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pHeartBeatCharacteristic = pService->createCharacteristic(HEARTBEAT_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pSaturationCharacteristic = pService->createCharacteristic(SATURATION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pSexCharacteristic = pServiceForSexAndAge->createCharacteristic(SEX_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pAgeCharacteristic = pServiceForSexAndAge->createCharacteristic(AGE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pDeadCharacteristic = pServiceForSexAndAge->createCharacteristic(DEAD_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pRunCharacteristic = pServiceForSexAndAge->createCharacteristic(RUN_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pColesterolCharacteristic = pService->createCharacteristic(COLESTEROL_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pSugarCharacteristic = pService->createCharacteristic(SUGAR_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pTemperatureCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pHeartBeatCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pSaturationCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pSexCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pAgeCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pColesterolCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pSugarCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pDeadCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pRunCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  //AGGIUNGO I DESCRIPTOR
  BLEDescriptor *pTemperatureDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pTemperatureDescriptor->setValue("Temperature");
  pTemperatureCharacteristic->addDescriptor(pTemperatureDescriptor);
  BLEDescriptor *pHeartBeatDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pHeartBeatDescriptor->setValue("HeartBeat");
  pHeartBeatCharacteristic->addDescriptor(pHeartBeatDescriptor);
  BLEDescriptor *pSaturationDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pSaturationDescriptor->setValue("Saturation");
  pSaturationCharacteristic->addDescriptor(pSaturationDescriptor);
  BLEDescriptor *pSexDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pSexDescriptor->setValue("Sex");
  pSexCharacteristic->addDescriptor(pSexDescriptor);
  BLEDescriptor *pAgeDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pAgeDescriptor->setValue("Age");
  pAgeCharacteristic->addDescriptor(pAgeDescriptor);
  BLEDescriptor *pColesterolDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pColesterolDescriptor->setValue("Colesterol");
  pColesterolCharacteristic->addDescriptor(pColesterolDescriptor);
  BLEDescriptor *pSugarDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pSugarDescriptor->setValue("Sugar");
  pSugarCharacteristic->addDescriptor(pSugarDescriptor);
  BLEDescriptor *pDeadDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pDeadDescriptor->setValue("Heart Disease Risk");
  pDeadCharacteristic->addDescriptor(pDeadDescriptor);
  BLEDescriptor *pRunDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  pRunDescriptor->setValue("Run");
  pRunCharacteristic->addDescriptor(pRunDescriptor);
  pTemperatureCharacteristic->setValue("1");
  pHeartBeatCharacteristic->setValue("0");
  pSaturationCharacteristic->setValue("3");
  pSexCharacteristic->setValue("1");
  pAgeCharacteristic->setValue("20");
  pColesterolCharacteristic->setValue("0");
  pSugarCharacteristic->setValue("50");
  pDeadCharacteristic->setValue("0");
  pRunCharacteristic->setValue("0");
  pService->start();
  pServiceForSexAndAge->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined!");

}

void loop() {
  //crea valori casuali per temperatura
  int temp = random(35, 42);
  pTemperatureCharacteristic->setValue(String(temp).c_str());
  pTemperatureCharacteristic->notify();

  //crea valori casuali per heartbeat
  int hb = random(40, 202);
  pHeartBeatCharacteristic->setValue(String(hb).c_str());
  pHeartBeatCharacteristic->notify();

  //crea valori casuali per saturazione
  int sat = random(90, 100);
  pSaturationCharacteristic->setValue(String(sat).c_str());
  pSaturationCharacteristic->notify();

  //crea valori casuali per colesterolo
  int col = random(0, 600);
  pColesterolCharacteristic->setValue(String(col).c_str());
  pColesterolCharacteristic->notify();

  //crea valori casuali per zucchero
  int sug = random(50, 200);
  pSugarCharacteristic->setValue(String(sug).c_str());
  pSugarCharacteristic->notify();
  int fastingBS = 0;
  if (sug > 120) {
    fastingBS = 1;
  }

  float inputs[5] = {stof(pAgeCharacteristic->getValue()), stof(pSexCharacteristic->getValue()),float(col),float(fastingBS),float(hb)};
  Serial.println("Valori inseriti: " + String(inputs[0]) + " " + String(inputs[1]) + " " + String(inputs[2]) + " " + String(inputs[3]) + " " + String(inputs[4]));
  float result = ml.predict(inputs);
  Serial.println("Result: " + String(result));
  if (result >= 0.9) {
    pDeadCharacteristic->setValue("1"); //rischio di heart disease
  } else {
    pDeadCharacteristic->setValue("0");
  }
  pDeadCharacteristic->notify();

  if(pRunCharacteristic->getValue() != "0"){
    Serial.println(String(pRunCharacteristic->getValue().c_str()));
    oled.clearDisplay();
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(32, 30);       // set position to display
    oled.println(String(pRunCharacteristic->getValue().c_str()));
    oled.display();
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if(pRunCharacteristic->getValue() == "0"){
    oled.clearDisplay();
    oled.print("");
    oled.display();
    digitalWrite(LED_BUILTIN, LOW);
  }
  delay(2000);
}
