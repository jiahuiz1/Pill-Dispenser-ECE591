#include "BLEDevice.h"
#include "esp32-hal-cpu.h"
#include "analogWrite.h"

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3        /* Time ESP32 will go to sleep (in seconds) */

#define MOTOR_PIN 17  /* Pin for the vibration motor */
#define LED1_PIN 26      /* Pin for LED 1 */
#define LED2_PIN 27      /* pin for LED 2 */

/* Specify the Service UUID of Server */
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
/* Specify the Characteristic UUID of Server */
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean isConnected = false;
static boolean doScan = true;
static char serverMessage[1000] = "defaultValue";
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* braceletServer;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData, size_t length, bool isNotify)
{
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient* pclient)
  {
    
      doScan = false;
  }

  void onDisconnect(BLEClient* pclient)
  {
    isConnected = false;
     doScan = true;
    Serial.println("onDisconnect");
  }
};

/* Start connection to the BLE Server */
bool connectToServer()
{
  Serial.print("Forming a connection to ");
  Serial.println(braceletServer->getAddress().toString().c_str());
    
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

    /* Connect to the remote BLE Server */
  pClient->connect(braceletServer);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

    /* Obtain a reference to the service we are after in the remote BLE server */
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");


  /* Obtain a reference to the characteristic in the service of the remote BLE server */
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  /* Read the value of the characteristic */
  /* Initial value is 'Hello, World!' */
  if(pRemoteCharacteristic->canRead())
  {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if(pRemoteCharacteristic->canNotify())
  {
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  }

    isConnected = true;
    return true;
}
/* Scan for BLE servers and find the first one that advertises the service we are looking for. */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
 /* Called for each advertising BLE server. */
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    /* We have found a device, let us now see if it contains the service we are looking for. */
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
      BLEDevice::getScan()->stop();
      braceletServer = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;

    }
  }
};


void setup()
{
  /* Set up the serial connection and the ESP32 */
  Serial.begin(115200);
  Serial.print("Starting Arduino BLE Client application with CPU @ ");
  setCpuFrequencyMhz(80); //Set CPU clock
  Serial.print(getCpuFrequencyMhz());
  Serial.println("MHz.");

  /* Light sleep configuration for optional power saving */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  /* Set up peripheral pins */
  pinMode(MOTOR_PIN, OUTPUT); 
  pinMode(LED1_PIN, OUTPUT); 
  pinMode(LED2_PIN, OUTPUT);  
  pinMode(0, INPUT);

  /* Set up Bluetooth Low Energy */
  BLEDevice::init("Test-Bracelet");

  /* Retrieve a Scanner and set the callback we want to use to be informed when we
     have detected a new device.  Specify that we want active scanning and start the
     scan to run for 5 seconds. */
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);  
  pBLEScan->start(5, false);
  
  

  
}


void loop()
{

  /* If the flag "doConnect" is true, then we have scanned for and found the desired
     BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
     connected we set the connected flag to be true. */
  if (doConnect == true)
  {
    if (connectToServer())
    {
      Serial.println("We are now connected to the BLE Server.");
    } 
    else
    {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  else   

  /* If we are connected to a peer BLE Server, update the characteristic each time we are reached
     with the current time since boot */
  if (isConnected)
  {
    
    analogWrite(27, 0);

    /* Read remote characteristic to determine if dispensing is taking place */
    strcpy(serverMessage, pRemoteCharacteristic->readValue().c_str());
    Serial.print("Received message from server: ");
    Serial.println(serverMessage);

    if (String(serverMessage).equals("dispensing"))
    {
      analogWrite(LED1_PIN, 10);
      analogWrite(MOTOR_PIN, 255/2);
    }
    else 
    {
      analogWrite(LED1_PIN, 0);
      analogWrite(MOTOR_PIN, 0);
    }
    
  
  }

  /* If we are not connected, and scanning is required */
  else if(doScan)
  {   
    analogWrite(27, 8);
    BLEDevice::getScan()->start(20);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
//    Serial.println("Going to sleep now");
//    if (doScan) esp_light_sleep_start();
  }
  
  delay(2000); /* Delay a second between loops */
}
