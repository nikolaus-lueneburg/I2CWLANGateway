////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Header
String P_Header(){
  String page = "<html lang='fr'><head><meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1'/>";
  page += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'><script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script><script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";
  page += "<title>" Sketch_Name "</title></head><body>";
  page += "<style>#button_desc { float: left; width: 100px;}</style>";
  page += "<div class='container-fluid'>";
  page +=   "<div class='row'>";
  page +=     "<div class='col-md-12'>";
  page +=       "<h1>" Sketch_Name "</h1>";
//  page +=       "<h2>I/O Dashboard</h2>";
  return page;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Menu

String P_Menu(){
  String page =       "<ul class='nav nav-pills'>";
  page +=         "<li class='active'><a href='/input'>Input</a></li>";
  page +=         "<li class='active'><a href='/output'>Output</a></li>";
/*
  page +=         "<li class='active'><a href='#'> <span class='badge pull-right'>";
  page +=         random(10, 20);
  page +=         "</span>Meldungen</a></li>";
*/
  page +=         "</ul>";
  return page;  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Input Menu

String P_Input()
{
  String page = "<!-- Start Input Loop -->";
  
  for(int i = 0; i < I_Module_NUM; i++)
  {
    page +=         "<h3>Output Module 0x";
    page +=         String(I_Module_Address[i], HEX);
    page +=         "</h3><div class='row'>";
    
    for(int j = 0; j < 8; j++)
    {
      bool value = bitRead(I_Module_VAL[i], j); // Load value for button
     
      if (I_Module_DESC[i][j].length() == 0) // Check if descripton is empty
      {
        if (show_empty)
        {
          page +=         "<div class='col-xs-4'><h4 class ='text-left'>";
          page +=         j+1;
          page +=         " - Not defined</h4></div>";
        }
        else
        {
          continue; // Nothing to show, jump to the next in loop
        }
      }
      else
      {
        page +=         "<div class='col-xs-4'><h4 class ='text-left'>";
        page +=         j+1;
        page +=         " - ";
        page +=         I_Module_DESC[i][j];
        page +=         (I_Module_INV[i][j]  ? " [INV]" : "");
        page +=         "</h4></div>";
      }
      
      if (I_Module_INV[i][j] ? value : !value)
      {
        page +=         "<div class='col-xs-6'><form action='/input' method='POST'><button type='button submit' name=";
        page +=         j;
        page +=         " value='1' class='btn btn-success btn-lg'>ON</button></form></div>";
      }
      else
      {
        page +=         "<div class='col-xs-6'><form action='/input' method='POST'><button type='button submit' name=";
        page +=         j;
        page +=         " value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";
      }
    }
  }
  page +=       "</div>";
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
    page +=         "<h3>Output Module 0x";
    page +=         String(O_Module_Address[i], HEX);
    page +=         "</h3><div class='row'>";
    
    for(int j = 0; j < 8; j++)
    {
      if (O_Module_DESC[i][j].length() == 0)
      {
        if (show_empty)
        {
        page +=         "<div class='col-xs-4'><h4 class ='text-left'>";
        page +=         j+1;
        page +=         " - Not defined";
        page +=         "   <span class='badge'>";
        page +=         (O_Module_VAL[i][j] ? "ON" : "OFF");
        page +=         "</span></h4></div>";
        }
        else
        {
          continue; // Nothing to show, jump to the next in loop
        }
      }
      else
      {
        page +=         "<div class='col-xs-4'><h4 class ='text-left'>";
        page +=         j+1;
        page +=         " - ";
        page +=         O_Module_DESC[i][j];
        page +=         "   <span class='badge'>";
        page +=         (O_Module_VAL[i][j] ? "ON" : "OFF");
        page +=         "</span></h4></div>";
      }
      
      // Button ON
      page +=         "<div class='col-xs-4'><form action='/output' method='POST'><input type='hidden' name='module' value='";
      page +=         String(O_Module_Address[i], DEC);
      page +=         "'><input type='hidden' name='output' value='";
      page +=         j;
      page +=         "'><button type='button submit' name='value' value='1' class='btn btn-success btn-lg'>ON</button></form></div>";

      // Button OFF
      page +=         "<div class='col-xs-4'><form action='/output' method='POST'><input type='hidden' name='module' value='";
      page +=         String(O_Module_Address[i], DEC);
      page +=         "'><input type='hidden' name='output' value='";
      page +=         j;
      page +=         "'><button type='button submit' name='value' value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";
    }
    page +=       "</div>";
  }
  page +=       "<!-- End Output Loop -->";
  return page;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// HTML Footer

String P_Footer()
{
  String page = "<br><p><a href='http://www.nikolaus-lueneburg.de'>www.nikolaus-lueneburg.de</a> - Version " Sketch_Version "</p>";
  page += "</div></div></div>";
  page += "</body></html>";
  
  return page;
}
