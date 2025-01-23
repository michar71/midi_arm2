//-Base class for LED Effects

/*
FX Linear Base Class.

Linear visula effects (LED Strip, Stack of DMX Lights

FX Should follow these rules:
- Draw in their own graphic context (PGraphic instantiated as P2D)
- Be scalable (Extract size from the PGraphic Content)
- Not delete PGraphic (So we can stack effects)
- All durations are in microseconds !!!
- Time-independant (Update might be called on a non-regular base with a timestamp...)
- Begin/EndDraw is handled in update function, no need to call them in ani-functions
- Use the Params Dictionary to implement parameters that can change before or during animation run
- Maximum brightness shall be observed

- Generic Parameters for control 
BRIGHT[0..255]
SPEED [0..1000]
SCALE [0..1000]
DIR [0..359]
BURST [0/1]
COMPLEX [0..1000]

COL_R [0..255]
COL_G [0..255]
COL_B [0..255]
*/

import java.util.*;

int beatcount = 0;
class led_fx_base{
  
  //Common Parameters. Use these as we are supporting blending on these when set
  protected float maxBrightness =255;
  protected float current_red;
  protected float current_green;
  protected float current_blue;
  protected float current_dir;
  protected float current_speed;
  protected float current_complexity;
  protected float current_scale;
  protected float current_x;
  protected float current_y;
  protected float current_pitch;
  protected float current_roll;
  protected float current_yaw;
  protected float current_acc_x;
  protected float current_acc_y;
  protected float current_acc_z;
  protected int current_burst;
  protected boolean loop;
  protected boolean decay;
  protected boolean lerp;
  
  //Other Variables
  protected Dictionary params = new Hashtable(); 
  
  protected int fxHeight;
  protected String name;
  protected String filenamestr = "";
  
  private float lerpspeed = 4;
  protected int cnt = 0;
  protected  byte[] dmxData = new byte[512];
  
  //Class Creator
  led_fx_base(int height)
  {
    fxHeight = height;
    name = "Linear Base";
    //Not much to do here....
    params.put("BRIGHT",maxBrightness);
  }
  
  
  //-------------------------------------------------------------
  //  These functions to  be overriden in custom animation class
  //-------------------------------------------------------------
  public boolean run_linear_ani(long us, PGraphics gfx)
  {
      
    //Test Code...Draw Ascending color dots...
    switch(beatcount)
    {
      default:
      case 0:
        gfx.stroke(255,255,255);
        break;
      case 1:
        gfx.stroke(255,0,0);
        break;
      case 2:
        gfx.stroke(0,255,0);
        break;
      case 3:
        gfx.stroke(0,0,255);
        break;          
    }
    
    beatcount++;
    if (beatcount == 4)
      beatcount = 0;
    
    gfx.line(0,cnt,0,cnt+4);
    
    cnt++;
    if (cnt > fxHeight)
      cnt = 0;

    return true;
  }
  
  
   //--------------------------
   //  End of override Section
   //--------------------------
   
   
   
   
    void clear_ani(PGraphics gfx)
    {
      gfx.beginDraw();
      gfx.stroke(0);
      gfx.line(0,0,0,gfx.height);
      gfx.endDraw();
    }
    
    
    
    //Set the maximum brightness
    final void setBrightness(int b)
    {
      if (b>255)
        b = 255;
      if (b<0)
        b = 0;
      maxBrightness = b;  
        
    }
    
/*
Lerp a value. Pass in the old value and reasign the value to itself.
Example current_green = lerpVal(current_green,(int)params.get("COL_R"));
*/
    final float lerpVal(float currentval, float newval)
    {
      if (newval != currentval)
      {
        if (abs(currentval - newval) < 0.5)
        {
          currentval = newval;
          return newval;
        }
        
        if (newval > currentval)
        {
          return currentval + lerpspeed;
        }
        else
        {
          return currentval - lerpspeed;
        }
      }
      else
      {
        return currentval;
      }   
    }
    
    final float updateval(float currentval,String field,boolean direct)
    {
      float val;
      
      try
      {
        if (params.get(field) instanceof Integer)
        {
           val = (float)(int)params.get(field);
        }
        else if (params.get(field) instanceof Float)
        {
           val = (float)params.get(field);
        }
        else
        {
           val = 0;
        }
        
        if (direct)
          return val;
        else  
          return lerpVal(currentval,val);
      }
      catch (NullPointerException e)
      {
        return 0;
      }
    }

