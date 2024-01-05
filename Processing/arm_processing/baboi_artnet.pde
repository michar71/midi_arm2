import ch.bildspur.artnet.*;

class baboi_artnet{
  
  ArtNetClient artnet;
  baboi_protocol bpr;
  baboi_settings bsr;  
  byte[] dmxData = new byte[512];
  
  public baboi_artnet(baboi_protocol bp,baboi_settings bs)
  {
    bpr = bp;
    bsr = bs;
    artnet = new ArtNetClient(null);
    artnet.start();
  }
  

  public void disable_artnet()
  {
    dmxData[0] = (byte) 0; 
    dmxData[1] = (byte) 0; 
    dmxData[2] = (byte) 0; 
    dmxData[3] = (byte) 0; 
    dmxData[4] = (byte) 0; 
    dmxData[5] = (byte) 0;     
    // send dmx to localhost
    //artnet.unicastDmx("127.0.0.1", 0, 0, dmxData);
    
    //Fuckit... Just send it to everybody ;-)
    artnet.broadcastDmx(0, 0, dmxData);
  }
  
  
  void send_artnet()
{
    // fill dmx array
    int val = 0;



    val =(int)map(bpr.cr,bsr.minr, bsr.maxr, 0,255);
    val = limit(val,0,255);
    dmxData[0] = (byte) val;
    
    val =(int)map(bpr.cp,bsr.minp, bsr.maxp, 255,0);
    val = limit(val,0,255);
    dmxData[1] = (byte) val;
    
    val =(int)map(bpr.cy,bsr.miny, bsr.maxy, 0,255);
    val = limit(val,0,255);
    dmxData[2] = (byte) val;
    
    val = (int)map(abs(bpr.accx),0,6,0,255);    
    dmxData[3] = (byte) val; 

    val = (int)map(abs(bpr.accy),0,6,0,255);    
    dmxData[4] = (byte) val; 

    val = (int)map(abs(bpr.accz),0,6,0,255);    
    dmxData[5] = (byte) val; 

    //Send glove data if it exists
    if (bpr.tension_ch1 != -1)
    {
      dmxData[6] = (byte)bpr.tension_ch1;
    }
    else
    {
      dmxData[6] = (byte) 0; 
    }
    if (bpr.tension_ch2 != -1)
    {
      dmxData[7] = (byte)bpr.tension_ch2;
    }
    else
    {
      dmxData[7] = (byte) 0; 
    }
    if (bpr.tension_ch3 != -1)
    {
      dmxData[8] = (byte)bpr.tension_ch3;
    }
    else
    {
      dmxData[8] = (byte) 0; 
    }
    if (bpr.tension_ch4 != -1)
    {
      dmxData[9] = (byte)bpr.tension_ch4;
    }
    else
    {
      dmxData[9] = (byte) 0; 
    }
    
    
    // send dmx to localhost
    //artnet.unicastDmx("127.0.0.1", 0, 0, dmxData);
    
    //Fuckit... Jsut send it to everybody ;-)
    //artnet.broadcastDmx(0, 0, dmxData);
    
  // send dmx to localhost
  artnet.unicastDmx("192.168.0.133",0, 1, dmxData);
}

  
  
}
