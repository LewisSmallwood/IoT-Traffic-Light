#include <Arduino.h>

// ESP266 WiFi and HTTP libraries.
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUDP.h>

ESP8266WiFiMulti WiFiMulti;
WiFiUDP UDP;

// WiFi config
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

int threshold = 200;      // Resistance threshold triggle value.
                          // 200 calibrated to detect me, but not light shopping/animals/etc.
                      
int pressureSensor = 0;   // Pin A0
int value = 0;            // Resistance value of pressure pad.

// The last 1 second of sensed resistance values.
// Takes 1 poll every 200ms and averages
// over 1 second to avoid false positives.
int history[5] = {0, 0, 0, 0, 0};

boolean isSitting = false;

IPAddress broadcast_ip(255,255,255,255);                 // BROADCAST IP

// Target MAC address of device to switch on when sat in sofa.
byte tvMACAddress[] = { 0x80, 0xC7, 0x55, 0x38, 0x3D, 0x73 };

void setup() {
  Serial.begin(115200);

  // Use the onboard LED to indicate if someone is sat in the sofa or not.
  pinMode(LED_BUILTIN, OUTPUT);

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

  // Start the UDP client.
  UDP.begin(9);
}

// Once setup has finished, begin...
void loop() {
  // If connected to WiFi, begin sensing.
  if ((WiFiMulti.run() == WL_CONNECTED)) {
      WiFi.hostname("Sofa");

      // Get the current pressure plate 
      value = analogRead(pressureSensor);
      addToHistory(history, value);

      // Check if the average pressure pad resistance
      // in the last second exceeds the threshold.
      if (average(history, 5) < threshold) {
          setSittingState(true);
      } else {
          setSittingState(false);
      }
  }

  delay(200);
}

// Called when the sitting state changes.
void onChangedSittingState() {
    if (isSitting) {
        digitalWrite(LED_BUILTIN, HIGH);
              
    } else {
        digitalWrite(LED_BUILTIN, LOW);
        SendWakeOnLAN(tvMACAddress, sizeof tvMACAddress);
    }
}

// Set the sitting state.
void setSittingState(boolean newState) {
    if (newState != isSitting) {
        isSitting = newState;
        onChangedSittingState();
    }
}

// Gets the average of all numbers within an int array.
float average (int * array, int len) {
    long sum = 0L;
    for (int i = 0 ; i < len ; i++) {
        sum += array[i];
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

// Send a Wake on LAN request to MAC address
void SendWakeOnLAN(byte * mac, size_t mac_size) {
    byte preamble[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    byte i;
    
    UDP.beginPacket(broadcast_ip, 9);
  
    UDP.write(preamble, sizeof preamble);
    
    for (i = 0; i < 16; i++) {
        UDP.write(mac, mac_size);
    }
    
    UDP.endPacket();
}
