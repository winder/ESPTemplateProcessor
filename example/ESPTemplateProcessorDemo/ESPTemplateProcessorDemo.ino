#ifdef ESP8266
#define WebServer ESP8266WebServer
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif

#include <ESPTemplateProcessor.h>

const char* ssid = "........";
const char* password = "........";

WebServer server(80);

const int led = 13;

String indexProcessor(const String& key) {
  Serial.println(String("KEY IS ") + key);
  if (key == "TITLE") return "Hello World!";
  else if (key == "BODY") return "It works!";

  return "oops";
}

void handleRoot() {
  if (ESPTemplateProcessor(server).send(String("/index.html"), indexProcessor)) {
    Serial.println("SUCCESS");
  } else {
    Serial.println("FAIL");
    server.send(200, "text/plain", "page not found.");
  }
}

void setup(void){
  SPIFFS.begin();

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
