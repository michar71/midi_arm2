/*
This code is made for processing https://processing.org/

EVERYTHING SHOULD BE IN ORDER 
PITCH
ROLL
YAW
*/

import processing.serial.*;     // import the Processing serial library

import java.lang.*;
import java.util.*;
import controlP5.*;
import com.jogamp.newt.opengl.GLWindow;

baboi_protocol bp;
baboi_midi bm;
baboi_artnet ba;
baboi_settings bs;
baboi_led_fx ledctrl;

ControlP5 cp5;

boolean isCal;
boolean isMap;
int lastUpdate = 0;

int m1 = 0;
int m2 = 0;
int m3 = 0;
int m11 = 0;
int m22 = 0;
int m33 = 0;

PShape arrow;

boolean isLive = true;

PVector getWindowLocation(String renderer) 
{
  PVector l = new PVector();
  if (renderer == P2D || renderer == P3D) 
  {
    com.jogamp.nativewindow.util.Point p = new com.jogamp.nativewindow.util.Point();
    ((com.jogamp.newt.opengl.GLWindow)surface.getNative()).getLocationOnScreen(p);
    l.x = p.getX();
    l.y = p.getY();
  } 
  else if (renderer == JAVA2D) 
  {
    java.awt.Frame f =  (java.awt.Frame) ((processing.awt.PSurfaceAWT.SmoothCanvas) surface.getNative()).getFrame();
    l.x = f.getX();
    l.y = f.getY();
  }
  return l;
}

void Splitp(boolean theFlag) 
{
  bs.splitp = theFlag;
  bs.save_settings();
}

void Splitr(boolean theFlag) 
{
  bs.splitr = theFlag;
  bs.save_settings();    
}

void Splity(boolean theFlag) 
{
  bs.splity = theFlag;
  bs.save_settings();   
}

void Artnet(boolean theFlag) 
{
  bs.artnet_en = theFlag;
  bs.save_settings();   

  if (bs.artnet_en == false)
  {
     ba.disable_artnet();
  } 
}

void RAWLog(boolean theFlag) 
{
  bs.log_en = theFlag;
  bs.save_settings();   

  if (bs.log_en == false)
  {
     bp.enableLogger(false);
  }
  else
  {
     bp.enableLogger(true);    
  }
}

void Network(boolean theFlag) 
{
  bs.network_en = theFlag;
  bs.save_settings();    
}


void settings()
{
  bs = new baboi_settings();
  bs.load_settings();
   
  if (bs.use2D)
    size(400, 260,P2D);
  else
    size(400, 260,P3D);
}


