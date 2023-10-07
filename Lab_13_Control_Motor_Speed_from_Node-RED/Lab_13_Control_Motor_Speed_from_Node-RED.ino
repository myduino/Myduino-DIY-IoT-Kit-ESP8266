#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <SimpleDHT.h>

int pinDHT11 = D4;
SimpleDHT11 dht11(pinDHT11);

const char ssid[] = "YOUR_SSID";
const char password[] = "YOUR_PASSWORD";
const char mqttPublishTemperature[] = "my-politeknik-YOUR_PHONE_NUMBER/suhu";
const char mqttPublishHumidity[] = "my-politeknik-YOUR_PHONE_NUMBER/kelembapan";

const char mqttSubscribeTopic[] = "my-politeknik-YOUR_PHONE_NUMBER/control";

WiFiClient net;
MQTTClient mqtt;

unsigned long lastMillis = 0;

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println(" connected!");
}

void messageReceived(String &topic, String &payload) {
  analogWrite(D1, payload.toInt());
}

void connectToMQTT(){
  Serial.print("Connecting to MQTT ...");
  
  mqtt.begin("broker.hivemq.com", net);
  mqtt.onMessage(messageReceived);

  String uniqueString = String(ssid) + String(random(1, 100)) + String(random(100, 1000));
  char uniqueClientID[uniqueString.length() + 1];
  
  uniqueString.toCharArray(uniqueClientID, uniqueString.length() + 1);
  
  while (!mqtt.connect(uniqueClientID)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected!");

  mqtt.subscribe(String(mqttSubscribeTopic));
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(D1, OUTPUT);

  // STEP 1 - Connecting to Wi-Fi router
  connectToWiFi();

  // STEP 2 - Connecting to MQTT broker
  connectToMQTT();
}

void loop() {
  mqtt.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if (!mqtt.connected()) {
    connectToMQTT();
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

  // STEP 3 - Publish a message with given interval.
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    
    mqtt.publish(String(mqttPublishTemperature), String(temperature));
    mqtt.publish(String(mqttPublishHumidity), String(humidity));
  }
}