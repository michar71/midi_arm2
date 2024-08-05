class led_fx_cylon extends led_fx_base
{
    int Cnt = 0;
    int current_led = 0;

    led_fx_cylon(int height)
    {
      super(height);
      name = "PRY";
      loadData();
    }
  
  
    public boolean run_linear_ani(long us, PGraphics gfx)
    {
      gfx.tint(0,(int)map(current_pitch,0,255,16,128));
      gfx.image(gfx,0,0);
      
      //Cnt--;
      //if (Cnt <= 0)
      {
        Cnt = (int)(current_yaw / 16);

       
        colorMode(HSB,255);
        color col = color(current_roll,255,128);
        colorMode(RGB,255);
        gfx.set(0,current_led,col);
        
        current_led++;
        if (current_led == 16)
          current_led = 0;
      }

      
      return true;
  }
}
