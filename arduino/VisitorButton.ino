#include <PubSubClient.h>
#include "config.h"
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>

#define PIN        D6
#define BUTTON     D5
#define NUMPIXELS 3
#define DELAYVAL 500

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
WiFiClient espClient;
PubSubClient client(espClient);

int ticks = 0;
bool lightActive = false;

void setup_wifi() {
  delay(10);
  pixels.setPixelColor(0, pixels.Color(10, 0, 0));
  pixels.show();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  pixels.setPixelColor(1, pixels.Color(10, 0, 0));
  pixels.show();

}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(topic=="visitorbutton/response"){
      JSONVar apiJson = JSON.parse(messageTemp);
      Serial.print("Changing RGB to ");
      Serial.println(messageTemp);
      pixels.clear();
      int pos = apiJson["pos"];
      int r = apiJson["color"]["r"];
      int g = apiJson["color"]["g"];
      int b = apiJson["color"]["b"];
      pixels.setPixelColor(pos, pixels.Color(r, g, b));
      pixels.show();
      delay(500);
      lightActive = true;
      ticks = 0;
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      client.subscribe("visitorbutton/response");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pixels.begin();
  setup_wifi();
  pinMode(BUTTON, INPUT);
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  pixels.setPixelColor(2, pixels.Color(10, 0, 0));
  pixels.show();
  delay(1000);
  pixels.clear();
  pixels.show();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");
    
  pixels.clear();

  if (lightActive) {
    ticks = ticks + 1;
    if (ticks > 800) {
      pixels.clear();
      pixels.show();
      lightActive = false;
      Serial.println("Ticks done");
    } else {
     delay(5);
    }
  }
  
  if (digitalRead(BUTTON) == HIGH) {
    Serial.println("We have a knock!");
    client.publish("visitorbutton/click", "1");
    pixels.clear();
    pixels.show();
    delay(5000);
  }


}
