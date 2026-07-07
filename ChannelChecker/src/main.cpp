#include <ElegantOTA.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <RichWave.h>
#include <ESPmDNS.h>
#include "index.h"

#define DATA_PIN 2 //ch1 pin on rx5808
#define LATCH_PIN 1 //ch2 pin on rx5808
#define CLK_PIN 0 //ch3 pin on rx5808
#define RSSI_IN 3 
int rssi_thr = 1000;
const char *ssid = "ChannelChecker";
const char *password = "";
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
DNSServer dnsServer;
JSONVar channels;
const char *host = "chcheck";
IPAddress local_IP(10, 0, 0, 1);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
String databuffer = "";
// CH1-CH3 pins are used as SPI Bus pins then! CH1 = DATA; CH2 = LATCH; CH3 = CLOCK
// DATA PIN; LATCH PIN; CLK PIN
RichWave vrx(2,1,0);
bool channelstatus[8] = {0};
const int freqs[8] = {5658, 5695, 5731, 5769, 5806, 5843, 5880, 5917};

String getSensorReadings(){
  
  channels["r1"] = channelstatus[0];
  channels["r2"] = channelstatus[1];
  channels["r3"] = channelstatus[2];
  channels["r4"] = channelstatus[3];
  channels["r5"] = channelstatus[4];
  channels["r6"] = channelstatus[5];
  channels["r7"] = channelstatus[6];
  channels["r8"] = channelstatus[7];
  
  String jsonString = JSON.stringify(channels);
  return jsonString;
}

void handleRoot() {

  //databuffer = String("<html><head><meta content=\"width=device-width\"><style> body{font-size: 56px; text-align:center; background-color:#11d0ed;}</style> <h1>GPS LOGGER<br> Max velocity: <a href=""/max"">max speed</a> <br>Current velocity: <a href=""/current"">current speed</a><br>Reset Data: <a href=""/reset"">reset</a><br>Start: <a href=""/start"">start</a><br>Stop log: <a href=""/stop"">stop</a><br>terminal: <a href=""/term.html"">terminal</a><br>update: <a href=""/update"">UPDATE</a></h1> <body>""</body></html>");
  databuffer = index_html;
  server.send(200, "text/html", databuffer);
  databuffer.remove(0);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  Serial.println("WS event");
  switch(type) {
    
    case WStype_TEXT:
      if (strncmp((char*)payload, "THR:", 4) == 0) {
        
        String payloadStr = String((char*)payload);
        rssi_thr = payloadStr.substring(4).toInt();
        Serial.print("New RSSI Threshold set to: ");
        Serial.println(rssi_thr);
      } else if (strcmp((char*)payload, "data") == 0) {
        
        String values = getSensorReadings();
        webSocket.sendTXT(num, values);
      } else if (strcmp((char*)payload, "ResetButton") == 0) {
       
      }   
      break;
      case WStype_ERROR:
        Serial.println("error");
      break;
      case WStype_DISCONNECTED:
        Serial.println("Disconnected!\n");
      break;
      case WStype_CONNECTED:         //IPAddress ip = webSocket.remoteIP(num);
        Serial.println(" Connected ws");
    
        // send message to client
        //webSocket.sendTXT(num, "Connected\n");		
      break; 
  
  } 

}

void setup()  {

  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(RSSI_IN, INPUT);
  WiFi.softAPdisconnect();
  esp_wifi_start();
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  // WiFi.softAP(ssid, "", 10, false, 4);
  Serial.println(WiFi.softAP(ssid, "", 10, false, 4) ? "Ready" : "Failed!");
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  if (!MDNS.begin(host)) { // http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.on("/", handleRoot);
  server.onNotFound(handleRoot);
  dnsServer.start(53, "*", WiFi.softAPIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  server.begin();
    


}

void loop() {

  for (int i = 0; i < 8; i++) {
    vrx.setFrequency(freqs[i]);
    delay(50);
    int temp_rssi = 0;
    for (int j = 0; j < 10; j++) {
      temp_rssi = temp_rssi + analogRead(RSSI_IN);
      delay(2);
    }
    temp_rssi = temp_rssi / 10;
    if (temp_rssi > rssi_thr) {
      channelstatus[i] = 1;
    } else {
      channelstatus[i] = 0;
    }
    Serial.print(" R");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(temp_rssi);
    Serial.print(" St: ");
    Serial.print(channelstatus[i]);
  }
  Serial.println("");
  dnsServer.processNextRequest();
  server.handleClient();
  ElegantOTA.loop();
  webSocket.loop();

}