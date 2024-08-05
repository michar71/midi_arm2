class led_fx_artnet extends led_fx_base
{
    int ledCnt = 0;

    led_fx_artnet(int height)
    {
      super(height);
      name = "Artnet";
      loadData();
    }
  
  
    public boolean run_linear_ani(long us, PGraphics gfx)
    {
      for(int ii=0;ii<16;ii++)
      {
        color col = color(dmxData[ii*3+0],dmxData[ii*3+1],dmxData[ii*3+2]);      
        gfx.set(0,ii,col);
      }
      return true;
  }
}