void setup() {
   bp = new baboi_protocol(this,bs.uartOverride); 
   bm = new baboi_midi(bp,bs,bs.midiOut);
   ba = new baboi_artnet(bp,bs);
   ledctrl = new baboi_led_fx(16);

  
  surface.setTitle("BABOI CONTROL");
  surface.setResizable(false);
  
  frameRate(30);
  
  cp5 = new ControlP5(this);
  prepareExitHandler();
  

  //Create UI
  cp5.addButton("Range")
     .setBroadcast(false)
     .setValue(0)
     .setPosition(width-110,10)
     .setSize(100,18)
     .setBroadcast(true)
     ;
  
  cp5.addButton("Map")
     .setBroadcast(false)
     .setValue(100)
     .setPosition(width-110,30)
     .setSize(100,18)
     .setBroadcast(true)
     ;
  
    cp5.addToggle("Splitp")
     .setBroadcast(false)
     .setValue(bs.splitp)
     .setPosition(width-110,60)
     .setSize(18,18)
     .setLabel("Split P")
     .setBroadcast(true)
     ;
  
    cp5.addToggle("Splitr")
     .setBroadcast(false)
     .setValue(bs.splitr)
     .setPosition(width-70,60)
     .setSize(18,18)
     .setLabel("Split R")
     .setBroadcast(true)
     ;
     
    cp5.addToggle("Splity")
     .setBroadcast(false)
     .setValue(bs.splity)
     .setPosition(width-30,60)
     .setSize(18,18)
     .setLabel("Split Y")
     .setBroadcast(true)
     ;     
     
    cp5.addToggle("RAWLog")
     .setBroadcast(false)
     .setValue(bs.log_en)
     .setPosition(width-44,height - 110)
     .setSize(18,18)
     .setLabel("RAWLog")
     .setBroadcast(true)
     ;   
     
    cp5.addToggle("Artnet")
     .setBroadcast(false)
     .setValue(bs.artnet_en)
     .setPosition(width-44,height - 70)
     .setSize(18,18)
     .setLabel("ArtNet")
     .setBroadcast(true)
     ;   
     
    cp5.addToggle("Network")
     .setBroadcast(false)
     .setValue(bs.network_en)
     .setPosition(width-44,height - 30)
     .setSize(18,18)
     .setLabel("Network")
     .setBroadcast(true)
     ;      
     
     cp5.addButton("CalB")
     .setBroadcast(false)
     .setValue(0)
     .setPosition(5,height-20)
     .setSize(50,18)
     .setBroadcast(true)
     .setCaptionLabel("CAL B")
     ; 
     
     cp5.addButton("CalG")
     .setBroadcast(false)
     .setValue(0)
     .setPosition(60,height-20)
     .setSize(50,18)
     .setBroadcast(true)
     .setCaptionLabel("CAL G")
     ; 
     
     List<String> fxnames = new ArrayList<String>();
     ledctrl.getNames(fxnames);
     /* add a ScrollableList, by default it behaves like a DropdownList */
     cp5.addScrollableList("FXList")
       .setBroadcast(false)
       .setPosition(width-110,96)
       .setSize(100,120)
       .setBarHeight(20)
       .setItemHeight(20)
       .addItems(fxnames)
       .setValue(bs.fx)
       .setBroadcast(true)
       ;

  bp.enableLogger(bs.log_en);
  

  
  ledctrl.setNextFx(ledctrl.findByIndex(bs.fx));
  
  arrow = loadShape("arrow.obj");
  arrow.scale(100);
  float x = arrow.getChild(0).getVertex(0).x;
  float y = arrow.getChild(0).getVertex(0).y;
  float z = arrow.getChild(0).getVertex(0).z;
  
  arrow.translate(-x,-y,-z);
  //arrow.rotateX(PI);
  arrow.rotateY(PI);
}

public void FXList(int n)
{
  bs.fx = n; 
  ledctrl.setNextFx(ledctrl.findByIndex(bs.fx));
  bs.save_settings();
  
}


public void Range(int theValue) {
        if (isCal)
        {
          isCal = false;
          bs.normalize_range_settings();
          bs.save_settings();
        }
        else
        {
          isCal = true;
          bs.clear_cal_min_max();
        }
}



public void Map(int theValue) {
        if (isCal == false)
        isMap = !isMap;
}


public void CalG(int theValue)
{
  if (bp.checkConnected())
    bp.sendSetupRequest(1);
}

public void CalB(int theValue)
{
  if (bp.checkConnected())
    bp.sendSetupRequest(0);
}

void show_map_text()
{
  fill(255);
  textAlign(LEFT);
  text("Mapping Keys:",130,height-80);
  text("Pitch=1,Roll=2,Yaw=3",130,height-64);
  text("P-Split=4,R-Split=5,=Y-Split=6",130,height-48);
  text("Button A=7,Button B=8",130,height-32);
  text("Finger 1=9,Finger 2=0",130,height-16);
}


void show_acceleration()
{
  int r = (int)map(bp.caccx,-4,4,64,255);
  int g = (int)map(bp.caccy,-4,4,64,255);
  int b = (int)map(bp.caccz+1,-4,4,64,255);
  stroke(r,g,b);
  fill(r,g,b);
}


