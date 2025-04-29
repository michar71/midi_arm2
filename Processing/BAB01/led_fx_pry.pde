class led_fx_pry extends led_fx_base
{
    int ledCnt = 0;

    led_fx_pry(int height)
    {
      super(height);
      name = "PRY";
      loadData();
    }
  
  
    public boolean run_linear_ani(long us, PGraphics gfx)
    {
      //Test Code...Draw Ascending color dots...
      gfx.stroke(current_pitch,current_roll,current_yaw);
      gfx.line(0,0,0,16);
      return true;
  }
}
