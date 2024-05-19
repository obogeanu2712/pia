#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

// TODO -- BLE server name
#define bleServerName ""

bool deviceConnected = false;
StaticJsonDocument<200> doc;
// TODO --  Generate a unique UUID for your Bluetooth service
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID "ca73b3ba-39f6-4ab3-91ae-186dc9577d99"

// Wi-Fi network parameters
String ssid = "";
String password = "";
bool connected_WiFi;
#define CONNECT_TIMEOUT 15000 // ms
long connectStart = 0;
String teamId = "A23";
// BEGIN CHARACTERISTICS
// FOR THE FOLLOWING LINES CHANGE ONLY THE UUID of the characteristic
// Define a caracteristic with the properties: Read, Write (with response), Notify
// Use the above link for generating UUIDs
BLECharacteristic characteristic(
  CHARACTERISTIC_UUID,
  BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
);
// Define a descriptor characteristic
// IMPORTANT -- The characteristic should have the descriptor UUID 0x2902 or 0x2901
BLEDescriptor *characteristicDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2902));

// Setup callbacks onConnect and onDisconnect (no change necessary)
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
    pServer->startAdvertising();
  }
};
// End setup callbacks onConnect and onDisconnect (no change necessary)

// Setup callbacks for charactristics
// IMPORTANT -- all caracteristics can use the same callbacks class 
// or you can define a different class for each characteristic containing the onWrite method
// The onWrite callback method is called when data is received by the ESP32
// This is where you will write you logic, according to the app specs
class CharacteristicsCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
        // Get characteristic value sent from the app, according to the specs
        std::string data = characteristic->getValue();
        Serial.println(data.c_str()); // <-- This is the message sent from the app, according to the specs
        DeserializationError err = deserializeJson(doc, data);
        String action = doc["action"];
        if(action=="getNetworks")
        {
            String message_teamId = doc["teamId"];
            String teamId = message_teamId;
            int n = WiFi.scanNetworks();

            if (n == 0)
            {
            Serial.println("No network");
            }
            else
            {
            for (int i = 0; i < n; i++)
            {
                DynamicJsonDocument network(1024);
                network["ssid"] = WiFi.SSID(i);
                network["strength"] = WiFi.RSSI(i);
                network["encryption"] = WiFi.encryptionType(i);
                network["teamId"] = teamId;
                
                String jsonString;
                serializeJson(network, jsonString);
                characteristic->setValue(jsonString.c_str());
                characteristic->notify();
                delay(100);
            }
            }
        }
        else if(action=="connect")
        {
            String message_ssid = doc["ssid"];
            ssid = message_ssid;
            String message_password = doc["password"];
            password = message_password;

            WiFi.begin(ssid.c_str(), password.c_str());

            int time = 0;
            while (WiFi.status() != WL_CONNECTED)
            {
                Serial.println(".");
                delay(500);
                time += 500;
                if (time > 10000)
                break;
            }
            bool wifi_connected;
            if (WiFi.status() != WL_CONNECTED)
            {
                wifi_connected = false;
            }
            else
            {
                wifi_connected = true;
            }
            DynamicJsonDocument doc(200);
            doc["ssid"] = ssid;
            doc["connected"] = wifi_connected;
            doc["teamId"] = teamId;
            String output;
            serializeJson(doc, output);
            characteristic->setValue(output.c_str());
            characteristic->notify();
        }
        else if (action == "getData")
        {

          //getData eu
            String URL = "http://proiectia.bogdanflorea.ro/api/avatar-the-last-airbender/characters";
            HTTPClient client;
            client.begin(URL);
            int statusCode = client.GET();
            if (statusCode != 200)
                Serial.println("Connection failed");
            else
            {
                String data = client.getString();
                Serial.println(data);
                DynamicJsonDocument doc(16000);

                DeserializationError err = deserializeJson(doc, data);

                JsonArray recordsArray = doc.as<JsonArray>();

                DynamicJsonDocument finalData(16000);

                for (JsonObject record : recordsArray)
                {
                    String  id = record["_id"];
                    finalData["id"] = id;
                    String name = record["name"];
                    finalData["name"] = name;
                    String image = record["photoUrl"];
                    finalData["image"] = image;
                    finalData["teamId"] = teamId.c_str();
                    String response;
                    serializeJson(finalData, response);
                    Serial.println(response);
                    characteristic->setValue(response.c_str());
                    characteristic->notify();
                }
            }
            client.end();
        }
        else if (action == "getDetails")
        {
            String id = doc["id"];
            String URL = "http://proiectia.bogdanflorea.ro/api/avatar-the-last-airbender/character?_id=" + id;
            HTTPClient client;

            client.begin(URL);
            int statusCode = client.GET();

            if (statusCode != 200)
                Serial.println("Error on sending GET request");
            else
            {
                String data = client.getString();
                Serial.print(data);
                DynamicJsonDocument doc(16000);

                DeserializationError err = deserializeJson(doc, data);

                DynamicJsonDocument finalData(16000);

                String id = doc["_id"];
                finalData["id"] = id;
                String image = doc["photoUrl"];
                finalData["image"] = image;
                String name = doc["name"];
                finalData["name"] = name;
                finalData["teamId"] = teamId.c_str();
                
                //description
                String description;

                //Allies
    
                JsonArray allies = doc["allies"];
                if(allies.size()>0)
                {
                  description += "Allies: ";
                  for(int i=0;i<(allies.size())-1;i++)
                  {
                    description += (allies[i].as<String>() + ", ");
                  }
                  description += (allies[allies.size()-1].as<String>()+".\n\n");
                }
            
                //Enemies
                
                JsonArray enemies = doc["enemies"];
                if(enemies.size()>0)
                {
                  description += "Enemies: ";
                  for(int i=0;i<(enemies.size())-1;i++)
                  {
                    description += (enemies[i].as<String>() + ", ");
                  }
                  description += (enemies[enemies.size()-1].as<String>()+".\n\n");
                }

                //Affiliation
                String affiliation = doc["affiliation"];
                description += ("Affiliation: " + affiliation + ".\n\n");

                finalData["description"] = description;
                
                String response;
                serializeJson(finalData, response);
                characteristic->setValue(response.c_str());
                characteristic->notify();
            }
            client.end();
        }
      // Possible steps:
      // 1. Deserialize data using the ArduinoJson library
      // 2. Get the action from the JSON object
      // 3. Check action and perform the corresponding operations
      // IMPORTANT -- if it is an API project, connect to WiFi when the appropriate action is requested and 
    // make sure to set the http request timeout to 10s or more
      // IMPORTANT -- If using the ArduinoJson library, use a DynamicJsonDocument with the size of 15000
      // 5. Define the response structure, according to the app specifications
      // (Use JsonArray or JsonObject, depending on the response type)
      // IMPORTANT -- The cacapcity of the response array/object must not exceed 512B, especially for BLE
      // 6. Populate the response object according to the app specs
      // 7. Encode the response object as a JSON string
      // 8. Write value to the characteristic and notify the app
      
      // TODO -- Write your code
      

    }
};


BLEServer *pServer;



void setup() {
  // Start serial communication 
  Serial.begin(115200);

  // BEGIN DON'T CHANGE
  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  // Set server callbacks
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bleService = pServer->createService(SERVICE_UUID);

  // Create BLE characteristics and descriptors
  bleService->addCharacteristic(&characteristic);  
  characteristic.addDescriptor(characteristicDescriptor);
  // Set chacrateristic callbacks
  characteristic.setCallbacks(new CharacteristicsCallbacks());
  
  // Start the service
  bleService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  // END DON'T CHANGE
}

void loop()
{

}