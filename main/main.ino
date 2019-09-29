#include <Arduino.h>

// ESP266 WiFi and HTTP libraries.
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

HTTPClient http;

ESP8266WiFiMulti WiFiMulti;
#define RED_LED D0
#define YELLOW_LED D1
#define GREEN_LED D2

// WiFi config
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

void setup() {
  Serial.begin(115200);

  // LEDs
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);

  // Wait 3 seconds for the WiFi card to initialise.
  for (uint8_t t = 4; t > 0; t--) {
      Serial.printf("[SETUP] WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
  }
  
  // Connect to the WiFi using credentials.
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  // Set the device hostname to Sofa.
  if (WiFi.status() == WL_CONNECTED) {
      WiFi.hostname("TRAFFIC-LIGHT");
  }
}

// Once setup has finished, begin...
void loop() {
  // If connected to WiFi, begin sensing.
  if ((WiFiMulti.run() == WL_CONNECTED)) {
      WiFi.hostname("TRAFFIC-LIGHT");
      
      http.begin("http://api.bespoke.dev/get-light-colour");
      int httpCode = http.GET();
      
      if (httpCode > 0) {
          String payload = http.getString();
          Serial.println(payload);
          
          if (payload == "green") {
              digitalWrite(RED_LED, LOW);
              digitalWrite(YELLOW_LED, LOW);
              digitalWrite(GREEN_LED, HIGH);
          } else if (payload == "red") {
              digitalWrite(RED_LED, HIGH);
              digitalWrite(YELLOW_LED, LOW);
              digitalWrite(GREEN_LED, LOW);
          } else {
              digitalWrite(RED_LED, LOW);
              digitalWrite(YELLOW_LED, HIGH);
              digitalWrite(GREEN_LED, LOW);
          }
            
      } else {
        digitalWrite(RED_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
      }
      
      http.end();
      
  } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
  }

  delay(30000);
}

