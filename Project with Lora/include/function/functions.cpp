#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <BLEDevice.h>
#include <SPI.h>     
#include <Arduino.h>

BLEClient* pClient;
BLEAdvertisedDevice myAncora;
bool AncoraFound = false;
BLEScan *pBLEScan;


class callbackgenerica: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
   //printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
};


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer) {
    //BLEDevice::startAdvertising();
    Serial.println("Device disconnected");
  }
};

// Funzione per convertire una stringa MAC in un array di byte
void macStrToByteArray(const String &macStr, uint8_t *macArray) {
    // Controllo se la lunghezza della stringa MAC è corretta
    if (macStr.length() != 17) {
        Serial.println("Formato MAC non valido");
        return;
    }

    char hexNum[3]; // Buffer temporaneo per memorizzare coppie di cifre esadecimali
    for (int i = 0; i < 6; ++i) {
        hexNum[0] = macStr.charAt(i * 3);
        hexNum[1] = macStr.charAt(i * 3 + 1);
        hexNum[2] = '\0'; // Terminatore di stringa per assicurare una corretta conversione

        // Converti la coppia di cifre esadecimali in un byte e memorizzalo nell'array
        macArray[i] = strtoul(hexNum, NULL, 16);
    }
}

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define ID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a7"


#endif
