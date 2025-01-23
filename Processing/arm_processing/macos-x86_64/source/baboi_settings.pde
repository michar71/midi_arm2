


class baboi_settings{
 
  
//Put all settings vartiables here\
float minp,maxp,minr,maxr,miny,maxy;
boolean splitp = false;
boolean splitr = false;
boolean splity = false;
boolean artnet_en =false;
boolean network_en =false;
String midiOut = "";
String uartOverride = "";
int winx;
int winy;
int fx;
boolean use2D = false;

 public void load_settings()
{
  JSONObject json;
  json = loadJSONObject("setup.json");
  
  if (json != null)
  {
    try
    {
      maxp = json.getFloat("maxp");
      minp = json.getFloat("minp");
      maxr = json.getFloat("maxr");
      minr = json.getFloat("minr");
      maxy = json.getFloat("maxy");
      miny = json.getFloat("miny");      

      splitp = json.getBoolean("splitp");
      splitr = json.getBoolean("splitr");
      splity = json.getBoolean("splity");
      
      artnet_en = json.getBoolean("artnet");
      network_en = json.getBoolean("network");
      
      winx = json.getInt("winx");        
      winy = json.getInt("winy");      
      fx = json.getInt("fx");  
    }
    
    catch (Exception e)
    { //Print the type of error
      println("Error loading Preset", e);
    }
    
    try
    {
      use2D = json.getBoolean("use2D");
      if (use2D)
        println("Force 2D Renderer");
      else
        println("Using 3D");
    }
    catch (Exception e)
    { //Print the type of error
      println("Using 3D");
    }
    
    
    try
    {
      midiOut = json.getString("midiBusOut","Bus 1");
      println("Using Midi Bus Out:" + midiOut);
    }
    catch (Exception e)
    { //Print the type of error
      midiOut = "Bus 1";
      println("Using Default Midi Bus Out:" + midiOut);
    }    
   
    try
    {
      uartOverride = json.getString("UARToverride","");
      if (0 == uartOverride.compareTo(""))
        println("Using default UART");
      else
        println("Using Custom UART:" + uartOverride);
    }
    catch (Exception e)
    { //Print the type of error
      uartOverride = "";
      println("Using Default UART");
    }          
  }
}

public void save_settings()
{
  JSONObject json = new JSONObject();

  json.setFloat("maxp",maxp);
  json.setFloat("minp",minp);
  json.setFloat("maxr",maxr);
  json.setFloat("minr",minr);
  json.setFloat("maxy",maxy);
  json.setFloat("miny",miny);
  
  json.setBoolean("splitp",splitp);    
  json.setBoolean("splitr",splitr);  
  json.setBoolean("splity",splity);  
  
  json.setBoolean("artnet",artnet_en);
  json.setBoolean("network",network_en);

  json.setInt("winx",winx);  
  json.setInt("winy",winy);  
  json.setInt("fx",fx);  
  
  saveJSONObject(json,"setup.json");
}

public void clear_cal_min_max()
{
  minp = 65535;
  maxp = -65535;
  minr = 65535;
  maxr = -65535;  
  miny = 65535;
  maxy = -65535; 
}



}
