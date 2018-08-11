////////////////////////////////////////////////////////////////////////////////////////
// Libraries

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>

#include "configuration.h"

////////////////////////////////////////////////////////////////////////////////////////
// Settings

// Telnet
WiFiServer TelnetServer(23); // Telnet Port
WiFiClient TelnetClient[MAX_TELNET_CLIENTS];
bool ConnectionEstablished; // Flag for telnet

// Web-Server
ESP8266WebServer WebServer(80); // HTTP Port

// Start UDP
WiFiUDP UDP;

////////////////////////////////

const int O_Module_NUM = (sizeof(O_Module_Address) / sizeof(O_Module_Address[0]));  // Number of output modules
int O_Module_VAL[O_Module_NUM][8];   // Flag for output state

const int I_Module_NUM = (sizeof(I_Module_Address) / sizeof(I_Module_Address[0]));  // Number of input modules
byte I_Module_VAL[I_Module_NUM];  // Flag for input state

////////////////////////////////////////////////////////////////////////////////////////
// Load Functions

#include "telnet.h"

////////////////////////////////////////////////////////////////////////////////////////
// Setup

void setup()
{
  Serial.begin(115200);
  Serial.println(Sketch_Name);
  Serial.println(Sketch_Version);

  Wire.begin();
  
  Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
  Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet); // auskommentieren, falls eine dynamische IP bezogen werden soll
  WiFi.begin(ssid, password);

  CheckWifiStatus();
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting Telnet server");
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);

  // OTA

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  ArduinoOTA.setPassword((const char *)"1234");
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  ArduinoOTA.begin();

  WebServer.on("/set", ArgCheck); // Handle HTTP GET command incoming
  WebServer.on("/", handleRoot); // Handle root website
  WebServer.on("/input", handleInput); // Handle Input website
  WebServer.on("/output", handleOutput); // Handle Output website

  // Start the server
  Serial.println("Starting WebServer");
  WebServer.begin();

  OpenLoxoneURL(LoxoneRebootURL);

  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(InterruptPin, INPUT);

  if (InterruptPin != D0)
  {
    Serial.println("Attach interrupt to Pin");
    attachInterrupt(digitalPinToInterrupt(InterruptPin), checkInputs, RISING);
  }
  
  // Initial Check of Input Modules
  Serial.println("Checking Inputs");
  checkInputs();
}

////////////////////////////////////////////////////////////////////////////////////////
// LOOP

