/*
 * Version: 1.6
 * Author: Stefan Nikolaus
 * Blog: www.nikolaus-lueneburg.de
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Header
String P_Header(){
  String page = "<html lang='de'><head><meta charset='utf-8' content='width=device-width, initial-scale=1'/>";
  page += "<link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css'><script src='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js'></script><script src='https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js'></script>";
  page += "<title>" Sketch_Name "</title></head><body>";
  page += "<style></style>";
  page += "<div id='content'><div class='container-fluid'>";
  page +=     "<div class='row justify-content-center mt-2'><h2>" Sketch_Name "</h2></div>";
  return page;
}

String P_Header_Small(){
  String page = "<html lang='de'><head><title>" Sketch_Name "</title></head><body>";
  return page;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Menu

String P_Menu(){
  String page =   "<ul class='nav nav-pills'>";
  page +=         "<li class='nav-item mr-2'><a class='nav-link active' href='/input'>Input</a></li>";
  page +=         "<li class='nav-item mr-2'><a class='nav-link active' href='/output'>Output</a></li>";
  page +=         "<li class='nav-item mr-2'><a class='nav-link active' href='/status'>Status</a></li>";
  page +=         "</ul><br>";
  return page;
}

String P_Favorite(){
  String page = "<!-- Start Favorite Loop -->";
  return page;
}
  
////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Input Menu

String P_Input()
{
  String page = "<!-- Start Input Loop -->";
  
  for(int i = 0; i < I_Module_NUM; i++)
  {
    page +=  "<div class='card shadow mb-4'>";

    // Card Header
    page +=  "<h4 class='card-header'>Input Module 0x";    
    page +=    String(I_Module_Address[i], HEX);
    page +=  "</h4>";

    // Card Body
    page +=  "<div class='card-body'>";
    
    for(int j = 0; j < 8; j++)
    {
      bool value = bitRead(I_Module_VAL[i], j); // Load value for button
      bool empty_DESC = I_Module_DESC[i][j].length() == 0; // Check if descripton is empty
      
      if (!show_empty && empty_DESC)
      {
        continue; // Nothing to show, jump to the next in loop
      }
      else
      {
        page +=         "<div class='row'>";
        page +=         "<div class='col col-lg-8'><h4 class ='text-left'><span class='badge badge-pill badge-primary mr-2'>";
        page +=         j+1;
        page +=         "</span>";
        page +=         empty_DESC ? " Not defined" : I_Module_DESC[i][j];
        page +=         (I_Module_INV[i][j]  ? "   <span class='badge badge-pill badge-secondary ml-2'>INV</span>" : "");        
        page +=         "</span></h4></div>";
      }
/*      
      {
        page +=         "<div class='row'><div class='col col-lg-8'><h4 class ='text-left'>";
        page +=         j+1;
        page +=         " - ";
        page +=         empty_DESC ? " Not defined" : I_Module_DESC[i][j];
        page +=         (I_Module_INV[i][j]  ? "   <span class='badge badge-pill badge-secondary ml-2'>INV</span>" : "");        
        page +=         "</span></h4></div>";
      }
*/
      if (I_Module_INV[i][j] ? value : !value)
      {
        page +=         "<div class='col col-lg-4'><form><button type='button submit' name=";
        page +=         j;
        page +=         " value='1' class='btn btn-success btn-lg'>ON</button></form></div>";
      }
      else
      {
        page +=         "<div class='col col-lg-4'><form><button type='button submit' name=";
        page +=         j;
        page +=         " value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";
      }
      page +=       "</div>";
    }
    page +=       "</div></div>";
  }

  page +=       "<!-- End Input Loop -->";
  
  return page;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Output Menu

