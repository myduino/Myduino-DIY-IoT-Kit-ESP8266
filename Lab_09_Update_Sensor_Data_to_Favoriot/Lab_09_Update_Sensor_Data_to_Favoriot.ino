#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <SimpleDHT.h>

int pinDHT11 = D4;
SimpleDHT11 dht11(pinDHT11);

const char ssid[] = "YOUR_SSID";
const char password[] = "YOUR_PASSWORD";
const char deviceDeveloperId[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";

unsigned long lastMillis = 0;

const uint8_t httpsFingerprint[20] = {0x0A, 0x7C, 0x66, 0xBB, 0x16, 0x4F, 0xC6, 0x3D, 0x14, 0xCF, 0xB0, 0x90, 0x4A, 0xFE, 0x5F, 0x7E, 0x5D, 0x82, 0x8A, 0xE3};

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println(" connected!");
}

void setup() {
  Serial.begin(9600);
  while(!Serial);

  // STEP 1 - Connect to Wi-Fi router/hotspot
  connectToWiFi();

}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  
  // STEP 2 - Data Acquisition
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    return;
  }
  
  Serial.print("Data OK: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println(" %RH");

  if(millis() - lastMillis > 15000){
    lastMillis = millis();

    // STEP 3 - Favoriot HTTPS API Request - Send data to Favoriot data stream
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    
    client->setFingerprint(httpsFingerprint);

    HTTPClient https;

    https.begin(*client, "https://apiv2.favoriot.com/v2/streams");
  
    https.addHeader("Content-Type", "application/json");
    https.addHeader("apikey", deviceAccessToken);
  
    String body = "{";
    body += "\"device_developer_id\":\"" + String(deviceDeveloperId) + "\",";
    body += "\"data\": {";
    body += "\"temperature\": " + (String)temperature + ",";
    body += "\"humidity\": " + (String)humidity;
    body += "}";
    body += "}";
    
    int httpCode = https.POST(body);
  
    if(httpCode > 0){
      Serial.print("HTTPS Request: ");
      
      httpCode == 201 ? Serial.print("Success, ") : Serial.print("Error, ");
      Serial.println(https.getString());
    }
    else{
      Serial.println("HTTP Request Connection Error!");
    }
  
    https.end();
  }
}
