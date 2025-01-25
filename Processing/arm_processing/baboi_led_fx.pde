//Link To LED FX
// -maintiain list
// - Manage active FX
// = Distribute Parameters
// - Generate LED Data for Update


import java.util.*; 

class baboi_led_fx{
  
    protected led_fx_base current_fx;
    protected led_fx_base next_fx;

  
    protected boolean newFx = false;
    protected int fxHeight;
    protected PGraphics fxGfx;
    public boolean useDecay = false;
  
  class fx_member
  {
      public String fx_name; 
      public led_fx_base fx;
      
      fx_member(led_fx_base fxs, String name)
      {
        fx_name = name;
        fx = fxs;
      }
  };

  public List<fx_member> fxlist = new ArrayList<fx_member>();
  int fxindex = 0;

  baboi_led_fx(int height)
  {
    fxHeight = height;
    
    //Add the Effects to a list
    fxlist.add(new fx_member((led_fx_base)new led_fx_none(fxHeight),"None"));
    fxlist.add(new fx_member((led_fx_base)new led_fx_test(fxHeight),"Test"));
    fxlist.add(new fx_member((led_fx_base)new led_fx_pry(fxHeight),"PRY"));
    fxlist.add(new fx_member((led_fx_base)new led_fx_cylon(fxHeight),"Cylon"));
    fxlist.add(new fx_member((led_fx_base)new led_fx_artnet(fxHeight),"ArtNet"));
  
    current_fx = getCurrent();
    next_fx = current_fx;
    newFx = false;
    fxGfx = createGraphics(1, fxHeight,P2D);
  }
  
  void setNextFx(led_fx_base fx)
  {
    next_fx = fx;
    newFx = true;
  }
  
  void SetDecayOn(boolean val)
  {
    useDecay = val;
  }
  
    void setParam(String param, int value)
  {
    current_fx.setParam(param,value);
  }
  
  int getParam(String param)
  {
    return current_fx.getParam(param);
  }
    
   void setFilename(String filen)
  {
    current_fx.setFilename(filen);
  }
  
    void saveData()
  {
    current_fx.saveData();
  }
  
boolean hasNext()
{
  if ((fxindex+1) > fxlist.size()-1)
    return false;
  else 
    return true;
}

boolean hasPrevious()
{
  if ((fxindex-1) < 0)
    return false;
  else 
    return true;
}

led_fx_base getNext()
{
  if (hasNext())
  {
    fxindex++;
    return fxlist.get(fxindex).fx;
  }
  return null;
}

led_fx_base getPrevious()
{
  if (hasPrevious())
  {
    fxindex--;
    return fxlist.get(fxindex).fx;
  }
  return null;
}

String getFxName()
{
  return fxlist.get(fxindex).fx_name;
}

boolean setIndex(int index)
{
  if((index > 0) & (index < fxlist.size()-1))
  {
    fxindex = index;
    return true;
  }
  else
  {
    return false;
  }
}

int getIndex()
{
  return fxindex;
}

led_fx_base getCurrent()
{
    return fxlist.get(fxindex).fx;
}

led_fx_base findByName(String name)
{
  for (int jj=0;jj<fxlist.size();jj++)
  {
    if (fxlist.get(jj).fx_name.equals(name) == true)
      return fxlist.get(jj).fx;
  }
  return null;
}

led_fx_base findByIndex(int ii)
{
  if (ii<fxlist.size())
  {
      return fxlist.get(ii).fx;
  }
  return null;
}

void getNames(List fxnames)
{
  for (int ii=0;ii< fxlist.size();ii++)
  {
    fxnames.add(fxlist.get(ii).fx_name);
  }
}

void setOrientation(float yaw, float pitch, float roll, float accX, float accY, float accZ)
{
  current_fx.setOrientation(yaw, pitch, roll, accX, accY, accZ);
}
      

color[] updateLED()
{
  color[] ledData = new color[16];

  if (next_fx != current_fx)
  {
    current_fx = next_fx;
  }
  
  
  if (true == current_fx.update(System.nanoTime()/1000,useDecay,fxGfx))
  {
     //UPdate LEDs
     fxGfx.loadPixels();
     for (int yy=0;yy<fxHeight;yy++)
     {
       ledData[yy] = fxGfx.pixels[yy];
     }    
  }
  return ledData;
}


void setArtnet(byte[] data)
{
  if (data != null)
    current_fx.setArtnet(data);
}


}
