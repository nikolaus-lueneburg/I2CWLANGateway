/*
 * Version: 1.4
 * Author: Stefan Nikolaus
 * Blog: www.nikolaus-lueneburg.de
 */

///////////////////////////////////////////////////////////////////////////////////////
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

char incomingPacket[255];  // buffer for incoming packets

// Logging
String loggingArray[logLength] = {};
byte logPointer = 0;

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
  if (EnableStaticIP)
  {
    WiFi.config(ip, gateway, subnet);
  }
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
  ArduinoOTA.setHostname(OTAHostname);

  ArduinoOTA.setPassword(OTAPassword);
  
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
  
  WebServer.on("/", httpHandleRoot); // Handle root website
  WebServer.on("/input", httpHandleInput); // Handle Input website
  WebServer.on("/output", httpHandleOutput); // Handle Output website
  WebServer.on("/status", httpHandleStatus); // Handle Status website  
  WebServer.on("/set", httpHandleSet); // Handle incoming (HTTP GET) set command
  WebServer.on("/get", httpHandleGet); // Handle incoming (HTTP GET) get command  
  WebServer.onNotFound(httpHandleNotFound); // When a client requests an unknown URI

  // Start the server
  Serial.println("Starting WebServer");
  WebServer.begin();

  // Start UDP listener
  UDP.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  // sendHTTPGet(LoxoneRebootURL);

  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(InterruptPin, INPUT);

  if (InterruptPin != D0)
  {
    Serial.println("Attach interrupt to Pin");
    attachInterrupt(digitalPinToInterrupt(InterruptPin), readInputs, RISING);
  }

  // Initial Check of Input Modules
  Serial.println("Checking Inputs");

  I2CScan();

  readInputs();

  updateOutput();

LogMsg("Startup completed");

}

////////////////////////////////////////////////////////////////////////////////////////
// LOOP

void loop()
{
  ArduinoOTA.handle(); // Wait for OTA connection
  
  Telnet();  // Handle telnet connections

  WebServer.handleClient();    // Handle incoming web requests

  // Invokes the "read Input" function if software interrupt is used
  softInterrupt();

  // Checks for changes on the input modules
  checkInputs();

  // looks for new UDP packages
  receiveUDP();

}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Webserver handling

void httpHandleNotFound()
{
  WebServer.send(404, "text/plain", "404: Page not found");
}

void httpHandleRoot()
{
  String P_Root = P_Header() + P_Menu() + P_Favorite() + P_Footer(); 
  WebServer.send(200, "text/html", P_Root);
}

void httpHandleInput()
{
  String P_Root = P_Header() + P_Menu() + P_Input() + P_Footer(); 
  WebServer.send(200, "text/html", P_Root);
}
/*
unsigned int hexToDec(String hexString) {
  
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}
*/

unsigned int hexToDec(String hexString) {
    unsigned int decValue = 0;
    for(int i=0, n=hexString.length(); i!=n; ++i) {
        char ch = hexString[i];
        int val = 0;
        if('0'<=ch && ch<='9')      val = ch - '0';
        else if('a'<=ch && ch<='f') val = ch - 'a' + 10;
        else if('A'<=ch && ch<='F') val = ch - 'A' + 10;
        else continue; // skip non-hex characters
        decValue = (decValue*16) + val;
    }
    return decValue;
}

void httpHandleOutput()
{
  String htmlHead;
  String message1 = "";
  String message2 = "";
  bool error = false;
  bool set = WebServer.arg("set").toInt();
  int module = WebServer.arg("module").toInt();
  int out = WebServer.arg("out").toInt();
  bool value = WebServer.arg("value").toInt();

  if (set)
  {
    SetOutput(module,out,value);
    delay(5);
  }

  htmlHead= P_Header() + P_Menu();
  // htmlHead= P_Header() + P_Menu() + P_Output() + P_Footer();
  
  WebServer.setContentLength(htmlHead.length() + P_Output().length() + P_Footer().length());
  WebServer.send(200,"text/html",htmlHead);
  WebServer.sendContent(P_Output());
  WebServer.sendContent(P_Footer());

  //WebServer.send(200, "text/html", page);
}

void httpHandleStatus()
{
  String httpHandleStatus = P_Header() + P_Menu() + P_Status() + P_Footer(); 
  WebServer.send(200, "text/html", httpHandleStatus);
}

void httpHandleGet() // Not implemented yet
{
  WebServer.send(404, "text/plain", "404: Page not found");
}

void httpHandleSet()
{
  String page;
  String message1 = "";
  String message2 = "";
  bool error = false;
  int module;
  //String modulearg;
  int out = WebServer.arg("out").toInt();
  bool value = WebServer.arg("value").toInt();

//  LogMsg((WebServer.arg("module"))); // Send Telnet Message

  if (WebServer.arg("module").startsWith("0x"))
  {
    module = hexToDec(WebServer.arg("module").substring(2));
  }
  else
  {
    module = WebServer.arg("module").toInt();
  }

  // Check "module"
  if (GetModuleIndex(module) == 99)
  {
    message2 += "Error: Module not found";
    message2 += "<br>";
    message2 += module;
    message2 += "<br>";
    message2 += WebServer.arg("module");
    error = true;
  }
  else
  {
/*
    message2 += "Module HEX 0x";
    message2 += String (module, HEX);  
    message2 += " - DEC ";
    message2 += module;
*/
  }

  // Check "out"
  out = out-1; // Adjust out to 0-7
  if (out > 7)
  {
    message2 += "Error: Variable 'out' is out of range";
    error = true;
  }

  // Check "value"
  if (!(value >= 0 && value <= 1))
  {
    message2 += "Error: Variable value must be 0 or 1";
    error = true;
  }

  if (error)
  {
    message1 += "Error";
  }
  else
  {    
    message1 += "OK";
    SetOutput(module,out,value);
  }
  delay(5);
  
  page = P_Header_Small() + message1  + message2;
  WebServer.send(200, "text/plain", page);
}


