#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <SimpleDHT.h>

int pinDHT11 = D4;
SimpleDHT11 dht11(pinDHT11);

const char ssid[] = "YOUR_SSID";
const char password[] = "YOUR_PASSWORD";
const char deviceDeveloperId[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char favoriotCoapUrl[] = "coap.favoriot.com";

IPAddress favoriotCoapHost(159, 65, 134, 213); //DNS lookup coap.favoriot.com
String favoriotCoapMethod = "POST";
int favoriotCoapPort = 5683;

WiFiUDP udp;
Coap coap(udp, 1024);

unsigned long lastMillis = 0;

void callback_response(CoapPacket &packet, IPAddress ip, int port);

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");

  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = 0;

  //response from coap server
  if(packet.type == 3 && packet.code == 0){
    Serial.println("ping ok");
  }

  Serial.println("Stream Created");
}

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

  // coap client response callback.
  coap.response(callback_response);
  // start coap server/client
  coap.start();
}

void loop() {
  coap.loop();

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

    // STEP 3 - Favoriot CoAP Request - Send data to Favoriot data stream
    String parameters = "{";
    parameters += "\"device_developer_id\":\"" + String(deviceDeveloperId) + "\",";
    parameters += "\"data\": {";
    parameters += "\"temperature\": " + (String)temperature + ",";
    parameters += "\"humidity\": " + (String)humidity;
    parameters += "}";
    parameters += "}";

    String payloadJSON = "{\"method\":\"" + favoriotCoapMethod + "\",";
    payloadJSON += "\"apikey\":\"" + String(deviceAccessToken) + "\",";
    payloadJSON += "\"parameters\":" + parameters;
    payloadJSON += "}";

    char payload[1024];
    payloadJSON.toCharArray(payload, 1024);
    Serial.println("Send Request");
    int msgid = coap.send(favoriotCoapHost, favoriotCoapPort, favoriotCoapUrl, COAP_CON, COAP_POST, NULL, 0, (uint8_t *)payload, strlen(payload));
    
  }
}