

class led_fx_none extends led_fx_base
{
    led_fx_none(int height)
    {
      super(height);
      name = "None";
      loadData();
    }
  
  
    public boolean run_linear_ani(long us, PGraphics gfx)
    {
      //Test Code...Draw Ascending color dots...
      gfx.stroke(0,0,0);
      gfx.line(0,0,0,16);
      return true;
  }
}
