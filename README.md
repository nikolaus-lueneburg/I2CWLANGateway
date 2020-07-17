# I2CWLANGateway
I2C WLAN Gateway with ESP8266 and I/O modules from www.Horter-Shop.de

For further details visit www.nikolaus-lueneburg.de

# Features
- Web-GUI
- MQTT
- HTTP API
- UDP Send - Input Module
- UDP Receive - Output Module
- Over the Air Update
- Telnet Log
- Interrupt
- Status page

# Output Module
Example: To change output 3 to ON on module with HEX 0x21 or DEC 33

## HTTP API ##

http://<ESP_IP>/set?module=0x21&out=3&value=1

or 

http://<ESP_IP>/set?module=33&out=3&value=1

## UDP ##

Send UDP value ==> 02131

## MQTT ##

<BASETOPIC>/21/set/1 - on

# Input Module
Example: Output 2 changes from 0 to 1 on module with HEX 0x20 or DEC 32

## HTTP API ##

Coming soon

## UDP ##

I2CWLANGateway send value ==> 02021

## MQTT ##

<BASETOPIC>/20/2 - on
