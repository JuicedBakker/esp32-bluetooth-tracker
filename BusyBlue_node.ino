#include "painlessMesh.h"
#include <WiFi.h>
#include <esp_now.h>
#include <BLEDevice.h>
#include <iostream>
#include <ArduinoJson.h>
#include <string>
#include <sstream>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define MESH_PREFIX "BusyBlue"
#define MESH_PASSWORD "busyblue123"
#define MESH_PORT 5555
#define SCAN_TIME 15
#define MSG_INTERVAL 15
#define ID 5
#define MASTER 1

using std::string; using std::hex;
using std::stringstream;

String nodeAddress;
Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
bool isEndpoint = false;
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
  }
};
String hexToStr(uint8_t* arr, int n)
{
  String result;
  result.reserve(2*n);
  for (int i = 0; i < n; ++i) {
    if (arr[i] < 0x10) {result += '0';}
    result += String(arr[i], HEX);
  }
  return result;
}
void sendMessage() ; // Prototype 

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  MyAdvertisedDeviceCallbacks myCallbacks;
  pBLEScan->setAdvertisedDeviceCallbacks(&myCallbacks);
  pBLEScan->setActiveScan(true);
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  int count = foundDevices.getCount();
  
  StaticJsonDocument<4096> doc;
  JsonArray array = doc.to<JsonArray>();

  for (int i = 0; i < count; i++)
  { 
    BLEAdvertisedDevice d = foundDevices.getDevice(i);
    JsonObject device = array.createNestedObject(); // Create a new JSONObject for the device
    device["nodeId"] = ID;
    device["address"] = hexToStr(*d.getAddress().getNative(), 6);
    device["rssi"] = d.getRSSI(); 
    if ((i > 0) && (i % 13 == 0) || (i == count-1))
    {
      String output;
      serializeJson(array, output);
      mesh.sendBroadcast(output);
      if (ID == MASTER)
      {
        Serial.println(output);
      }
      doc.clear();
      array = doc.to<JsonArray>();
    }
  }
  taskSendMessage.setInterval(TASK_SECOND * MSG_INTERVAL);
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  #if ID == MASTER
    Serial.printf("%s\n", msg.c_str());
  #endif
}

void newConnectionCallback(uint32_t nodeId) {
  BLEDevice::init("");
}

void changedConnectionCallback() {
}

void nodeTimeAdjustedCallback(int32_t offset) {

}

void setup() {
  Serial.begin(115200);
  esp_err_t errRc=esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9); 
  mesh.setDebugMsgTypes( ERROR | STARTUP ); 

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}