    final void updatevalswithlerp()
    {
      maxBrightness = updateval(maxBrightness,"BRIGHT",false);
      current_speed = updateval(current_speed,"SPEED",false);
      current_scale = updateval(current_scale,"SCALE",false);
      current_complexity = updateval(current_complexity,"COMPLEX",false);
      current_dir = updateval(current_dir,"DIR",false);                      
      current_red = updateval(current_red,"COL_R",false);
      current_green = updateval(current_green,"COL_G",false);
      current_blue = updateval(current_blue,"COL_B",false);
      current_x = updateval(current_x,"POS_X",false);
      current_y = updateval(current_y,"POS_Y",false);
      //dumpData();
    }


    final void updatevals()
    {
      maxBrightness = updateval(maxBrightness,"BRIGHT",true);
      current_speed = updateval(current_speed,"SPEED",true);
      current_scale = updateval(current_scale,"SCALE",true);
      current_complexity = updateval(current_complexity,"COMPLEX",true);
      current_dir = updateval(current_dir,"DIR",true);                      
      current_red = updateval(current_red,"COL_R",true);
      current_green = updateval(current_green,"COL_G",true);
      current_blue = updateval(current_blue,"COL_B",true);
      current_x = updateval(current_x,"POS_X",true);
      current_y = updateval(current_y,"POS_Y",true);
    }   
    
    
    
    final boolean update(long currentTimeUS,boolean useDecay,PGraphics gfx)
    {
      //First check if we need to update or not
      boolean anires = false;
      
      
      updatevalswithlerp();
        
      //Update Graphics  

      gfx.beginDraw();
      //gfx.stroke(0);
      //gfx.line(0,0,0,gfx.height);
      anires = run_linear_ani(currentTimeUS,gfx);
      gfx.endDraw();
      
      current_burst = 0;
      params.put("BURST",current_burst);

      
      //If decay is enabled on every frame we do not update we lower the brightness by some amount and refresh...
      if (useDecay == true)
      {
        gfx.beginDraw();
        gfx.tint(0,64);
        gfx.image(gfx,0,0);
        gfx.endDraw();
        anires = true;
      }
      
      return anires;
    }
    
    

    final boolean checkParam(String keyname)
    {
      if (params.get(keyname)== null)
        return false;
      else
        return true;
    }
    
    final void setParam(String keyname,int value)
    {
      params.put(keyname,value);
    }
    
    final int getParam(String keyname)
    {
      return (int)params.get(keyname);
    }  
    

    final String getName()
    {
      return name;
    }

    
    final boolean loadData()
    {
      //Generate filename
      JSONArray values;
      String filename = name + ".json";
      
      //Check if file exists
      File f = dataFile(filename);
      boolean exist = f.isFile();
      if (exist == false)
      {
          saveData();
          return false;
      }

      //Load data from file  
       values = loadJSONArray(filename);
      for (int i = 0; i < values.size(); i++) 
      {   
        JSONObject setting = values.getJSONObject(i); 
        String param = setting.getString("param");
        int val = setting.getInt("val");
        params.put(param,val);
      }
      updatevals();
      return true;
    }
    
    final boolean saveData()
    {
      JSONArray values;
       //create filename
      String filename = name + ".json";
      filename = dataPath(filename);

      
      //Save Data to file (We iterate through the current property list)
      values = new JSONArray();
      int i = 0;
      
      for (Enumeration k = params.keys(); k.hasMoreElements();) 
      { 
          String param = (String)k.nextElement(); 
          int val;
          if (params.get(param) instanceof Integer)
          {
             val = (int)params.get(param);
          }
          else if (params.get(param) instanceof Float)
          {
             val = (int)(float)params.get(param);
          }
          else
          {
             val = 0;
          }
          
          JSONObject setting = new JSONObject();

          setting.setString("param", param);
          setting.setInt("val", val);
          values.setJSONObject(i, setting);
          i++;
      } 
      saveJSONArray(values, filename); 
      return true;
    }
    
    
    
    final void dumpData()
    {
      for (Enumeration k = params.keys(); k.hasMoreElements();) 
      { 
          String param = (String)k.nextElement(); 
          int val;
          if (params.get(param) instanceof Integer)
          {
             val = (int)params.get(param);
          }
          else if (params.get(param) instanceof Float)
          {
             val = (int)(float)params.get(param);
          }
          else
          {
             val = 0;
          }
          
          JSONObject setting = new JSONObject();

          print(param);
          print(":");
          println(val);
      } 
    }
    
    final void setFilename(String filen)
    {
        filenamestr = filen;
        println(filenamestr);
    }
    
    
    void setOrientation(float yaw, float pitch, float roll, float accX, float accY, float accZ)
    {
        current_pitch = pitch;  
        current_roll = roll;
        current_yaw = yaw;
        current_acc_x = accX;
        current_acc_y = accY;
        current_acc_z = accZ;
    }
    
    void setArtnet(byte[] data)
    {
      dmxData = data;
    }
}
