/*
 * Version: 1.31
 * Author: Stefan Nikolaus
 * Blog: www.nikolaus-lueneburg.de
 */

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
  delay(5);  // to avoid strange characters left in buffer  
}
