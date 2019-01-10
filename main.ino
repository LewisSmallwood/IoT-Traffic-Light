#include <Arduino.h>

// ESP266 WiFi and HTTP libraries.
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

// WiFi config
const WIFI_SSID = "";
const WIFI_PASSWORD = "";

int threshold = 200;  // Resistance threshold triggle value.
                      // 200 calibrated to detect me, but not light shopping/animals/etc.
                      
int knockSensor = 0;  // Pin A0
int value = 0;        // Resistance value of pressure pad.

// The last 1 second of sensed resistance values.
// Takes 1 poll every 200ms and averages
// over 1 second to avoid false positives.
int history[5] = {0, 0, 0, 0, 0};

void setup() {
  Serial.begin(115200);

  // Use the onboard LED to indicate if someone is sat in the sofa or not.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.println();

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
      WiFi.hostname("Sofa");
  }
}

// Once setup begin...
void loop() {
  // If connected to WiFi, begin sensing.
  if ((WiFiMulti.run() == WL_CONNECTED)) {
      WiFi.hostname("Sofa");
      
      value = analogRead(knockSensor);
      addToHistory(history, value);
  
      if (average(history, 5) < threshold) {
          digitalWrite(LED_BUILTIN, HIGH);
      } else {
          digitalWrite(LED_BUILTIN, LOW);
      }
  }

  delay(200); // Wait for 200ms before looping again.
}

// Gets the average of all numbers within an array.
float average (int * array, int len)  // assuming array is int.
{
    long sum = 0L ;  // sum will be larger than an item, long for safety.
    
    for (int i = 0 ; i < len ; i++) {
        sum += array [i] ;
    }
    
    return  ((float) sum) / len;
}

// Push a value to the history shifting history backwards.
void addToHistory(int * array, int value) {
    array[4] = array[3];
    array[3] = array[2];
    array[2] = array[1];
    array[1] = array[0];
    array[0] = value;
}

