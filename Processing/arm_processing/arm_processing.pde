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
import com.bckmnn.udp.*;
import java.net.InetSocketAddress;
import java.net.InetAddress;
import java.net.SocketAddress;

//Data Sentences
char ID_DATA = 'D';   //Mition Data
char ID_QUERY = 'Q';  //Info Query
char ID_INFO = 'I';   //Answer to Query
char ID_SETUP = 'S';  //Setup Parameters
char ID_PING = 'P';  //PING to reset timeout counter in uC 
int baboi_port = 2255;

UDPHelper udp;
ControlP5 cp5;
ArtNetClient artnet;
byte[] dmxData = new byte[512];

Serial myPort;                  // The serial port
String my_port = "/dev/cu.usbmodem145101";        // choose your port
//String my_port = "/dev/tty.MIDIARM";        // choose your port
float p, r, y;
float cp,cr,cy;
float npo = 0;
float nro = 0;
float nyo = 0;

float accx,accy,accz;
int tension_ch1,tension_ch2,tension_ch3,tension_ch4;
float minp,maxp,minr,maxr,miny,maxy;
boolean isCal;
boolean isMap;
MidiBus myBus; // The MidiBus
String ID;
int pos;


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


//boolean crossp = false;
//boolean crossr = false;
//boolean crossy = false;

boolean artnet_en =false;
boolean network_en =false;

int lastUpdate = 0;
int lastSerial = 0;
int winx;
int winy;

int min_ver = 0;
int maj_ver = 0;
String deviceName = "UNKNOWN";
boolean isValidDevice=false;
PVector loc;



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
  
  int maxping = 5;
  
   for(int ii=0;ii<maxping;ii++)
   {
     String id_query = String.format("%c\n", ID_QUERY);
     myPort.write(id_query);
     println("Query Sent "+ii);
     delay(200);

     //Need to be able to identify different types of BABOIs
     if (isValidDevice)
        return true;
   }
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

boolean try_to_connect_wifi()
{
  int maxping = 5;
 
  
  for(int ii=0;ii<maxping;ii++)
  {
    String id_query = String.format("%c\n", ID_QUERY);
    try
    {

      //SocketAddress all = new InetSocketAddress(InetAddress.getByName("192.168.1.1"),baboi_port);
      //udp.sendMessage(UDPHelper.bytesFromString(id_query),all);
        
      SocketAddress all = new InetSocketAddress(InetAddress.getByName("255.255.255.255"),baboi_port);
      udp.sendMessage(UDPHelper.bytesFromString(id_query),all);   
        
     } 
     catch(Exception e)
     {
       e.printStackTrace();
     }
     println("Wifi Query Sent "+ii);
     delay(200);

    //Need to be able to identify different types of BABOIs
    if (isValidDevice)
       return true;
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
    
    //Fuckit... Just send it to everybody ;-)
    artnet.broadcastDmx(0, 0, dmxData);
  }
  
}

void Network(boolean theFlag) 
{
  network_en = theFlag;
  save_settings();    
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
     .setPosition(width-44,height - 60)
     .setSize(18,18)
     .setLabel("ArtNet")
     .setBroadcast(true)
     ;   
     
    cp5.addToggle("Network")
     .setBroadcast(false)
     .setValue(network_en)
     .setPosition(width-44,height - 30)
     .setSize(18,18)
     .setLabel("Network")
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
  
  udp = new UDPHelper(this);
  udp.setLocalPort(baboi_port);
  udp.startListening();
  
  float p_noise = 4;//2
  float s_noise = 16;//32
  float e_error = 128;//128
 
  KalFilterP = new kalman(p_noise,s_noise,e_error,0.0);
  KalFilterR = new kalman(p_noise,s_noise,e_error,0.0);  
  KalFilterY = new kalman(p_noise,s_noise,e_error,0.0);  
  
  smooth();
}

int c;

public void Test(int theValue) {
  if (myPort != null)
        myPort.write(ID_SETUP);
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

      splitp = json.getBoolean("splitp");
      splitr = json.getBoolean("splitr");
      splity = json.getBoolean("splity");
      
      artnet_en = json.getBoolean("artnet");
      network_en = json.getBoolean("network");
      
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
  
  json.setBoolean("splitp",splitp);    
  json.setBoolean("splitr",splitr);  
  json.setBoolean("splity",splity);  
  
  json.setBoolean("artnet",artnet_en);
  json.setBoolean("network",network_en);

  json.setInt("winx",winx);  
  json.setInt("winy",winy);  
  
  saveJSONObject(json,"setup.json");
}


void show_map_text()
{
  fill(255);
  textAlign(LEFT);
  text("Mapping Keys:",20,height-80);
  text("Pitch=1,Roll=2,Yaw=3",20,height-64);
  text("P-Split=4,R-Split=5,=Y-Split=6",20,height-48);
  text("Button A=7,Button B=8",20,height-32);
  text("Finger 1=9,Finger 2=0, Finger 3 = -",20,height-16);
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
  
  //Send Tension midi data... We only send it if it actually exists (not -1...)
  if (tension_ch1 != -1)
  {
    ControlChange change = new ControlChange(0, 9, tension_ch1/2);
    myBus.sendControllerChange(change);
  }
  if (tension_ch2 != -1)
  {
    ControlChange change = new ControlChange(0, 10, tension_ch2/2);
    myBus.sendControllerChange(change);
  }
  if (tension_ch3 != -1)
  {
    ControlChange change = new ControlChange(0, 11, tension_ch3/2);
    myBus.sendControllerChange(change);
  }
  if (tension_ch4 != -1)
  {
    ControlChange change = new ControlChange(0, 12, tension_ch4/2);
    myBus.sendControllerChange(change);
  }  
}

