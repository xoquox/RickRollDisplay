#include <ArduinoOTA.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SSD1306.h>
#include <SSD1306Ui.h>
#include "settings.h"
//#include <WiFiUDP.h>


WiFiClient wifiClient;
PubSubClient mqttClient;

WiFiUDP listener;


SSD1306   display(0x3c, D3, D4);
SSD1306Ui ui     ( &display );

void debug(const char* text)
{
  Serial.println(text);
  display.clear();
  display.drawString(64, 10, text);
  display.display();
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  // initialize dispaly
  display.init();
  display.clear();
  display.display();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.hostname(HOSTNAME_ESP);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

 listener.begin(8266);
    
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display.clear();
  display.drawString(64, 10, "Client running");
  display.display();
    
}

void callback(char* topic, byte* payload, unsigned int length)
{
   display.clear();
   char buffer[length + 1];
   for (int i = 0; i < length; i++) 
   {
    buffer[i]=((char)payload[i]);
   }
   buffer[length] = '\0';

   //display.drawString(64, 10, buffer);
   display.drawStringMaxWidth(64, 1, 100, buffer);
   display.display();
}

void connectMqtt() 
{

  while (!mqttClient.connected()) 
  {
    mqttClient.setClient(wifiClient);
    mqttClient.setServer(mqttHost, 1883);
    mqttClient.connect("tinyAlarm");
    mqttClient.subscribe("psa/rick_me");
    
    mqttClient.setCallback (callback);
    delay(1000);
  }
}


void loop()
{
  int cb = listener.parsePacket();
  if (cb) {
    IPAddress remote = listener.remoteIP();
    int cmd  = listener.parseInt();
    int port = listener.parseInt();
    int sz   = listener.parseInt();
    Serial.println("Got packet");
    Serial.printf("%d %d %d\r\n", cmd, port, sz);
    WiFiClient cl;
    if (!cl.connect(remote, port)) {
      Serial.println("failed to connect");
      return;
    }

    listener.stop();

    if (!ESP.updateSketch(cl, sz)) {
      Serial.println("Update failed");
    }
  }
  delay(100);
  connectMqtt();
  mqttClient.loop();
}

