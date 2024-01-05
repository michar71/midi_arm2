


class baboi_settings{
 
  
//Put all settings vartiables here\
float minp,maxp,minr,maxr,miny,maxy;
boolean splitp = false;
boolean splitr = false;
boolean splity = false;
boolean artnet_en =false;
boolean network_en =false;
int winx;
int winy;
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
