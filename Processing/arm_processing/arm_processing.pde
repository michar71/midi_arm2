/*
This code is made for processing https://processing.org/

EVERYTHING SHOULD BE IN ORDER 
PITCH
ROLL
YAW


*/

import processing.serial.*;     // import the Processing serial library
import themidibus.*; //Import the library
import java.lang.*;
import java.util.*;
import controlP5.*;
import com.jogamp.newt.opengl.GLWindow;
import ch.bildspur.artnet.*;
import  java.nio.ByteBuffer;

ControlP5 cp5;
ArtNetClient artnet;
byte[] dmxData = new byte[512];

Serial myPort;                  // The serial port
String my_port = "/dev/cu.usbmodem145101";        // choose your port
//String my_port = "/dev/tty.MIDIARM";        // choose your port
float p, r, y;
float cp,cr,cy;
float accx,accy,accz;
float minp,maxp,minr,maxr,miny,maxy;
boolean isCal;
boolean isMap;
MidiBus myBus; // The MidiBus


kalman KalFilterP;
kalman KalFilterR;
kalman KalFilterY;

//Raw Debug Values
float a[] = {0,0,0};
float g[] = {0,0,0};
float m[] = {0,0,0};

int m1 = 0;
int m2 = 0;
int m3 = 0;
int m11 = 0;
int m22 = 0;
int m33 = 0;

int ignorelines = 0;
boolean isLive = true;
boolean b_A_state = false;
boolean b_B_state = false;
boolean b_C_state = false;
boolean last_A_state = false;
boolean last_B_state = false;
boolean last_C_state = false;

boolean isConnected = false;
boolean splitp = false;
boolean splitr = false;
boolean splity = false;


boolean crossp = false;
boolean crossr = false;
boolean crossy = false;

boolean artnet_en =false;

int lastUpdate = 0;
int lastSerial = 0;
int winx;
int winy;

int min_ver = 0;
int maj_ver = 0;
String deviceName = "UNKNOWN";
boolean isValidDevice=false;


float fourBytesToFloat(byte b1, byte b2, byte b3, byte b4)
{
  byte [] data = new byte[] {1,2,3,4};
  data[0] = b1;
  data[1] = b2;
  data[2] = b3;
  data[3] = b4; 
  ByteBuffer b = ByteBuffer.wrap(data);
  return b.getFloat();
}

boolean get_usbmodem_list(ArrayList<String> list)
{

  String substring = "usbmodem";
   
  try 
  {
    //printArray(Serial.list());
    int i = Serial.list().length;
    if (i != 0) 
    {
    //Buil a list of all the USB Modems
      for (int j = 0; j < i;j++) 
      {
        if (Serial.list()[j].contains(substring) == true)
        {
          list.add(Serial.list()[j]);
        }
      }
      println(list);
      return true;
    }
    else
    {
      println("No Serial Port Found");
      return false;
    }
    
  }
  catch (Exception e)
  { //Print the type of error
    println("Serial List Error:", e);
    return false;  //Tried to connect but no success... Maybe already used?
  }
}


boolean try_to_open(String comport)
{
  try
  {
    if (myPort !=null)
    {
      myPort.stop();
      myPort = null;
    }
    myPort = new Serial(this, comport, 230400);
    if (myPort != null)
    {
      myPort.bufferUntil('\n');
      return true;
    }
    else 
    {
      return false; //No Serial Port device detected at all...
    }
  }
  catch (Exception e)
  { //Print the type of error
    println("Serial Open Error:", e);
    return false;  //Tried to connect but no success... Maybe already used?
  }  
}



boolean ping_usbmodem()
{
  
  int maxping = 20;
  isValidDevice = false;
   for(int ii=0;ii<maxping;ii++)
   {
     myPort.write('Q');
     println("Query Sent"+ii);
     delay(100);
     //We need to figure out how to do this right
     //Need to be ablew to identify different types of BABOIs
     if (isValidDevice)
        return true;
   }
   isValidDevice = true;
   return false;
}


