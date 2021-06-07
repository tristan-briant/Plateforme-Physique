#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_system.h>
#include <BLE2902.h>
#include <pref-util.h>

extern float accData[10];
extern float gyrData[10];
extern float magData[10];

//uuid's can be generated on https://www.uuidgenerator.net/
#define SERVICE_UUID "65460a23-3bbf-4c67-be33-1ccc83259975"

BLECharacteristic *aCharacteristic1;
BLECharacteristic *aCharacteristic2;
BLECharacteristic *aCharacteristic3;
BLEService *aService;
BLEServer *aServer;

void BLEinit()
{
    BLEDevice::init(getDeviceName());
    //BLEServer *
    aServer = BLEDevice::createServer();
    BLEService *aService = aServer->createService(SERVICE_UUID);

    aCharacteristic1 = aService->createCharacteristic(
        "9e34301c-5f00-4875-8cab-86c5b043dc8e",
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    aCharacteristic1->addDescriptor(new BLE2902());

    aCharacteristic2 = aService->createCharacteristic(
        "59f51a40-8852-4abe-a50f-2d45e6bd51ac",
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    aCharacteristic2->addDescriptor(new BLE2902());

    aCharacteristic3 = aService->createCharacteristic(
        "360404ec-3ce8-47d9-93ff-727c1cdc1169",
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    aCharacteristic3->addDescriptor(new BLE2902());

    //BLE server is being started
    aService->start();
    BLEAdvertising *aAdvertising = aServer->getAdvertising();
    aAdvertising->start();
}

bool BLEisConnected()
{
    return aServer->getConnectedCount() > 0;
}

void BLEsendData()
{
    aCharacteristic1->setValue((uint8_t *)accData, 20);
    aCharacteristic1->notify();

    aCharacteristic2->setValue((uint8_t *)gyrData, 20);
    aCharacteristic2->notify();

    aCharacteristic3->setValue((uint8_t *)magData, 20);
    aCharacteristic3->notify();
}
