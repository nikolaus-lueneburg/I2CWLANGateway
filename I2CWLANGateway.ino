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
byte I_Module_VAL_NEW[I_Module_NUM];  // Flag for input state

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

  // WebServer.on("/set", setOutput); // Handle HTTP GET command incoming
  WebServer.on("/", handleRoot); // Handle root website
  WebServer.on("/input", handleInput); // Handle Input website
  WebServer.on("/output", handleOutput); // Handle Output website

  // Start the server
  Serial.println("Starting WebServer");
  WebServer.begin();

  // OpenLoxoneURL(LoxoneRebootURL);

  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(InterruptPin, INPUT);

  if (InterruptPin != D0)
  {
    Serial.println("Attach interrupt to Pin");
    attachInterrupt(digitalPinToInterrupt(InterruptPin), readInputs, RISING);
  }
  
  // Initial Check of Input Modules
  Serial.println("Checking Inputs");
  readInputs();
}

////////////////////////////////////////////////////////////////////////////////////////
// LOOP

void loop()
{
  ArduinoOTA.handle(); // Wait for OTA connection
  
  Telnet();  // Handle telnet connections

  WebServer.handleClient();    // Handle incoming web requests

  softInterrupt();

  checkInputs();
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
  String page;
  String message1 = "";
  String message2 = "";
  bool error = false;
  bool gui = WebServer.arg("gui").toInt();
  bool set = WebServer.arg("set").toInt();
  int module = WebServer.arg("module").toInt();
  int out = WebServer.arg("out").toInt();
  bool value = WebServer.arg("value").toInt();

  if (gui || set)
  { 
    if (set)
    {
      out = out-1; // Adjust out
      
      Serial.println("Check Parameter");
      if (GetModuleIndex(module) == 99)
      {
        message2 += "<h3><font color='red'>Error module</font></h3>";
        error = true;
      }
      if (out >= 8)
      {
        message2 += "<h3><font color='red'>Error out</font></h3>";
        error = true;
      }
      if (!(value >= 0 && value <= 1))
      {
        message2 += "<h3><font color='red'>Error value</font></h3>";
        error = true;
      }
    }

    if (error)
    {
      message1 += "<h2><font color='red'>Error</font></h2>";
    }
    else
    {    
      message1 += "<h2><font color='green'>OK</font></h2>";
    }
    
    if (!error)
    {
      Serial.println("SetOutput");
      SetOutput(module,out,value);
      delay(5);
    }
  }

  if (set)
  {
    page = P_Header_Small() + message1  + message2;
  }
  else
  {
    page = P_Header() + P_Menu() + P_Output() + P_Footer();
  }
  WebServer.send(200, "text/html", page);
}


////////////////////////////////////////////////////////////////////////////////////////
// Function to set Output Modul

void SetOutput(int module, int out, bool value)
{
  byte com;
  Wire.requestFrom(module, 1);    // Ein Byte (= 8 Bits) vom PCF8574 lesen
  while(Wire.available() == 0);         // Warten, bis Daten verfügbar 
  com = Wire.read();
  bitWrite(com, out, !value);
  Wire.endTransmission();
  
  Wire.beginTransmission(module);     //Begin the transmission to PCF8574 (0,0,0)
  Wire.write(com);
  Wire.endTransmission();         //End the Transmission

  O_Module_VAL[GetModuleIndex(module)][out] = value; // Store value
  
  TelnetMsg("OUT | 0x" + String(module, HEX) + " | P" + String(out+1) + " | " + (value ? "ON " : "OFF")); // Send Telnet Message
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Get Module Number (for multi array)

int GetModuleIndex(int address)
{
  int index = 99;
  
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
  UDP.print(text);
  UDP.endPacket();
  delay(5);
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - softInterrupt

void softInterrupt()
{
  if (InterruptPin == D0)
  {
    if (digitalRead(InterruptPin) == 0) // Check PIN D0
    {
      readInputs();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - readInputs - Check all Input Modules

void readInputs()
{
  for (int i = 0; i < I_Module_NUM; i++)
  {
    Wire.requestFrom(I_Module_Address[i], 1);    // Ein Byte (= 8 Bits) vom PCF8574 lesen
    while(Wire.available() == 0);         // Warten, bis Daten verfügbar 

    I_Module_VAL_NEW[i] = Wire.read(); // Set new input value
  }
}  

////////////////////////////////////////////////////////////////////////////////////////
// Function - readInputs - Check all Input Modules

void checkInputs()
{
  for (int i = 0; i < I_Module_NUM; i++)
  {
    if (I_Module_VAL_NEW[i] != I_Module_VAL[i])
    {
      for(int j = 0; j < 8; j++)
      {
        if (bitRead(I_Module_VAL_NEW[i], j) != bitRead(I_Module_VAL[i], j))
        {
          bool NewValue = bitRead(I_Module_VAL[i], j);
          SendChange(i,j,NewValue);
        }
      }
    }
    I_Module_VAL[i] = I_Module_VAL_NEW[i]; // Store new value
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - sendChange - Send telnet and UDP message

void SendChange(int module, int out, bool value)
{
  TelnetMsg("IN  | 0x" + String(I_Module_Address[module], HEX) + " | P" + String(out+1) + " | " + (value ? "ON " : "OFF") + " | " + I_Module_DESC[module][out] + " | UDP 020" + String(out+1) + value);
  sendUDP("0" + String(I_Module_Address[module]) + String(out+1) + value);
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Open Loxone URL

void OpenLoxoneURL(String URL)
{
  // Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  HTTPClient http;

  http.begin("http://" + LoxoneIP.toString() + "/dev/sps/io/" + URL + "/pulse");
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