boolean try_connect_usb_modem()
{
  boolean hasList = false;
  boolean isOpen = false;

  ArrayList<String> Seriallist = new ArrayList<String>();
  //Build a list of all USB Modems
  
  hasList = get_usbmodem_list(Seriallist);
  if (hasList == false)
    return false;
  
 //Loop Through List
 for (int ii = 0;ii < Seriallist.size();ii++)
 {
   isOpen = false;
   //Try to open Serial Port
   isOpen = try_to_open(Seriallist.get(ii));
   if (isOpen)
   {
     return ping_usbmodem();
   }
 }
 return false;
}

void Splitp(boolean theFlag) 
{
  splitp = theFlag;
  save_settings();
}

void Splitr(boolean theFlag) 
{
  splitr = theFlag;
  save_settings();    
}

void Splity(boolean theFlag) 
{
  splity = theFlag;
  save_settings();   
}

void Artnet(boolean theFlag) 
{
  artnet_en = theFlag;
  save_settings();   
  
  if (artnet_en == false)
  {
    dmxData[0] = (byte) 0; 
    dmxData[1] = (byte) 0; 
    dmxData[2] = (byte) 0; 
    dmxData[3] = (byte) 0; 
    dmxData[4] = (byte) 0; 
    dmxData[5] = (byte) 0;     
    // send dmx to localhost
    //artnet.unicastDmx("127.0.0.1", 0, 0, dmxData);
    
    //Fuckit... Jsut send it to everybody ;-)
    artnet.broadcastDmx(0, 0, dmxData);
  }
  
}

void setup() {
  size(400, 260,P3D);
  surface.setTitle("BABOI CONTROL");
  surface.setResizable(false);
  
  
  cp5 = new ControlP5(this);
  prepareExitHandler();
 
    load_settings();
    //setLocation(winx, winy);
    
  
  // create a new button with name 'buttonA'
  cp5.addButton("Range")
     .setBroadcast(false)
     .setValue(0)
     .setPosition(width-110,10)
     .setSize(100,18)
     .setBroadcast(true)
     ;
  
  // and add another 2 buttons
  cp5.addButton("Map")
     .setBroadcast(false)
     .setValue(100)
     .setPosition(width-110,30)
     .setSize(100,18)
     .setBroadcast(true)
     ;
  
    cp5.addToggle("Splitp")
     .setBroadcast(false)
     .setValue(splitp)
     .setPosition(width-110,60)
     .setSize(18,18)
     .setLabel("Split P")
     .setBroadcast(true)
     ;
  
    cp5.addToggle("Splitr")
     .setBroadcast(false)
     .setValue(splitr)
     .setPosition(width-70,60)
     .setSize(18,18)
     .setLabel("Split R")
     .setBroadcast(true)
     ;
     
    cp5.addToggle("Splity")
     .setBroadcast(false)
     .setValue(splity)
     .setPosition(width-30,60)
     .setSize(18,18)
     .setLabel("Split Y")
     .setBroadcast(true)
     ;     
     
    cp5.addToggle("Artnet")
     .setBroadcast(false)
     .setValue(artnet_en)
     .setPosition(width-34,height - 40)
     .setSize(18,18)
     .setLabel("ArtNet")
     .setBroadcast(true)
     ;   
     
    // create a new button with name 'buttonA'
    /*
  cp5.addButton("Test")
     .setBroadcast(false)
     .setValue(0)
     .setPosition(5,height-20)
     .setSize(50,18)
     .setBroadcast(true)
     ;
     */
     
  MidiBus.list();
  myBus = new MidiBus(this, -1, "Bus 1"); // Create a new MidiBus with no input device and the default MacOS Midi Distributor as output
   
  artnet = new ArtNetClient(null);
  artnet.start();
  
  float p_noise = 2;
  float s_noise = 32;//32
  float e_error = 128;
 
  KalFilterP = new kalman(p_noise,s_noise,e_error,0.0);
  KalFilterR = new kalman(p_noise,s_noise,e_error,0.0);  
  KalFilterY = new kalman(p_noise,s_noise,e_error,0.0);  
  
  smooth();
}

int c;

public void Test(int theValue) {
  if (myPort != null)
        myPort.write('S');
}

// function colorA will receive changes from 
// controller with name colorA
public void Range(int theValue) {
        if (isCal)
        {
          isCal = false;
          save_settings();
        }
        else
        {
          isCal = true;
          clear_cal_min_max();
        }
}

