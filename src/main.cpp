#include <Arduino.h>
#include <string>
#include <cctype>
#include "globals.h"
#include "canHelper.h"
// UUIDs for the BLE service and characteristic
#define SERVICE_UUID "e5553316-4ef1-4a9c-9941-a15f1d2394ea"
#define CHARACTERISTIC_UUID "c2e9e949-5e8a-4567-b09f-069c88f27d1b"
#define CHARACTERISTIC_MPPT_UUID "35438811-63ae-45c9-90ff-ed789a57b7f0"
#define CHARACTERISTIC_MPPT_2_UUID "62f5a02d-3ec6-4145-9318-c088f995fc52"
#define CHARACTERISTIC_TEMP_UUID "17551872-8786-4c56-b01c-9ed95b6b3e7b"
#define CHARACTERISTIC_SHUNT_01_UUID "e76aa94c-f764-4e6a-bf70-50d23b107b8f"
#define CHARACTERISTIC_SHUNT_02_UUID "21da5327-c37e-46ed-bcd3-059312a01727"

unsigned long canStartMillis;
unsigned long canCurrentMillis;
const unsigned long canStatusPeriod = 100;

// BLE server callback class for handling BLE events
class MyServerCallbacks : public BLEServerCallbacks
{
  // Called when a BLE device connects to the ESP32
  void onConnect(BLEServer *pServer)
  {
    Serial.println("Device Connected!");
  }

  // Called when a BLE device disconnects from the ESP32
  void onDisconnect(BLEServer *pServer)
  {
    pServer->startAdvertising(); // Restart advertising to allow new connections
    Serial.println("Device Disconnected!");
  }
};

// BLE characteristic callback class for handling data written to the characteristic
class ESP32Callbacks : public BLECharacteristicCallbacks
{
  // Called when data is written to the characteristic
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue(); // Get the written value

    // Log received data
    if (value.length() > 0)
    {
      if (value == "035b454f-2dd7-4c13-a734-997dbe7081d2") // LED_ON
      {
        canHelper::send_toggle_message(0, 1);
      }
      else if (value == "086a34e3-5de2-4f70-b9db-8e6c38fca634") // Device 02 On
      {
        canHelper::send_toggle_message(1, 1);
      }
      else if (value == "814b1dfc-ef07-4968-9f7a-ed2863471887") // Device 03 On
      {
        canHelper::send_toggle_message(2, 1);
      }
      else if (value == "a81d44d5-717b-4a3c-9290-cb3a73dd3a96") // Device04 On
      {
        canHelper::send_toggle_message(3, 1);
      }
      else if (value == "2738808b-7f1d-4228-884b-79e0a180ae3a") // Device05 toggle
      {
        canHelper::send_toggle_message(4, 1);
      }
      else if (value == "b3f739b7-b114-48db-bd7f-774e85597985") // Deivce 06 On
      {
        canHelper::send_toggle_message(5, 1);
      }
      else if (value == "aa6f906b-0cef-4bfe-84a3-7681fa60f00e") // Device 07 On
      {
        canHelper::send_toggle_message(6, 1);
      }
      else if (value == "3e08d853-ab35-4bad-9af9-caa40286743a") // Device 08 On
      {
        canHelper::send_toggle_message(7, 1);
      }
      else if (value == "93c8e31a-7bdc-443f-92e4-ba4d78cba630")
      { // All off
        canHelper::action_turn_all_devices_off();
      }
      else if (value == "a79fb878-3c5d-4133-a9ef-6138ae5adab1")
      {
        canHelper::action_turn_all_devices_on();
      }
    }
  }
};

class MySecurityCallbacks : public BLESecurityCallbacks
{
  uint32_t onPassKeyRequest()
  {
    Serial.println("Passkey requested");
    return 123456; // Use your preferred 6-digit passkey
  }

  void onPassKeyNotify(uint32_t pass_key)
  {
    Serial.printf("Passkey Notify: %d\n", pass_key);
  }

  bool onConfirmPIN(uint32_t pass_key)
  {
    Serial.printf("Confirm passkey: %d\n", pass_key);
    return true; // Automatically accept; optionally add logic
  }

  bool onSecurityRequest()
  {
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
  {
    if (cmpl.success)
    {
      Serial.println("Authentication successful!");
    }
    else
    {
      Serial.println("Authentication failed.");
    }
  }
};

void setup()
{
  Serial.begin(115200);
  canHelper::canSetup();
  BLEDevice::init("TrailCURRENT BT Gateway");
  BLEDevice::setSecurityCallbacks(new MySecurityCallbacks());
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setCapability(ESP_IO_CAP_OUT); // Use OUT for passkey display
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); // Set server callbacks

  // Create BLE service and characteristic
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);
  // Set characteristic callbacks and start the service
  pCharacteristic->setCallbacks(new ESP32Callbacks());
  pCharacteristic->setValue("Device Control");

  mpptCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_MPPT_UUID,
          BLECharacteristic::PROPERTY_NOTIFY);
  // Set characteristic callbacks and start the service
  mpptCharacteristic->setCallbacks(new ESP32Callbacks());
  mpptCharacteristic->setValue(mpptData01, 7);

  mppt2Characteristic = pService->createCharacteristic(
      CHARACTERISTIC_MPPT_2_UUID,
          BLECharacteristic::PROPERTY_NOTIFY);
  // Set characteristic callbacks and start the service
  mppt2Characteristic->setCallbacks(new ESP32Callbacks());
  mppt2Characteristic->setValue(mpptData02, 3);

  tempCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_TEMP_UUID,
          BLECharacteristic::PROPERTY_NOTIFY);
  // Set characteristic callbacks and start the service
  tempCharacteristic->setCallbacks(new ESP32Callbacks());
  tempCharacteristic->setValue(tempValues, 4);

  shuntCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_SHUNT_01_UUID,
          BLECharacteristic::PROPERTY_NOTIFY);
  // Set characteristic callbacks and start the service
  shuntCharacteristic->setCallbacks(new ESP32Callbacks());
  shuntCharacteristic->setValue(shuntData01, 7);

  shuntCharacteristic2 = pService->createCharacteristic(
      CHARACTERISTIC_SHUNT_02_UUID,
          BLECharacteristic::PROPERTY_NOTIFY);
  // Set characteristic callbacks and start the service
  shuntCharacteristic2->setCallbacks(new ESP32Callbacks());
  shuntCharacteristic2->setValue(shuntData02, 3);

  delay(1000);
  pService->start();

  // Setup BLE advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(0x1234);
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();

  Serial.println("BLE Device Initialized and Ready");
}

void loop()
{
  canCurrentMillis = millis();
  if (canCurrentMillis - canStartMillis >= canStatusPeriod)
  {
    canHelper::canLoop();
    canStartMillis = canCurrentMillis;
  }
}