void draw_labels()
{

  hint(DISABLE_DEPTH_TEST);
  if (isCal)
  {
    fill(128,0,0);
    rect(0,0,160,66);
  }
  else if (isMap)
  {
    fill(0,128,0);
    rect(0,0,160,66);     
  }

  fill(255);
  
  textAlign(LEFT);
  textSize(14);
  text("P:"+nf(bp.cp,0,2),5,10);
  text("R:"+nf(bp.cr,0,2),5,22);
  text("Y:"+nf(bp.cy,0,2),5,34);
  text(nf(bs.minp,0,2)+"/"+nf(bs.maxp,0,2),60,10);
  text(nf(bs.minr,0,2)+"/"+nf(bs.maxr,0,2),60,22);
  text(nf(bs.miny,0,2)+"/"+nf(bs.maxy,0,2),60,34);
  
  if (isCal == false)
  {
    if (bs.splitp)
      text(m1+"/"+m11,128,10);
    else
      text(m1,128,10);
      
    if (bs.splitr)  
      text(m2+"/"+m22,128,22);
    else
      text(m2,128,22);
    
    if (bs.splity)
      text(m3+"/"+m33,128,34);  
    else
      text(m3,128,34);  
  }    
      
  if (bp.b_A_state)
    text("A", 5,46);
  
  if (bp.b_B_state)
    text("B", 20,46);
    
  if (bp.b_C_state)
    text("C", 35,46);
  
  text(bp.deviceName+" V"+bp.maj_ver+"."+ bp.min_ver, 5,58);  

    
  if (isMap)
         show_map_text();
         
  
  hint(ENABLE_DEPTH_TEST);
}

void delay(int time) 
{
  int current = millis();
  while (millis () < current+time) Thread.yield();
}


void draw_cube()
{  
  float dirY = (((float)height/4*3) / float(height) - 0.5) * 2;
  float dirX = (((float)width/4*3) / float(width) - 0.5) * 2;
  directionalLight(204, 204, 204, dirX, dirY, -1); //Why is this linked to the mouse? 
  //noStroke();
  pushMatrix();
  show_acceleration();
  translate(width/2, height/2);
  /*
  PMatrix3D rm = new PMatrix3D();
  rm = toMatrix(rm,qx,qy,qz,qw);
  applyMatrix(rm);
  */
  if (bp.ctension_ch1 != -1)
  {
    translate(-180, 0, -100); 
    box(50, bp.ctension_ch1, 50);
    translate(180, 0, 100); 
  }
  if (bp.ctension_ch2 != -1)
  {
    translate(180, 0, -100); 
    box(50, bp.ctension_ch2, 50);
    translate(-180, 0, 100);  
  }
  rotateY(bp.cy);//yaw
  rotateX(bp.cp);//pitch
  rotateZ(bp.cr);//roll

  shape(arrow);
  //TODO turen this into an arrow.....
  //box(100*1.5, 100/3, 100);
  
  popMatrix(); 
}


void draw_horizon()
{
  text("T1:"+nf(bp.ctension_ch1,0,2),5,70);
  text("T2:"+nf(bp.ctension_ch2,0,2),5,82);
}



void draw_led(color[] data)
{
  for(int yy=0;yy<data.length;yy++)
  {
    fill(data[yy]);
    stroke(data[yy]);
    rect(0,140-(yy*4),4,4);
  }
}