void loop()
{
  ArduinoOTA.handle(); // Wait for OTA connection
  
  Telnet();  // Handle telnet connections

  WebServer.handleClient();    // Handle incoming web requests

  SoftInterrupt();
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Open Loxone URL

void OpenLoxoneURL(String URL)
{
  // Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  HTTPClient http;

  http.begin("http://" + LoxoneIP.toString() + "/dev/sps/io" + URL);
  http.setAuthorization(LoxoneAuthorization);

  // Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0)
  {
      // HTTP header has been send and Server response header has been handled
      // Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK)
      {
          String payload = http.getString();
          Serial.println(payload);
      }
  }
  else
  {
      // Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Webserver handling

void handleRoot()
{
  String P_Root = P_Header() + P_Menu() + P_Footer(); 
  WebServer.send(200, "text/html", P_Root);
}

void handleInput()
{
  String P_Root = P_Header() + P_Menu() + P_Input() + P_Footer(); 
  WebServer.send(200, "text/html", P_Root);
}

void handleOutput()
{
  if (WebServer.hasArg("module"))
  {
    SetOutput(WebServer.arg("module").toInt(),WebServer.arg("output").toInt(),WebServer.arg("value").toInt());
  }
  
  String P_Root = P_Header() + P_Menu() +  P_Output() + P_Footer(); 
  WebServer.send(200, "text/html", P_Root);
}

void ArgCheck()
{
  String message = "";
  message += "<h1 style=\"font-family:courier; text-align:center\">WLAN IR Modul</h1>";
  
  if (WebServer.arg("module") == "")  //Parameter not found
  {
    message += "Error: module Argument missing <br>";
  }

  if (WebServer.arg("output") == "")  //Parameter not found
  {
    message += "Error: output Argument missing<br>";
  }
  
  if (WebServer.arg("command") == "")  //Parameter not found
  {
    message += "Error: command Argument missing<br>";
  }

  SetOutput(WebServer.arg("module").toInt(),WebServer.arg("output").toInt(),WebServer.arg("command").toInt());
  
  delay(5);
  WebServer.send(200, "text/html", message);          //Returns the HTTP response
}

////////////////////////////////////////////////////////////////////////////////////////
// Function to set Output Modul

void SetOutput(int module, int output, bool value)
{
  byte com;
  Wire.requestFrom(module, 1);    // Ein Byte (= 8 Bits) vom PCF8574 lesen
  while(Wire.available() == 0);         // Warten, bis Daten verfügbar 
  com = Wire.read();
  bitWrite(com, output, !value);
  Wire.endTransmission();
  
  Wire.beginTransmission(module);     //Begin the transmission to PCF8574 (0,0,0)
  Wire.write(com);
  Wire.endTransmission();         //End the Transmission

  O_Module_VAL[GetModuleIndex(module)][output] = value; // Store value
  
  TelnetMsg("OUT | 0x" + String(module, HEX) + " | P" + String(output+1) + " | " + (value ? "ON " : "OFF")); // Send Telnet Message
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Get Module Number (for multi array)

int GetModuleIndex(int address)
{
  int index;
  
  for (int i = 0; i < O_Module_NUM; i++)
  {
    if (address == O_Module_Address[i])
    {
      index = i;
      break;
    }
  }
  
  return index;
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Send UDP

void sendUDP(String text)
{
    UDP.beginPacket(LoxoneIP, RecipientPort);
    // Udp.write("Test");
    UDP.print(text);
    UDP.endPacket();
    delay(5);
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Check Wifi Status

void CheckWifiStatus()
{
  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  Serial.print("Waiting for wireless connection ");
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    delay(500);
    Serial.print(".");
      digitalWrite(D4, 1);
      delay(50);
      digitalWrite(D4, 0);
  }
  Serial.println();
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    digitalWrite(D4, 1);
    delay(5000);
    ESP.restart();
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Interrupt

void SoftInterrupt()
{
  if (InterruptPin == D0)
  {
    if (digitalRead(InterruptPin) == 0) // Check PIN D0
    {
      checkInputs();
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////
// Function - Check all Input Modules

void checkInputs()
{
  for (int i = 0; i < I_Module_NUM; i++)
  {
    Wire.requestFrom(I_Module_Address[i], 1);    // Ein Byte (= 8 Bits) vom PCF8574 lesen
    while(Wire.available() == 0);         // Warten, bis Daten verfügbar 

      byte I_Module_VAL_OLD = I_Module_VAL[i];
      I_Module_VAL[i] = Wire.read(); // Set new input value

      if (I_Module_VAL_OLD != I_Module_VAL[i])
      {
        //Serial.print("Input change on module ");Serial.println(i);

        for(int j = 0; j < 8; j++)
        {
          //Serial.print(!(bitRead(I_Module_VAL[i], j)));
          if (bitRead(I_Module_VAL_OLD, j) != bitRead(I_Module_VAL[i], j))
          {
            //Serial.print("Input change on pin ");Serial.println(j);
            
            bool NewInputValue = bitRead(I_Module_VAL[i], j);
            
            //Serial.print("New value");Serial.println(NewInputValue);
            SendChange(i,j,NewInputValue);
          }
        }
      }
/*
      Serial.println(255 - I_Module_VAL[i]);
      
      for(int j = 0; j < 8; j++)
      {
        Serial.print(!(bitRead(I_Module_VAL[i], j)));
      }
      Serial.println();
*/
  }
}

void SendChange(int module, int port, bool value)
{
  TelnetMsg("IN  | 0x" + String(I_Module_Address[module], HEX) + " | P" + String(port+1) + " | " + (value ? "ON " : "OFF") + " | " + I_Module_DESC[module][port] + " | UDP 020" + String(port+1) + value);
  sendUDP("0" + String(I_Module_Address[module]) + String(value+1) + value);
}

////////////////////////////////////////////////////////////////////////////////////////