// function colorB will receive changes from 
// controller with name colorB
public void Map(int theValue) {
        if (isCal == false)
        isMap = !isMap;
}

void load_settings()
{
  JSONObject json;
  json = loadJSONObject("setup.json");
  
  if (json != null)
  {
    try
    {
      maxp = json.getFloat("maxp");
      minp = json.getFloat("minp");
      maxr = json.getFloat("maxr");
      minr = json.getFloat("minr");
      maxy = json.getFloat("maxy");
      miny = json.getFloat("miny");      

      crossp = json.getBoolean("crossp");
      crossr = json.getBoolean("crossr");
      crossy = json.getBoolean("crossy");

      splitp = json.getBoolean("splitp");
      splitr = json.getBoolean("splitr");
      splity = json.getBoolean("splity");
      
      artnet_en = json.getBoolean("artnet");
      
      winx = json.getInt("winx");        
      winy = json.getInt("winy");      
    }
    
    catch (Exception e)
    { //Print the type of error
      println("Error loading Preset", e);
      return;  //Tried to connect but no success... Maybe already used?
    }
      
  }
}

void save_settings()
{
  JSONObject json = new JSONObject();

  json.setFloat("maxp",maxp);
  json.setFloat("minp",minp);
  json.setFloat("maxr",maxr);
  json.setFloat("minr",minr);
  json.setFloat("maxy",maxy);
  json.setFloat("miny",miny);
  
  json.setBoolean("crossp",crossp);
  json.setBoolean("crossr",crossr);
  json.setBoolean("crossy",crossy);
  
  json.setBoolean("splitp",splitp);    
  json.setBoolean("splitr",splitr);  
  json.setBoolean("splity",splity);  
  
  json.setBoolean("artnet",artnet_en);

  json.setInt("winx",winx);  
  json.setInt("winy",winy);  
  
  saveJSONObject(json,"setup.json");
}


  
void show_map_text()
{
  fill(255);
  textAlign(LEFT);
  text("Mapping Keys:",20,height-80);
  text("Pitch=1,Roll=2,Yaw=3",20,height-60);
  text("P-Split=4,R-Split=5,=Y-Split=6",20,height-40);
  text("Button A=7,Button B=8, Button C=9",20,height-20);
}


void show_acceleration()
{
  int r = (int)map(accx,-4,4,64,255);
  int g = (int)map(accy,-4,4,64,255);
  int b = (int)map(accz+1,-4,4,64,255);
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
  text("P:"+nf(cp,0,2),5,10);
  text("R:"+nf(cr,0,2),5,22);
  text("Y:"+nf(cy,0,2),5,34);
  text(nf(minp,0,2)+"/"+nf(maxp,0,2),55,10);
  text(nf(minr,0,2)+"/"+nf(maxr,0,2),55,22);
  text(nf(miny,0,2)+"/"+nf(maxy,0,2),55,34);
  
  if (splitp)
    text(m1+"/"+m11,120,10);
  else
    text(m1,120,10);
    
  if (splitr)  
    text(m2+"/"+m22,120,22);
  else
    text(m2,120,22);
  
  if (splity)
    text(m3+"/"+m33,120,34);  
  else
    text(m3,120,34);  
      
      
  if (b_A_state)
    text("A", 5,46);
  
  if (b_B_state)
    text("B", 20,46);
    
  if (b_C_state)
    text("C", 35,46);
    
  text(deviceName+" V"+maj_ver+"."+min_ver, 5,58);  

    
  if (isMap)
         show_map_text();
         
  
  hint(ENABLE_DEPTH_TEST);
}

void delay(int time) {
  int current = millis();
  while (millis () < current+time) Thread.yield();
}

int limit(int in, int min, int max)
{
  if (in > max)
    return max;
  if (in < min)
    return min;
  
  return in;  
}