void send_led()
{
  color[] ledData;
  
  //we map yaw, pitch, roll to 0..255
  
  float div;
  float offset;
  
  div = (int)(bp.cy / (2*PI));
  offset = (2*PI) * div;
  if (bp.cy < 0)
     offset = offset - (2*PI);    
  
  int y = (int)map(bp.cy,bs.miny+offset, bs.maxy+offset, 0,255);
  y = (int)constrain(y,0,255);
  
  div = (int)(bp.cr / (2*PI));
  offset = (2*PI) * div;
  if (bp.cr < 0)
     offset = offset - (2*PI);  
     
  int r = (int)map(bp.cr,bs.minr+offset, bs.maxr+offset, 0,255);
  r = (int)constrain(r,0,255);
  
  div = (int)(bp.cp / (2*PI));
  offset = (2*PI) * div;
  if (bp.cp < 0)
     offset = offset - (2*PI);   
  int p = (int)map(bp.cp,bs.minp+offset, bs.maxp+offset, 0,255);  
  p = (int)constrain(p,0,255);
  
  ledctrl.setOrientation(y, p, r, bp.caccx,bp.caccy,bp.caccz);
  ledctrl.setArtnet(ba.getArtnetData());
  ledData = ledctrl.updateLED();
  
  
  draw_led(ledData);
  bp.sendLEDdata(ledData);  
}

void draw() 
{
  if(1==frameCount) 
  {
     surface.setLocation(bs.winx,bs.winy-28); //That '28' is the height of the menu bar... Ugly fudge but apparently hard to fix...
  }
  
  if (isLive)
    background(0);
  else
    background(255,0,0);
    
  if (bp.checkConnected())
  {
    if (bs.use2D)
    {
      draw_horizon();
    }
    else
    {
      draw_cube();
    }
    draw_labels();
  
    if (isCal)
    {
      calc_call_min_max();
    }
    else if (isMap)
    {
      //Do nothin...
    }
    else
    {
      if ((bp.checkConnected()) && (isLive))
      { 
        //Update midi and LED and Artnet 30 times/sec to limit data throughput
        if ((millis() - lastUpdate)>33)
        {
          lastUpdate = millis();
          bm.update_midi();
          if (bs.artnet_en)
          {
            ba.send_artnet();
            //Just a Test...
            send_led();
          }
        }
      }
    }
    
    //nake sure network is kept alive
    if (bp.checkConnected())
    {
      bp.send_ping();
      bp.check_timeout();  
    }
  }
  else
  {
    fill(255,0,0);
    stroke(255,0,0);
    line(0,0,width,height);
    line(width,0,0,height);

    int w = (int)textWidth("NO CONNECTION");
    fill(0,0,0);
    stroke(255,0,0);
    rect(width/2-(w/2)-4,height/2-13-50,w+8,20);
    fill(255,0,0);   
    stroke(255,0,0);
    text("NO CONNECTION",width/2-(w/2),height/2-50);
    bp.reconnect(bs.network_en);
  }

  PVector loc = getWindowLocation(P3D);
  bs.winx = (int)loc.x;
  bs.winy = (int)loc.y;
}


void calc_call_min_max()
{
  if (bp.cp<bs.minp)
    bs.minp = bp.cp;
  if (bs.maxp<bp.cp)
    bs.maxp=bp.cp;
  if (bp.cr< bs.minr)
    bs.minr = bp.cr;
  if (bs.maxr<bp.cr)
    bs.maxr= bp.cr;
  if (bp.cy<bs.miny)
    bs.miny = bp.cy;
  if (bs.maxy<bp.cy)
    bs.maxy=bp.cy;
}


void keyPressed() 
{
     if (bp.checkConnected())
   {
    if (key == 'm')
    {
      if (isCal == false)
        isMap = !isMap;
    }
    
    if (isMap)
    {
      //Send midid map key here
      bm.send_midi_map(key);
        
    }
    else
    {
      if (key == 'r')
      {
        if (isCal)
        {
          isCal = false;
          bs.normalize_range_settings();
          bs.save_settings();
        }
        else
        {
          isCal = true;
          bs.clear_cal_min_max();
        }
      }      
    }
   }
}
   
private void prepareExitHandler () 
{

Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() 
{
  public void run () 
  {
    System.out.println("SHUTDOWN HOOK");
    bs.save_settings();
  }
}));
}

void serialEvent(Serial myPort) 
{  
  bp.serialEvent(myPort);
}

public void onUdpMessageRecieved(SocketAddress client, byte[] message)
{
  bp.onUdpMessageRecieved(client,message);
}