void send_artnet()
{
    // fill dmx array
    int val = 0;



    val =(int)map(cr,minr, maxr, 0,255);
    val = limit(val,0,255);
    dmxData[0] = (byte) val;
    
    val =(int)map(cp,minp, maxp, 255,0);
    val = limit(val,0,255);
    dmxData[1] = (byte) val;
    
    val =(int)map(cy,miny, maxy, 0,255);
    val = limit(val,0,255);
    dmxData[2] = (byte) val;
    
    val = (int)map(abs(accx),0,6,0,255);    
    dmxData[3] = (byte) val; 

    val = (int)map(abs(accy),0,6,0,255);    
    dmxData[4] = (byte) val; 

    val = (int)map(abs(accz),0,6,0,255);    
    dmxData[5] = (byte) val; 

    //Send glove data if it exists
    if (tension_ch1 != -1)
    {
      dmxData[6] = (byte)tension_ch1;
    }
    else
    {
      dmxData[6] = (byte) 0; 
    }
    if (tension_ch2 != -1)
    {
      dmxData[7] = (byte)tension_ch2;
    }
    else
    {
      dmxData[7] = (byte) 0; 
    }
    if (tension_ch3 != -1)
    {
      dmxData[8] = (byte)tension_ch3;
    }
    else
    {
      dmxData[8] = (byte) 0; 
    }
    if (tension_ch4 != -1)
    {
      dmxData[9] = (byte)tension_ch4;
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
  if (tension_ch1 != -1)
  {
    translate(-180, 0, -100); 
    box(50, tension_ch1, 50);
    translate(180, 0, 100); 
  }
  if (tension_ch2 != -1)
  {
    translate(180, 0, -100); 
    box(50, tension_ch2, 50);
    translate(-180, 0, 100); 
  }
  
  if (tension_ch3 != -1)
  {
    translate(-180, 0, -200); 
    box(50, tension_ch3, 50);
    translate(180, 0, 200); 
  }
  if (tension_ch4 != -1)
  {
    translate(180, 0, -200); 
    box(50, tension_ch4, 50);
    translate(-180, 0, 200); 
  }
  
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


void check_timeout()
{
  int timeout = 3000;
  int current_time = millis();
  if ((current_time - lastSerial) > timeout)
  {
    // Clear the buffer, or available() will still be > 0
    try
    {
      if (myPort != null)
      {
        myPort.clear();
        // Close the port
        myPort.stop();
      }
    }
    catch (Exception e)
    {
      println("Serial EXCEPTION");
    }
    println("TIMEOUT");
    isValidDevice = false;
    isConnected = false;
  }
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
   
    if (isConnected)
      check_timeout();  

  }
  else
  {
    fill(255,0,0);
    stroke(255,0,0);
    line(0,0,width,height);
    line(width,0,0,height);
    text("NO CONNECTION",width/2,height/2);
    isConnected = try_connect_usb_modem();
    if (network_en)
    {
      if (isConnected == false)
        isConnected = try_to_connect_wifi();
    }
  }

  loc = getWindowLocation(P3D);
  winx = (int)loc.x;
  winy = (int)loc.y;
}

void clear_cal_min_max()
{
  minp = 65535;
  maxp = -65535;
  minr = 65535;
  maxr = -65535;  
  miny = 65535;
  maxy = -65535; 
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

}

void process_received_string(String myString)
{
  float v1,v2,v3,v4;
  
  myString = trim(myString);
  String[] list = split(myString, ':');
  println(myString);
  
  if (list[0].contains(String.valueOf(ID_INFO)))
  {  
    deviceName = list[1];
    maj_ver = parseInt(list[2]);
    min_ver = parseInt(list[3]);
    ID = list[4];
    pos = parseInt(list[5]);
    println(deviceName+":"+maj_ver+"."+min_ver+" ID:"+ID+" POS:"+pos);
    isValidDevice = true;
    return;
  }
  else if (list[0].contains(String.valueOf(ID_DATA)))
  {
    float sensors[] = float(list);
    v1 = sensors[11];
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
      
      p = KalFilterP.getFilteredRADValue(p);
      r = KalFilterR.getFilteredRADValue(r);     
      y = KalFilterY.getFilteredRADValue(y);
      
      cp = p;
      cy = y;
      cr = r;
      
      accx = sensors[4];
      accy = sensors[5];
      accz = sensors[6];   
      
      tension_ch1 = int(sensors[7]);   
      tension_ch2 = int(sensors[8]);  
      tension_ch1 = int(sensors[9]);   
      tension_ch2 = int(sensors[10]);  
       
      v2 = sensors[12];
      v3 = sensors[13];
      v4 = sensors[14];  
      
      
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
    
  }


  //println("roll: " + r + " pitch: " + p + " yaw: " + y + "\n"); //debug
  lastSerial = millis();

}


//This sould receive boradcast and unicast....
public void onUdpMessageRecieved(SocketAddress client, byte[] message)
{
  String messageString = UDPHelper.stringFromBytes(message);
  //println(client + " sent you this message: " + messageString);
  process_received_string(messageString);
}


void serialEvent(Serial myPort) 
{  
  String myString = myPort.readStringUntil('\n'); 
  process_received_string(myString);
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
      if (key =='0')
      {
          ControlChange change1 = new ControlChange(0, 10, 1);
          myBus.sendControllerChange(change1);
      }    
      if (key =='-')
      {
          ControlChange change1 = new ControlChange(0, 11 , 1);
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
