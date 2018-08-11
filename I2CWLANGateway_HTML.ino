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

String P_Input(){  
  String page =         "<h3>Input Board 0x20</h3>";
  page +=         "<div class='row'>";  

  for(int i = 0; i < Input20_NUM; i++)
  {
    page +=         "<div class='col-xs-6'><h4 class ='text-left'>0x20 - ";
    page +=         i;
    page +=         " - ";
    page +=         Input20_DESC[i];
    page +=         (Input20_INV[i]  ? " [INV]" : "");
    page +=         "</h4></div>";
    
    
    if (Input20_INV[i]  ? Input20_VAL[i] : !Input20_VAL[i])
    {
      page +=         "<div class='col-xs-6'><form action='/input' method='POST'><button type='button submit' name=";
      page +=         i;
      page +=         " value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";
    }
    else
    {
      page +=         "<div class='col-xs-6'><form action='/input' method='POST'><button type='button submit' name=";
      page +=         i;
      page +=         " value='1' class='btn btn-success btn-lg'>ON</button></form></div>";
    }
  }
  page +=       "</div>";

  return page;
}

  String P_Output(){

  String page =   "<h3>Output Board 0x21</h3>";
  page +=         "<div class='row'>";  

  for(int i = 0; i < Output21_NUM; i++)
  {
    page +=         "<div class='col-xs-4'><h4 class ='text-left'>0x21 - ";
    page +=         i+1;
    page +=         " - ";
    page +=         Output21_DESC[i];
    page +=         "   <span class='badge'>";
    page +=         (Output21_VAL[i] ? "ON" : "OFF");
    page +=         "</span></h4></div>";
    page +=         "<div class='col-xs-4'><form action='/output' method='POST'><input type='hidden' name='board' value='21'><input type='hidden' name='output' value='";
    page +=         i;
    page +=         "'><button type='button submit' name='value' value='1' class='btn btn-success btn-lg'>ON</button></form></div>";
    page +=         "<div class='col-xs-4'><form action='/output' method='POST'><input type='hidden' name='board' value='21'><input type='hidden' name='output' value='";
    page +=         i;
    page +=         "'><button type='button submit' name='value' value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";
  }
  page +=       "</div>";
  
  return page;
}  

String P_Footer()
{
  String page = "<br><p><a href='http://www.nikolaus-lueneburg.de'>www.nikolaus-lueneburg.de</a> - Version " Sketch_Version "</p>";
  page += "</div></div></div>";
  page += "</body></html>";
  
  return page;
}
