#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

ESP8266WebServer server(80);

String webPage = "";
int gpio5_pin = 5;
int gpio4_pin = 4;

void setup(){
  // preparing GPIOs
  pinMode(gpio5_pin, OUTPUT);
  digitalWrite(gpio5_pin, LOW);
  pinMode(gpio4_pin, OUTPUT);
  digitalWrite(gpio4_pin, LOW);

  Serial.begin(9600);
  while(!Serial);

  WiFi.begin(ssid, password);

  // Wait for connection
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());

  if(mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  webPage += "<h1>ESP8266 Web Server</h1><p>Socket #1 <a href=\"socket1On\"><button>ON</button></a>&nbsp;<a href=\"socket1Off\"><button>OFF</button></a></p>";

  webPage += "<p>Socket #2 <a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"socket2Off\"><button>OFF</button></a></p>";


  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });

  server.on("/socket1On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio5_pin, HIGH);
    delay(1000);
  });

  server.on("/socket1Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio5_pin, LOW);
    delay(1000);
  });

  server.on("/socket2On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio4_pin, HIGH);
    delay(1000);
  });

  server.on("/socket2Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(gpio4_pin, LOW);
    delay(1000);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(){
  server.handleClient();
}