void send_midi()
{
  
  if (splitp)
  {
      float half = map(50,0,100, minp,maxp);
      if (cp < half)
      {
        m1 =(int)map(cp,minp, half, 127,0);
        m1 = limit(m1,0,127);
        m11 = 0;
      }
      else
      {
        m11 =(int)map(cp,half, maxp, 0,127);
        m11 = limit(m11,0,127);
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
    m1 =(int)map(cp,minp, maxp, 0,127);
    m1 = limit(m1,0,127);
    ControlChange change1 = new ControlChange(0, 1, m1);
    myBus.sendControllerChange(change1);
  }
  
  if (splitr)
  {
      float half = map(50,0,100, minr,maxr);
      if (cr < half)
      {
        m2 =(int)map(cr,minr, half, 127,0);
        m2 = limit(m2,0,127);
        m22 = 0;
      }
      else
      {
        m22 =(int)map(cr,half, maxr, 0,127);
        m22 = limit(m22,0,127);
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
    m2 =(int)map(cr,minr, maxr, 0,127);
    m2 = limit(m2,0,127);
    ControlChange change1 = new ControlChange(0, 2, m2);
    myBus.sendControllerChange(change1);
  }
  
  if (splity)
  {
      float half = map(50,0,100, miny,maxy);
      if (cy < half)
      {
        m3 =(int)map(cy,miny, half, 127,0);
        m3 = limit(m3,0,127);
        m33 = 0;
      }
      else
      {
        m33 =(int)map(cy,half, maxy, 0,127);
        m33 = limit(m33,0,127);
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
    m3 =(int)map(cy,miny, maxy, 0,127);
    m3 = limit(m3,0,127);
    ControlChange change1 = new ControlChange(0, 3, m3);
    myBus.sendControllerChange(change1);
  }
}

void send_artnet()
{
    // fill dmx array
    int val = 0;
    


    val =(int)map(cp,minp, maxp, 0,255);
    val = limit(val,0,255);
    dmxData[0] = (byte) val;

    val =(int)map(cr,minr, maxr, 0,255);
    val = limit(val,0,255);
    dmxData[1] = (byte) val;
    
    val =(int)map(cy,miny, maxy, 0,255);
    val = limit(val,0,255);
    dmxData[2] = (byte) val;
    
    val = (int)map(accx,-4,4,0,255);    
    dmxData[3] = (byte) val;
  
    val = (int)map(accy,-4,4,0,255);    
    dmxData[4] = (byte) val;
    
    val = (int)map(accz,-4,4,0,255);    
    dmxData[5] = (byte) val;    
    
    // send dmx to localhost
    //artnet.unicastDmx("127.0.0.1", 0, 0, dmxData);
    
    //Fuckit... Jsut send it to everybody ;-)
    artnet.broadcastDmx(0, 0, dmxData);
}

void send_buttons()
{

  if ((last_A_state == false) && (b_A_state == true))
  {
    last_A_state = b_A_state;
    ControlChange change1 = new ControlChange(0, 7, 1);
    myBus.sendControllerChange(change1);
  }

  else if ((last_A_state == true) && (b_A_state == false))
  {
    last_A_state = b_A_state;
    ControlChange change1 = new ControlChange(0, 7, 0);
    myBus.sendControllerChange(change1);
  }

  if ((last_B_state == false) && (b_B_state == true))
  {
    last_B_state = b_B_state;
    ControlChange change1 = new ControlChange(0, 8, 1);
    myBus.sendControllerChange(change1);
  }

  else if ((last_B_state == true) && (b_B_state == false))
  {
    last_B_state = b_B_state;
    ControlChange change1 = new ControlChange(0, 8, 0);
    myBus.sendControllerChange(change1);
  }
  
  if ((last_C_state == false) && (b_C_state == true))
  {
    last_C_state = b_C_state;
    ControlChange change1 = new ControlChange(0, 9, 1);
    myBus.sendControllerChange(change1);
  }

  else if ((last_C_state == true) && (b_C_state == false))
  {
    last_C_state = b_C_state;
    ControlChange change1 = new ControlChange(0, 9, 0);
    myBus.sendControllerChange(change1);
  }  
}



void draw_cube()
{

  float dirY = (((float)height/4*3) / float(height) - 0.5) * 2;
  float dirX = (((float)width/4*3) / float(width) - 0.5) * 2;
  directionalLight(204, 204, 204, -dirX, -dirY, -1); //Why is this linked to the mouse? 
  noStroke();
  pushMatrix();
  show_acceleration();
  translate(width/2, height/2);
  /*
  PMatrix3D rm = new PMatrix3D();
  rm = toMatrix(rm,qx,qy,qz,qw);
  applyMatrix(rm);
  */

  drawDebugVectors();
  
  rotateY(y);//yaw
  rotateX(p);//pitch
  rotateZ(r);//roll

  box(100, 100/2, 100*2);
  
  popMatrix();
}

void drawDebugVectors()
{
  stroke(255,0,0);
  line(0,0,0,a[0] * 100, a[1] * 100, a[2] * 100);
  stroke(0,255,0);
  line(0,0,0,g[0] * 100, g[1] * 100, g[2] * 100);
  stroke(0,0,255);
  line(0,0,0,m[0] * 100, m[1] * 100, m[2] * 100);  
}


void update_midi()
{

  //limit update rate  
  if ((millis() - lastUpdate)>30)
  {
    lastUpdate = millis();
    send_midi();
    send_buttons();
    if (artnet_en)
    {
      send_artnet();
    }
  }
}


boolean check_timeout()
{
  int timeout = 3500;
  int current_time = millis();
  if ((current_time - lastSerial) > timeout)
  {
    // Clear the buffer, or available() will still be > 0
    myPort.clear();
    // Close the port
    myPort.stop();
    println("TIMEOUT");
    return false;
  }
  else
    return true;
}

void draw() 
{
  if(1==frameCount) 
  {
     surface.setLocation(winx,winy-28); //That '28' is the height of the menu bar... Ugly fudge but apparently hard to fix...
  }
  
  if (isLive)
    background(0);
  else
    background(255,0,0);
  //if (isConnected)
  //  println("CONNECTED");
  //else
  //  println("NOT CONNECTED");
    
  if (isConnected)
  {
    draw_cube();
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
      if ((isConnected) && (isLive))
        update_midi();
    }
   
     isConnected = check_timeout();  
  }
  else
  {
    fill(255,0,0);
    stroke(255,0,0);
    line(0,0,width,height);
    line(width,0,0,height);
    text("NO CONNECTION",width/2,height/2);
    isConnected = try_connect_usb_modem();
  }
  
    GLWindow glw = (GLWindow)surface.getNative();
    winx = glw.getX();
    winy = glw.getY();
}

void clear_cal_min_max()
{
  minp = 65535;
  maxp = -65535;
  minr = 65535;
  maxr = -65535;  
  miny = 65535;
  maxy = -65535; 
  crossp = false;
  crossr = false;
  crossy = false;  
}

void calc_call_min_max()
{
  if (cp<minp)
    minp = cp;
  if (maxp<cp)
    maxp=cp;
  if (cr<minr)
    minr = cr;
  if (maxr<cr)
    maxr=cr;
  if (cy<miny)
    miny = cy;
  if (maxy<cy)
    maxy=cy;
    
  //Zero Crossover Detection
  if (((minp <+ -3.1) || (maxp >= 3.1)) && (crossp == false))
  {
    crossp = true;
    minp = 65535;
    maxp = -65535;  
  }  
  if (((minr <= -3.1) || (maxr >= 3.1)) && (crossr == false))
  {
    crossr = true;
    minr = 65535;
    maxr = -65535;  
  }
  if (((miny <= -3.1) || (maxy >+ 3.1)) && (crossy == false))
  {
    crossy = true;
    miny = 65535;
    maxy = -65535;  
  }  
}

void serialEvent(Serial myPort) 
{

  float v1,v2,v3,v4;
  String myString = myPort.readStringUntil('\n');
  //println(myString);
  if (ignorelines == 0)
  {
    myString = trim(myString);

    String[] list = split(myString, ':');
    
    if (list[0].contains("BABOI"))
    {
      
      deviceName = list[0];
      maj_ver = parseInt(list[1]);
      min_ver = parseInt(list[2]);
      println(deviceName+":"+maj_ver+"."+min_ver);
      isValidDevice = true;
      return;
    }
    else if (list[0].contains("C"))
    {
      float sensors[] = float(list);
      v1 = sensors[7];
      if (v1 == 0)
      {
        isLive = false;
        b_A_state = false;
        b_B_state = false;
        b_C_state = false;
      }
      else
      {
        isLive = true;
        y = sensors[1];
        p = sensors[2];
        r = sensors[3];  
          
        p = KalFilterP.getFilteredValue(p);
        r = KalFilterR.getFilteredValue(r);     
        y = KalFilterY.getFilteredValue(y);
        
        cp = p;
        cr = r;
        cy = y;
        
        if (crossp)
        {
          if (cp < 0)
            cp = cp + 2*PI;
        }           
        if (crossr)
        {
          if (cr < 0)
            cr = cr + 2*PI;
        }
        if (crossy)
        {
          if (cy < 0)
            cy = cy + 2*PI;
        }   
        
        
        accx = sensors[4];
        accy = sensors[5];
        accz = sensors[6];   
        
        
        v2 = sensors[8];
        v3 = sensors[9];
        v4 = sensors[10];  
        
        
        if (v2 == 0)
          b_A_state = false;
        else
          b_A_state = true;
          
        if (v3 == 0)
          b_B_state = false;
        else
          b_B_state = true;
          
        if (v4 == 0)
          b_C_state = false;
        else
          b_C_state = true;         
      }
    }
    else if (list[0].contains("D"))
    {
      float sensors[] = float(list);
      
      a[0] = sensors[1];
      a[1] = sensors[2];
      a[2] = sensors[3];
      g[0] = sensors[4];
      g[1] = sensors[5];
      g[2] = sensors[6];
      m[0] = sensors[7];
      m[1] = sensors[8];
      m[2] = sensors[9];
      
      normalizeVectors();
    }
  }
  else
  {  
    ignorelines--;
  }
  //println("roll: " + xx + " pitch: " + yy + " yaw: " + zz + "\n"); //debug
  lastSerial = millis();

}

void normalizeVectors()
{
  float norm;
  float tmp;
  
  tmp = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
  if (tmp > 0.0) {
    // Normalise accelerometer (assumed to measure the direction of gravity in body frame)
    norm = 1.0 / sqrt(tmp);
    a[0] *= norm;
    a[1] *= norm;
    a[2] *= norm;
  }
  

  tmp = g[0] * g[0] + g[1] * g[1] + g[2] * g[2];
  if (tmp > 0.0) {
    // Normalise accelerometer (assumed to measure the direction of gravity in body frame)
    norm = 1.0 / sqrt(tmp);
    g[0] *= norm;
    g[1] *= norm;
    g[2] *= norm;
  }

  
  tmp = m[0] * m[0] + m[1] * m[1] + m[2] * m[2];
  if (tmp > 0.0) {
    // Normalise accelerometer (assumed to measure the direction of gravity in body frame)
    norm = 1.0 / sqrt(tmp);
    m[0] *= norm;
    m[1] *= norm;
    m[2] *= norm;
  }  
}



void keyPressed() 
{
   if(isConnected)
   {
    if (key == 'm')
    {
      if (isCal == false)
        isMap = !isMap;
    }
    
    if (isMap)
    {
      if (key =='1')
      {
          ControlChange change1 = new ControlChange(0, 1, 63);
          myBus.sendControllerChange(change1);
      }
      if (key =='2')
      {
          ControlChange change1 = new ControlChange(0, 2, 63);
          myBus.sendControllerChange(change1);
      }
      if (key =='3')
      {
          ControlChange change1 = new ControlChange(0, 3, 63);
          myBus.sendControllerChange(change1);
      }   
      if (key =='4')
      {
          ControlChange change1 = new ControlChange(0, 4, 1);
          myBus.sendControllerChange(change1);
      }
      if (key =='5')
      {
          ControlChange change1 = new ControlChange(0, 5, 1);
          myBus.sendControllerChange(change1);
      }
      if (key =='6')
      {
          ControlChange change1 = new ControlChange(0, 6, 1);
          myBus.sendControllerChange(change1);
      }       
      if (key =='7')
      {
          ControlChange change1 = new ControlChange(0, 7, 1);
          myBus.sendControllerChange(change1);
      }
      if (key =='8')
      {
          ControlChange change1 = new ControlChange(0, 8, 1);
          myBus.sendControllerChange(change1);
      }
      if (key =='9')
      {
          ControlChange change1 = new ControlChange(0, 9, 1);
          myBus.sendControllerChange(change1);
      }          
    }
    else
    {
      if (key == 'r')
      {
        if (isCal)
        {
          isCal = false;
          save_settings();
        }
        else
        {
          isCal = true;
          clear_cal_min_max();
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
    save_settings();
  }
}));
}