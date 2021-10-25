#include <AccelStepper.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include "wificonfig.h"
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <WebSockets2_Generic.h>

#define IN1  D4
#define IN2  D3
#define IN3  D2
#define IN4  D1

const uint16_t turns = 4096;
AccelStepper stepper(AccelStepper::HALF4WIRE, D4, D2, D3, D1);


IPAddress broadcastIP(255, 255, 255, 255);
WiFiUDP Udp;
constexpr uint16_t PORT = 8266;
char packetBuffer[255];
bool alreadyConnected = false;

using namespace websockets2_generic;
WebsocketsClient webSocket;


int steps=12;
int currentStep=0;

enum mode {NONE,INIT,RUN,RESET};
mode MODE=NONE;
void setup()
{
  Serial.begin(115200);
  //  strip.begin();
  //  for (int i = 0; i < 12; i++)
  //    strip.setPixelColor(i, strip.Color(0, 0, 0));
  //  strip.show();

  // WiFi.mode(WIFI_AP);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.hostname("asdads");
  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(3000);
    ESP.restart();
  }

  if (MDNS.begin("photontable")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  ArduinoOTA.setHostname("PHOTON-TABLE");
  ArduinoOTA.begin();
  Udp.begin(PORT);

  stepper.setMaxSpeed(1000); // 1000
  stepper.setAcceleration(2000); // 8000

  // run callback when events are occuring
  webSocket.onEvent(webSocketEvent);

  webSocket.onMessage([&](WebsocketsMessage message)
  {
    Serial.print("Got Message: ");
    Serial.println(message.data());
    String data=message.data();
    int i1;
    if((i1=data.indexOf(":"))>0){
      String c=data.substring(0,i1);
      String c2=data.substring(i1+1,data.length());
     if(c=="INIT"){
          MODE=INIT;
          steps=c2.toInt();
          webSocket.send("Initializing");
//          moveTo(0); //(84/12));
          currentStep=0;
             webSocket.send("InitDone");
      } else if(c=="STEP"){
        if(currentStep<steps-1){
          currentStep++;
          move(((turns*(84/12))/steps));
        } else {
          webSocket.send("AlreadyFinished");
        }
      } else if(c=="RESET"){
        MODE=RESET;
        currentStep=0;
      //  moveTo(turns*(84/12));
      }
      
    }
    
  });

  //  webSocket.onEvent(webSocketEvent);
  //  webSocket.begin("192.168.1.56", 8082, "turnws/");
}


void webSocketEvent(WebsocketsEvent event, String data)
{
  if (event == WebsocketsEvent::ConnectionOpened)
  {
    alreadyConnected = true;

    Serial.print("[WSc] Connected to url: ");
    Serial.println(data);

    // send message to server when Connected
    webSocket.send("Connected");
  }
  else if (event == WebsocketsEvent::ConnectionClosed)
  {
    if (alreadyConnected)
    {
      Serial.println("[WSc] Disconnected!");
      alreadyConnected = false;
    }

  }
  else if (event == WebsocketsEvent::GotPing)
  {
    Serial.println("Got a Ping!");
  }
  else if (event == WebsocketsEvent::GotPong)
  {
    Serial.println("Got a Pong!");
  }



}
bool stopSent = false;
void moveTo(long i) {
  stopSent = false;
  stepper.moveTo(i); //(84/12));
}
void move(long i) {
  stopSent = false;
  stepper.move(i); //(84/12));
}


long lastPing = 0;
long lastConnect = 0;

void loop()
{
  ArduinoOTA.handle();

  if (!alreadyConnected && millis() - lastConnect > 5000) {
    bool connected = webSocket.connect(ip, 8082, "/turnws");

    lastConnect = millis();
  }
  if (webSocket.available())
  {
    webSocket.poll();
  }
  const uint32_t pause = 1000;
  static bool hin = true;
  stepper.run();

  const long cur = millis();
  if (cur - lastPing > 5000L) {
    webSocket.ping();
    lastPing = cur;
  }

  if (stepper.distanceToGo() == 0)
  {
    if (!stopSent) {
      stopSent = true;
      if(MODE==INIT){
        webSocket.send("InitDone");
        MODE=RUN;
      }else if(MODE==RESET){
        webSocket.send("ResetDone");
      } else {
        if(currentStep==steps-1){
      webSocket.send("Finished");
        } else {
      webSocket.send("GoalReached");
        }
      }
    }
  }
}