////////////////////////////////////////////////////////////////////////////////////////
// Function - Read Module (Input & Output Module)

byte readModule(int module)
{
  Wire.requestFrom(module, 1);    // Ein Byte (= 8 Bits) vom PCF8574 lesen
  while(Wire.available() == 0);         // Warten, bis Daten verfügbar 
  byte value = Wire.read();
  return value;
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Update Output Module

void updateOutput()
{
  for (int i = 0; i < O_Module_NUM; i++)
  {
    byte value = readModule(O_Module_Address[i]);
    for(int j = 0; j < 8; j++)
    {
      O_Module_VAL[i][j] = !bitRead(value, j);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function to set Output Modul

void SetOutput(int module, int out, bool value)
{
  byte com = readModule(module);
  
  bitWrite(com, out, !value); // Change bit value
  
  Wire.beginTransmission(module); // Begin the transmission to PCF8574
  Wire.write(com); // Write new value
  Wire.endTransmission(); // End the Transmission

  O_Module_VAL[GetModuleIndex(module)][out] = value; // Store value
  
  LogMsg("SET OUT | 0x" + String(module, HEX) + " | P" + String(out+1) + " | " + (value ? "ON " : "OFF")); // Log Message
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
  return index; // if return value 99 the module cannout be found
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Send UDP

void sendUDP(String text)
{
  UDP.beginPacket(LoxoneIP, RecipientPort);
  UDP.print(text);
  UDP.endPacket();
  delay(5);
  String logText = "Send UDP Message >" + text + "< to " + LoxoneIP.toString();
  LogMsg(logText);  // Log Message
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Receive UDP

void receiveUDP()
{
  bool error = false;
  
  int packetSize = UDP.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, UDP.remoteIP().toString().c_str(), UDP.remotePort());
    int len = UDP.read(incomingPacket, 255);
    LogMsg(String(len));
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }

    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    String incoming = incomingPacket;

    if (incoming == "99999")
    {
      LogMsg("Rebooting...");
      delay(2000);
      ESP.restart();
    }

    int module = (int) strtol( &incoming.substring(1,3)[0], NULL, 16);
    // int module = incoming.substring(1,3).toInt(); // Switch to use DEC instead of HEX I2C address
    int out = incoming.substring(3,4).toInt();
    int value = incoming.substring(4,5).toInt();

    // Check "module"
    if (GetModuleIndex(module) == 99)
    {
      error = true;
    }

    // Check "out"
    out = out-1; // Adjust out to 0-7
    if (out > 7)
    {
      error = true;
    }

    // Check "value"
    if (!(value >= 0 && value <= 1))
    {
      error = true;
    }
    
    if (error)
    {
      LogMsg("UDP Error");   // Log Message
    }
    else
    {    
      SetOutput(module,out,value);
    }

    if (replyUDP)
    {
      // send back a reply, to the IP address and port we got the packet from
      UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
      UDP.write(replyPacket);
      UDP.endPacket();
    }
  }
}

/*
int PartOfArray(char text)
{
  for (int i = 0 ; i < 5 ; i++) incomingPacket[i] = items[i];
  test[5] = '\0';
  
  return index;
}
*/
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
    I_Module_VAL_NEW[i] = readModule(I_Module_Address[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - readInputs - Check all Input Modules

void checkInputs()
{
  for (int i = 0; i < I_Module_NUM; i++)
  {
    if (I_Module_VAL[i] != I_Module_VAL_NEW[i])
    {
      for(int j = 0; j < 8; j++)
      {
        if (bitRead(I_Module_VAL_NEW[i], j) != bitRead(I_Module_VAL[i], j))
        {
          bool NewValue = bitRead(I_Module_VAL[i], j);
          SendChange(i,j,NewValue);
        }
      }
      I_Module_VAL[i] = I_Module_VAL_NEW[i]; // Store new value
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - sendChange - Send UDP and log message

void SendChange(int module, int out, bool value)
{
  LogMsg("IN  | 0x" + String(I_Module_Address[module], HEX) + " | P" + String(out+1) + " | " + (value ? "ON " : "OFF") + " | " + I_Module_DESC[module][out] + " | UDP 020" + String(out+1) + value);   // Log Message
  sendUDP("0" + String(I_Module_Address[module], HEX) + String(out+1) + value);
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Send HTTP Get to URL

void sendHTTPGet(IPAddress IP, String URL)
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

////////////////////////////////////////////////////////////////////////////////////////
// Function - I2C Scanner

void I2CScan()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("Done!");
  }
  LogMsg("Found " + String(nDevices) + " I2C devices");
}

////////////////////////////////////////////////////////////////////////////////////////
// Function - Telnet Message

void LogMsg(String text)
{
  // Status Message  
  if (loggingEnabled) {
    if (logPointer == (logLength)) {
      logPointer = 0;
    }
    String logText;
    logText += millis();
    logText += "-";
    logText += text;
    loggingArray[logPointer] = logText;
    logPointer++;
  }
  
  // Telnet Message
  for(int i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] || TelnetClient[i].connected())
    {
      TelnetClient[i].println(text);
    }
  }
  delay(1);  // to avoid strange characters left in buffer


}
