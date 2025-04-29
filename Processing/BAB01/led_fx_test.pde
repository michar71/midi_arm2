

class led_fx_test extends led_fx_base
{
    int current_led = 0;

    led_fx_test(int height)
    {
      super(height);
      name = "Test";
      loadData();
    }
  
  
    public boolean run_linear_ani(long us, PGraphics gfx)
    {
      gfx.stroke(0,0,0);
      gfx.line(0,0,0,15);        
      gfx.stroke(128,192,255);
      gfx.line(0,0,0,current_led);  
      
      current_led++;
      if (current_led == 16)
        current_led = 0;
      
      return true;
  }
}
