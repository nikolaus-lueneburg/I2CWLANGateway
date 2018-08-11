#define Sketch_Name       "I2C WLAN Gateway - Werkstatt"    // Name des Skripts
#define Sketch_Version    "1.12"                  // Version des Skripts

////////////////////////////////////////////////////////////////////////////////////////
// Libraries

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PCF8574.h> //
#include <ESP8266HTTPClient.h>
#include <Wire.h>

////////////////////////////////////////////////////////////////////////////////////////
// Settings

////////////////////////////////
// Network

// WLAN Configuration
const char* ssid = "MYSSID";
const char* password = "WLANSECRET";

// ESP8266 IP Configuration
IPAddress ip(192,168,1,21); // IP-Address Wemos D1
IPAddress gateway(192,168,1,1); // IP-Address network gateway
IPAddress subnet(255,255,255,0); // Subnetzmaske

// UDP packet destination IP & Port
IPAddress LoxoneIP(192, 168, 1, 5);
unsigned int RecipientPort = 12345;

// Telnet
#define MAX_TELNET_CLIENTS 2
WiFiServer TelnetServer(23); // Telnet Port

// Web-Server
ESP8266WebServer WebServer(80); // HTTP Port

////////////////////////////////
// Input Module

// Interrupt Pin (Horter I2C WLAN Modul Prototype Modul is connected to D0 - So currently no real interrupt)
#define InterruptPin D0

// 0x20
PCF8574 PCF_20(0x20);  // Input Module with address 0x20

const int Input20_INV[3] = {false,true,false}; // Invert Input
bool Input20_VAL[3] = {0,0,0}; // Flag for input state
const String Input20_DESC[3] = {"Taster-Licht","Bewegungsmelder","Taster-Aussen"}; // Input Description

const int Input20_NUM = sizeof(Input20_VAL);

////////////////////////////////
// Output Module

// 0x21
PCF8574 PCF_21(0x21);  // Output Module with address 0x21

bool Output21_VAL[4] = {0,0,0,0}; // Flag for output state
const String Output21_DESC[4] = {"Licht","Steckdose","Ladestation","Drehstrom"}; // Output description

const int Output21_NUM = sizeof(Output21_VAL);

////////////////////////////////
// Loxone
// See function OpenLoxoneURL for further details

const char* LoxoneAuthorization = "ABCDEF1234567890"; // Base64 encoded Username and Password
const char* LoxoneRebootURL = "/21_GARAGE/pulse"; // Opens the URL after a reboot

WiFiClient TelnetClient[MAX_TELNET_CLIENTS];
bool ConnectionEstablished; // Flag for telnet

// Start UDP
WiFiUDP UDP;

////////////////////////////////////////////////////////////////////////////////////////
// Setup

void setup()
{
  Serial.begin(115200);
  Serial.println(Sketch_Name);
  Serial.println(Sketch_Version);
  
  pinMode(D4, OUTPUT); 

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

  // PCF8574
  PCF_20.begin();
  PCF_21.begin();

  WebServer.on("/set", ArgCheck); // Handle HTTP GET command incoming
  WebServer.on("/", handleRoot); // Handle root website
  WebServer.on("/input", handleInput); // Handle Input website
  WebServer.on("/output", handleOutput); // Handle Output website

  // Start the server
  WebServer.begin();
  Serial.println("WebServer started");

  OpenLoxoneURL(LoxoneRebootURL);

  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(InterruptPin, INPUT);

  // Initial Check of Input Modules
  checkInputs();
}

////////////////////////////////////////////////////////////////////////////////////////
// LOOP

void loop()
{
  ArduinoOTA.handle(); // Wait for OTA connection
  
  Telnet();  // Handle telnet connections

  WebServer.handleClient();    // Handle incoming web requests

  checkInterrupt(); // Check Interrupt
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
  if (WebServer.hasArg("board"))
  {
    SetOutput(WebServer.arg("board").toInt(),WebServer.arg("output").toInt(),WebServer.arg("value").toInt());
  }
  
  String P_Root = P_Header() + P_Menu() +  P_Output() + P_Footer(); 
  WebServer.send(200, "text/html", P_Root);
}

