////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sketch
#define Sketch_Name       "I2C WLAN Gateway - Test"    // Name des Skripts
#define Sketch_Version    "1.26"                  // Version des Skripts

////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupt Pin
const int InterruptPin = D0;

// Horter I2C WLAN Modul Prototype - fixed to D0 (Software Interrupt)
// Horter I2C WLAN Modul Final Version - Change with jumper to D0 D3 D4 D8

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loxone
// See function OpenLoxoneURL for further details

const char* LoxoneAuthorization = "ABCDEF1234567890"; // Base64 encoded Username and Password
const char* LoxoneRebootURL = "/21_GARAGE/pulse"; // Opens the URL after a reboot

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Output Module Configuration

int O_Module_Address[] = {0x21,56}; // HEX or DEC address from module

const String O_Module_DESC[][8] = // Description for each module and port (null or empty means "Not in use")
{
  {"Licht","Steckdose","Ladestation","Drehstrom","","","","Test"},
  {"Ventil 1","Ventil 2","Ventil 3","Ventil 4","Ventil 5","Ventil 6","Ventil 7","Ventil 8"}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input Module Configuration

int I_Module_Address[] = {0x20}; // HEX or DEC address from Module

const String I_Module_DESC[][8] = // Description for each module and port (null or empty means "Not in use")
{
  {"Taster-Licht","Bewegungsmelder","Taster-Aussen","","","","","Test"}
};

const bool I_Module_INV[][8] = // Invert Input
{
  {false,true,false,false,false,false,false,true}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Configuration

bool show_empty = false; // Show not definded ports in HTML view



