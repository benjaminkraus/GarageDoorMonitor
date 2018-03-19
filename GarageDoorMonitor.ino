#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "TOTP.h"
#include "LocalSettings.h"

/* Define the following constants within LocalSettings.h.

// WiFi definitions
const char WiFiSSID[] = "ssid";
const char WiFiPSK[] = "password";

// Pin definitions
const int LED_PIN = 5; // Thing's onboard, green LED
const int DOOR_CLOSED_PIN = 0; // Pin connected to the garage door closed reed switch
const int DOOR_CONTROL_PIN = 4; // Pin connected to the garage door control relay

// ThingSpeak settings
const char* thingSpeakWriteKey = "writekey";

*/

const char* thingSpeakHost = "api.thingspeak.com";
String thingSpeakURL = "/update.json";

void setup() 
{
  initHardware();
  connectWiFi();
}

void loop() 
{
}

void initHardware()
{
  Serial.begin(115200);
  
  pinMode(DOOR_CLOSED_PIN, INPUT_PULLUP);
  
  pinMode(DOOR_CONTROL_PIN, OUTPUT);
  digitalWrite(DOOR_CONTROL_PIN, LOW);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void connectWiFi()
{
  byte ledStatus = LOW;

  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  Serial.print("Connecting to ");
  Serial.println(WiFiSSID);
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  digitalWrite(LED_PIN, LOW);
  delay(250);
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Update ThingSpeak with the IP address.
  updateThingSpeak(String("status=") + WiFi.localIP().toString());
}

void updateThingSpeak(String data) {
  Serial.print("Connecting to ");
  Serial.println(thingSpeakHost);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(thingSpeakHost, 80)) {
    Serial.println("Connection failed");
    return;
  }
  
  // Create a URI for the request
  String url = thingSpeakURL;
  url += String("?api_key=") + thingSpeakWriteKey + "&" + data;

  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // Send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
    "Host: " + thingSpeakHost + "\r\n" +
    "Connection: close\r\n\r\n");
  
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Client timeout");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("Closing connection");
}