void ArgCheck()
{
  String message = "";
  message += "<h1 style=\"font-family:courier; text-align:center\">WLAN IR Modul</h1>";
  
  if (WebServer.arg("board") == "")  //Parameter not found
  {
    message += "Error: board Argument missing <br>";
  }

  if (WebServer.arg("output") == "")  //Parameter not found
  {
    message += "Error: output Argument missing<br>";
  }
  
  if (WebServer.arg("command") == "")  //Parameter not found
  {
    message += "Error: command Argument missing<br>";
  }

  SetOutput(WebServer.arg("board").toInt(),WebServer.arg("output").toInt(),WebServer.arg("command").toInt());
  
  delay(5);
  WebServer.send(200, "text/html", message);          //Returns the HTTP response
}

void SetOutput(int board, int output, bool value)
{
  if (board == 21)
  {
    PCF_21.write(output, !value);
    Output21_VAL[output] = value;
  }

  TelnetMsg("OUT" + String(board) + " | P" + String(output+1) + " | " + (value ? "ON " : "OFF")); // Send Telnet Message

}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Telnet Service

void Telnet()
{
  // Cleanup disconnected session
  for(int i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] && !TelnetClient[i].connected())
    {
      Serial.print("Client disconnected ... terminate session "); Serial.println(i+1); 
      TelnetClient[i].stop();
    }
  }
  
  // Check new client connections
  if (TelnetServer.hasClient())
  {
    ConnectionEstablished = false; // Set to false
    
    for(int i = 0; i < MAX_TELNET_CLIENTS; i++)
    {
      // Serial.print("Checking telnet session "); Serial.println(i+1);
      
      // find free socket
      if (!TelnetClient[i])
      {
        TelnetClient[i] = TelnetServer.available(); 
        
        Serial.print("New Telnet client connected to session "); Serial.println(i+1);

        TelnetClient[i].flush();  // clear input buffer, else you get strange characters
        TelnetClient[i].println(Sketch_Name);
        TelnetClient[i].print("Welcome! Session #");
        TelnetClient[i].println(i+1);
        TelnetClient[i].print("Version: ");
        TelnetClient[i].println(Sketch_Version);
        TelnetClient[i].print("Free Heap RAM: ");
        TelnetClient[i].println(ESP.getFreeHeap());
        TelnetClient[i].println("----------------------------------------------------------------");
        
        ConnectionEstablished = true; 
        
        break;
      }
      else
      {
        // Serial.println("Session is in use");
      }
    }

    if (ConnectionEstablished == false)
    {
      Serial.println("No free sessions ... drop connection");
      TelnetServer.available().stop();
      // TelnetMsg("An other user cannot connect ... MAX_TELNET_CLIENTS limit is reached!");
    }
  }

  for(int i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] && TelnetClient[i].connected())
    {
      if(TelnetClient[i].available())
      { 
        //get data from the telnet client
        while(TelnetClient[i].available())
        {
          Serial.write(TelnetClient[i].read());
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Telnet Message

void TelnetMsg(String text)
{
  for(int i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] || TelnetClient[i].connected())
    {
      TelnetClient[i].println(text);
    }
  }
  delay(10);  // to avoid strange characters left in buffer  
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Send UDP

void sendUDP(String text)
{
    UDP.beginPacket(LoxoneIP, RecipientPort);
    // Udp.write("Test");
    UDP.print(text);
    UDP.endPacket();
    delay(10);
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
// Function - Check Interrupt

void checkInterrupt()
{
  if (digitalRead(InterruptPin) == 0) // Check PIN D0
  {
    TelnetMsg("INTERRUPT"); 
    checkInputs(); // Check all Input Modules
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Check Input Modules

void checkInputs() // Add aditional Input Modules
{
  check20();
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Check Input Module 0x20

void check20()
{
  for(int i = 0; i < Input20_NUM; i++)
  {
    if(!PCF_20.readButton(i))
    {
      if(Input20_VAL[i] == 0)
      {
        Input20_VAL[i] = 1;
        send20(i);
      }
    }
    else if (Input20_VAL[i] == 1)
    {
      Input20_VAL[i] = 0;
      send20(i);
    }
  }
}

void send20(int input)
{
  TelnetMsg("IN20-" + String(input+1) + " | " + (Input20_VAL[input] ? "ON " : "OFF") + " | " + Input20_DESC[input] + " | UDP 020" + String(input+1) + Input20_VAL[input]);
  sendUDP("020" + String(input+1) + Input20_VAL[input]);
}

////////////////////////////////////////////////////////////////////////////////////////