String P_Output()
{
  String page = "<!-- Start Output Loop -->";
  for(int i = 0; i < O_Module_NUM; i++)
  {
    page +=  "<div class='card shadow mb-4'>";

    // Card Header
    page +=  "<h4 class='card-header'>Output Module 0x";    
    page +=    String(O_Module_Address[i], HEX);
    page +=  "</h4>";

    // Card Body
    page +=  "<div class='card-body'>";
    
    for(int j = 0; j < 8; j++)
    {
      bool empty_DESC = O_Module_DESC[i][j].length() == 0; // Check if descripton is empty
      if (!show_empty && empty_DESC)
      {
        continue; // Nothing to show, jump to the next in loop
      }
      else
      {
        page +=         "<div class='row'>";
        page +=         "<div class='col col-lg-6'><h4 class ='text-left'><span class='badge badge-pill badge-primary mr-2'>";
        page +=         j+1;
        page +=         "</span>";
        page +=         empty_DESC ? "Not defined" : O_Module_DESC[i][j];
        page +=         "   <span class='badge badge-pill badge-secondary ml-2'>";
        page +=         (O_Module_VAL[i][j] ? "ON" : "OFF");
        page +=         "</span></h4></div>";
      }

      // Button ON
      page +=         "<div class='col col-lg-4'><form action='/output' method='POST'><input type='hidden' name='module' value='";
      page +=         String(O_Module_Address[i], DEC);
      page +=         "'><input type='hidden' name='set' value='1'><input type='hidden' name='out' value='";
      page +=         j;
      page +=         "'><button type='button submit' name='value' value='1' class='btn btn-success btn-lg mr-5'>ON</button>";
      // Button OFF
      page +=         "<input type='hidden' name='module' value='";
      page +=         String(O_Module_Address[i], DEC);
      page +=         "'><input type='hidden' name='set' value='1'><input type='hidden' name='out' value='";
      page +=         j;
      page +=         "'><button type='button submit' name='value' value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";

      page +=       "</div>";
    }
    page +=       "</div></div>";
  }
  page +=       "<!-- End Output Loop -->";
  return page;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Input Menu

String P_Status()
{
  String page = "<!-- Start Status Loop -->";

  page +=   "<div class='card shadow mb-4'>";
  page +=   "  <h4 class='card-header'>Wifi</h4>";
  page +=   "  <div class='card-body'>";
  page +=   "    <p class='card-text'>";
  page +=         "SSID: ";
  page +=         ssid;
  page +=         "<br>";
  page +=         "RSSI: ";
  page +=         WiFi.RSSI();
  page +=         "</p>";  
  page +=   "  </div>";
  page +=   "</div>";
  
if (mqttEnabled) {
  page +=   "<div class='card shadow mb-4'>";
  page +=   "  <h4 class='card-header'>MQTT</h4>";
  page +=   "  <div class='card-body'>";
  page +=   "    <p class='card-text'>";
  page +=         "Status: ";
    if (client.connected()) {
      page +=         "Online";
    }
    else {
      page +=         "Offline";
    }
  page +=         "<br>";
  page +=         "Basetopic: ";
  page +=         mqttBaseTopic;
  page +=         "</p>";
  page +=   "  </div>";
  page +=   "</div>";
}

if (udpEnabled) {
  page +=   "<div class='card shadow mb-4'>";
  page +=   "  <h4 class='card-header'>UDP</h4>";
  page +=   "  <div class='card-body'>";
  page +=   "    <p class='card-text'>";
  page +=         "Server IP: ";
  page +=         LoxoneIP.toString();
  page +=         "<br>";
  page +=         "Port: ";
  page +=         RecipientPort;
  page +=         "</p>";
  page +=   "  </div>";
  page +=   "</div>";
}

  page +=   "<div class='card shadow mb-4'>";
  page +=   "  <h4 class='card-header'>System</h4>";
  page +=   "  <div class='card-body'>";
  page +=   "    <p class='card-text'>";
  page +=      "<b>Sketch</b><br>";
  page +=      "Total size: ";
  page +=       ESP.getSketchSize();
  page +=       " kb";
  page +=       "<br>";  
  page +=       "Free size: ";
  page +=       ESP.getFreeSketchSpace();
  page +=       " kb";
  page +=       "<br>";
  page +=      "<b>Memory</b><br>";
  page +=       "Free heap: ";
  page +=       ESP.getFreeHeap();
  page +=       " kb";
  page +=       "<br>";  
  page +=       "Heap fragmentation: ";
  page +=       ESP.getHeapFragmentation();
  page +=       "%";
  page +=     "</p>";
  page +=   "  </div>";
  page +=   "</div>";

  page +=         "<!-- End Status Loop -->";
  
  return page;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Footer

String P_Footer()
{
  String page = "<!-- Start Footer -->";
  page += "<footer class='sticky-footer bg-white'><div class'container my-auto'>";
  page += "<div class='copyright text-center my-auto'><span><a href='https://www.Nikolaus-Lueneburg.de'>www.Nikolaus-Lueneburg.de</a> - Version " Sketch_Version "</span></div>";
  page += "</div></footer>";
  
  page += "</div></div>";
  page += "</body></html>";  
  
  return page;
}
