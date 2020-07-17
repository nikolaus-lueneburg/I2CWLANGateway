/*
 * Version: 1.4
 * Author: Stefan Nikolaus
 * Blog: www.nikolaus-lueneburg.de
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sketch
#define Sketch_Name       "I2C WLAN Gateway - Test"    // Name des Skripts
#define Sketch_Version    "1.6"                  // Version des Skripts

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Network

// WLAN Configuration
const char* ssid = "<MYSSID>";
const char* password = "<WLANSECRET>";

// DHCP or Static
const bool EnableStaticIP = true; // false = DHCP

// ESP8266 IP Configuration
IPAddress ip(192,168,1,123); // IP-Address Wemos D1
IPAddress dns(192,168,1,1); // IP-Address DNS
IPAddress gateway(192,168,1,1); // IP-Address network gateway
IPAddress subnet(255,255,255,0); // Subnetzmaske

////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTA settings
const char* OTAPassword = "1234"; 
const char* OTAHostname = "ESP-Test";

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Telnet settings
const bool telnetEnabled = true;

#define MAX_TELNET_CLIENTS 2

////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDP settings
const bool udpEnabled = true;

// UDP packet destination IP & Port
IPAddress LoxoneIP(192, 168, 1, 5);
const unsigned int RecipientPort = 12345;

// UDP Settings
const unsigned int localUdpPort = 8000;  // local port to listen on
const bool replyUDP = true;
const char  replyPacket[] = "OK";  // a reply string to send back

////////////////////////////////////////////////////////////////////////////////////////////////////////
// MQTT
const bool mqttEnabled = true;

const char* mqttServer = "<Server FQDN>";
const int mqttPort = 8883;

const char* mqttUser = "<USERNAME>";
const char* mqttPassword = "<PASSWORD>";
const char* mqttClientID = "<CLIENTID>";

const char mqttBaseTopic[] = "<BASETOPIC>"; // Example "I2CGW/TEST/"
const char mqttLWtopic[] = "status";


////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupt Pin
#define InterruptPin D0

// Horter I2C WLAN Modul Prototype - fixed to D0 (Software Interrupt)
// Horter I2C WLAN Modul Final Version - Change with jumper to D0 D3 D4 D8

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loxone
// See function OpenLoxoneURL for further details

const char* LoxoneAuthorization = "<BASE64STRING>"; // Base64 encoded Username and Password (user:password)
const char* LoxoneRebootURL = "I2CGW_TEST"; // Opens the URL after a reboot

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Configuration

const bool show_empty = true; // If false, hide not definded ports in HTML view

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging

const bool loggingEnabled = true; // If true, HTML log was activated
const byte logLength = 10;

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Output Module Configuration

const int O_Module_Address[] = {0x21,56}; // HEX or DEC address from output module 0x20 = 32 / 0x21 = 33 / ... / 0x38 = 56 / ...

const String O_Module_DESC[][8] = // Description for each module and port (null or empty means "Not in use")
{
  {"Licht","Steckdose","Ladestation","Drehstrom","","","","Test"},
  {"Ventil 1","Ventil 2","Ventil 3","Ventil 4","Ventil 5","Ventil 6","Ventil 7","Ventil 8"}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input Module Configuration

const int I_Module_Address[] = {0x20}; // HEX or DEC address from input module 0x20 = 32 / 0x21 = 33 / ... / 0x38 = 56 / ...

const String I_Module_DESC[][8] = // Description for each module and port (null or empty means "Not in use")
{
  {"Taster-Licht","Bewegungsmelder","Taster-Aussen","","","","","Test"}
};

const bool I_Module_INV[][8] = // Invert Input - true inverts the input value
{
  {false,true,false,false,false,false,false,true}
};
