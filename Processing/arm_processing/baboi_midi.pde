import themidibus.*; //Import the library


class baboi_midi{


boolean last_A_state = false;
boolean last_B_state = false;
boolean last_C_state = false;

baboi_protocol bpr;
baboi_settings bsr;
MidiBus myBus; // The MidiBus

 public baboi_midi(baboi_protocol bp,baboi_settings bs)
{
    bpr = bp;
    bsr = bs;
    MidiBus.list();
    myBus = new MidiBus(this, -1, "Bus 1"); // Create a new MidiBus with no input device and the default MacOS Midi Distributor as output
    //myBus = new MidiBus(this, -1, "Bluetooth"); // Midi Bus needs to be selectable or a command line/settings parameter
} 

  
  
  

public void send_midi()
{
  
  if (bsr.splitp)
  {
      float half = map(50,0,100, bsr.minp,bsr.maxp);
      if (bpr.cp < half)
      {
        m1 =(int)map(bpr.cp,bsr.minp, half, 127,0);
        m1 = (int)constrain(m1,0,127);
        m11 = 0;
      }
      else
      {
        m11 =(int)map(bpr.cp,half, bsr.maxp, 0,127);
        m11 = (int)constrain(m11,0,127);
        m1 = 0;
      }
      ControlChange change1 = new ControlChange(0, 1, m1);
      myBus.sendControllerChange(change1);
      ControlChange change2 = new ControlChange(0, 4, m11);
      myBus.sendControllerChange(change2);        
  }
  else
  {
    m11 = 0;
    m1 =(int)map(bpr.cp,bsr.minp, bsr.maxp, 0,127);
    m1 = (int)constrain(m1,0,127);
    ControlChange change1 = new ControlChange(0, 1, m1);
    myBus.sendControllerChange(change1);
  }
  
  if (bsr.splitr)
  {
      float half = map(50,0,100, bsr.minr,bsr.maxr);
      if (bpr.cr < half)
      {
        m2 =(int)map(bpr.cr,bsr.minr, half, 127,0);
        m2 = (int)constrain(m2,0,127);
        m22 = 0;
      }
      else
      {
        m22 =(int)map(bpr.cr,half, bsr.maxr, 0,127);
        m22 = (int)constrain(m22,0,127);
        m2 = 0;
      } 
      ControlChange change1 = new ControlChange(0, 2, m2);
      myBus.sendControllerChange(change1);
      ControlChange change2 = new ControlChange(0, 5, m22);
      myBus.sendControllerChange(change2);           
  }
  else
  {
    m22 = 0;
    m2 =(int)map(bpr.cr,bsr.minr, bsr.maxr, 0,127);
    m2 = (int)constrain(m2,0,127);
    ControlChange change1 = new ControlChange(0, 2, m2);
    myBus.sendControllerChange(change1);
  }
  
  if (bsr.splity)
  {
      float half = map(50,0,100, bsr.miny,bsr.maxy);
      if (bpr.cy < half)
      {
        m3 =(int)map(bpr.cy,bsr.miny, half, 127,0);
        m3 = (int)constrain(m3,0,127);
        m33 = 0;
      }
      else
      {
        m33 =(int)map(bpr.cy,half, bsr.maxy, 0,127);
        m33 = (int)constrain(m33,0,127);
        m3 = 0;
      }
      ControlChange change1 = new ControlChange(0, 3, m3);
      myBus.sendControllerChange(change1);
      ControlChange change2 = new ControlChange(0, 6, m33);
      myBus.sendControllerChange(change2);            
  }
  else
  {
    m33 = 0;
    m3 =(int)map(bpr.cy,bsr.miny, bsr.maxy, 0,127);
    m3 = (int)constrain(m3,0,127);
    ControlChange change1 = new ControlChange(0, 3, m3);
    myBus.sendControllerChange(change1);
  }
  
  //Send Tension midi data... We only send it if it actually exists (not -1...)
  if (bpr.tension_ch1 != -1)
  {
    ControlChange change = new ControlChange(0, 9, bpr.tension_ch1/2);
    myBus.sendControllerChange(change);
  }
  if (bpr.tension_ch2 != -1)
  {
    ControlChange change = new ControlChange(0, 10, bpr.tension_ch2/2);
    myBus.sendControllerChange(change);
  }
}


public void send_buttons()
{

  if ((last_A_state == false) && (bpr.b_A_state == true))
  {
    last_A_state = bpr.b_A_state;
    ControlChange change1 = new ControlChange(0, 7, 1);
    myBus.sendControllerChange(change1);
  }

  else if ((last_A_state == true) && (bpr.b_A_state == false))
  {
    last_A_state = bpr.b_A_state;
    ControlChange change1 = new ControlChange(0, 7, 0);
    myBus.sendControllerChange(change1);
  }

  if ((last_B_state == false) && (bpr.b_B_state == true))
  {
    last_B_state = bpr.b_B_state;
    ControlChange change1 = new ControlChange(0, 8, 1);
    myBus.sendControllerChange(change1);
  }

  else if ((last_B_state == true) && (bpr.b_B_state == false))
  {
    last_B_state = bpr.b_B_state;
    ControlChange change1 = new ControlChange(0, 8, 0);
    myBus.sendControllerChange(change1);
  }
  
  if ((last_C_state == false) && (bpr.b_C_state == true))
  {
    last_C_state = bpr.b_C_state;
    ControlChange change1 = new ControlChange(0, 9, 1);
    myBus.sendControllerChange(change1);
  }

  else if ((last_C_state == true) && (bpr.b_C_state == false))
  {
    last_C_state = bpr.b_C_state;
    ControlChange change1 = new ControlChange(0, 9, 0);
    myBus.sendControllerChange(change1);
  }  
}


public void send_midi_map(char mkey)
{
        
      if (mkey =='1')
      {
          ControlChange change1 = new ControlChange(0, 1, 63);
          myBus.sendControllerChange(change1);
      }
      if (mkey =='2')
      {
          ControlChange change1 = new ControlChange(0, 2, 63);
          myBus.sendControllerChange(change1);
      }
      if (mkey =='3')
      {
          ControlChange change1 = new ControlChange(0, 3, 63);
          myBus.sendControllerChange(change1);
      }   
      if (mkey =='4')
      {
          ControlChange change1 = new ControlChange(0, 4, 1);
          myBus.sendControllerChange(change1);
      }
      if (mkey =='5')
      {
          ControlChange change1 = new ControlChange(0, 5, 1);
          myBus.sendControllerChange(change1);
      }
      if (mkey =='6')
      {
          ControlChange change1 = new ControlChange(0, 6, 1);
          myBus.sendControllerChange(change1);
      }       
      if (mkey =='7')
      {
          ControlChange change1 = new ControlChange(0, 7, 1);
          myBus.sendControllerChange(change1);
      }
      if (mkey =='8')
      {
          ControlChange change1 = new ControlChange(0, 8, 1);
          myBus.sendControllerChange(change1);
      }
      if (mkey =='9')
      {
          ControlChange change1 = new ControlChange(0, 9, 1);
          myBus.sendControllerChange(change1);
      }   
      if (mkey =='0')
      {
          ControlChange change1 = new ControlChange(0, 10, 1);
          myBus.sendControllerChange(change1);
      }    
      if (mkey =='-')
      {
          ControlChange change1 = new ControlChange(0, 11 , 1);
          myBus.sendControllerChange(change1);
      } 
}

public void update_midi()
{
  send_midi();
  send_buttons();
}

}
