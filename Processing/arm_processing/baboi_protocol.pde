
//Really all the comm init and comm methods for the baboi should go in here...
import com.bckmnn.udp.*;
import java.net.InetSocketAddress;
import java.net.InetAddress;
import java.net.SocketAddress;



class baboi_protocol{
//Data Sentences
char ID_DATA = 'D';   //Mition Data
char ID_QUERY = 'Q';  //Info Query
char ID_INFO = 'I';   //Answer to Query
char ID_SETUP = 'S';  //Setup Parameters (S:0 = Acc/Gyro Cal, S:1 = Glove Cal
char ID_PING = 'P';  //PING to reset timeout counter in uC 
char ID_LED = 'L';
  
//Put all parsed values here
float cp,cr,cy;
float accx,accy,accz;
int tension_ch1,tension_ch2;
boolean b_A_state = false;
boolean b_B_state = false;
boolean b_C_state = false;

String ID;
int pos;
int min_ver = 0;
int maj_ver = 0;
String deviceName = "UNKNOWN";

int baboi_port = 2255;

UDPHelper udp;
SocketAddress lastUDPclient = null;
Serial myPort;                  // The serial port
String my_port = "/dev/cu.usbmodem145101";        // choose your port

boolean isConnected = false;
boolean isValidDevice=false;


int lastSerial = 0;
int lastPing = 0;
PApplet theSketch;


baboi_protocol(PApplet sketch)
{
  theSketch = sketch;
  udp = new UDPHelper(theSketch);
  udp.setLocalPort(baboi_port);
  udp.startListening();
}
  
boolean checkConnected()
{
  return isConnected;
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
    //Build a list of all the USB Modems
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
    myPort = new Serial(theSketch, comport, 460800);
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


void reconnect(boolean nw)
{
      isConnected = try_connect_usb_modem();
    if (nw)
    {
      if (isConnected == false)
        isConnected = try_to_connect_wifi();
    }
}

boolean ping_usbmodem()
{
  
  int maxping = 10;
  
   for(int ii=0;ii<maxping;ii++)
   {
     String id_query = bp.build_query();
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
  int maxping = 10;
 
  
  for(int ii=0;ii<maxping;ii++)
  {
    String id_query = bp.build_query();
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

void send_ping()
{
  int pingRate = 1000;
  int current_time = millis();
  if ((current_time - lastPing) > pingRate)
  {
    lastPing = millis();
    sendPing();
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

public String build_query()
{
  return String.format("%c\n", ID_QUERY);
}
  
public void process_received_string(String myString)
{
  float v1,v2,v3,v4;
  
  myString = trim(myString);
  String[] list = split(myString, ':');
  //println(myString);
  
  if (list[0].contains(String.valueOf(ID_INFO)))
  {  
    deviceName = list[1];
    maj_ver = parseInt(list[2]);
    min_ver = parseInt(list[3]);
    //ID = list[4];
    //pos = parseInt(list[5]);
    println(deviceName+":"+maj_ver+"."+min_ver+" ID:"+ID+" POS:"+pos);
    isValidDevice = true;
    return;
  }
  else if (list[0].contains(String.valueOf(ID_DATA)))
  {
    float sensors[] = float(list);
    v1 = sensors[9];
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

      
      tension_ch1 = (int)KalFilterT0.getFilteredValue((float)tension_ch1);
      tension_ch2 = (int)KalFilterT1.getFilteredValue((float)tension_ch2);
      v2 = sensors[10];
      v3 = sensors[11];
      v4 = sensors[12];  
      
      
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


  //println("roll: " + r + " pitch: " + p + " yaw: " + y + "\n"); //debug
  lastSerial = millis();
}


//HMMM.. .These should either go via Serial or Wifi... or both....
//check if network is activated
//Get a link to UDP helper on init
//Send messages accordingly
void sendPing()
{
    String Ping = String.format("%c\n", ID_PING);
    myPort.write(Ping);
    
    if (lastUDPclient != null)
    {
      udp.sendMessage(UDPHelper.bytesFromString(Ping),lastUDPclient);   
    }
}

void sendSetupRequest(int setupID)
{
    String Setup = String.format("%c:%d\n", ID_SETUP,setupID);
    myPort.write(Setup);
    
    if (lastUDPclient != null)
    {
      udp.sendMessage(UDPHelper.bytesFromString(Setup),lastUDPclient);   
    }
}

void sendLEDdata(color[] data)
{
  StringBuilder stringBuilder = new StringBuilder();
  String ledData = "";
  
  if (data.length < 16)
    return;
    
  for (int ii=0; ii<16;ii++)
  {
    if (ii==15)
    {
      stringBuilder.append(str((int)((data[ii] >> 16) & 0xFF)));
      stringBuilder.append(",");
      stringBuilder.append(str((int)((data[ii] >>> 8 ) & 0xFF)));
      stringBuilder.append(",");
      stringBuilder.append(str((int)(data[ii] & 0xFF)));  
    }
    else
    {
      stringBuilder.append(str((int)((data[ii] >> 16) & 0xFF)));
      stringBuilder.append(",");
      stringBuilder.append(str((int)((data[ii] >>> 8 ) & 0xFF)));
      stringBuilder.append(",");
      stringBuilder.append(str((int)(data[ii] & 0xFF)));  
      stringBuilder.append(",");      
    }
  }
  ledData = stringBuilder.toString();
  String Led = String.format("%c:%s\n", ID_LED,ledData);
  myPort.write(Led);
  
  if (lastUDPclient != null)
  {
    udp.sendMessage(UDPHelper.bytesFromString(Led),lastUDPclient);   
  }
}


//This sould receive broadcast and unicast....
public void onUdpMessageRecieved(SocketAddress client, byte[] message)
{
  lastUDPclient = client;
  String messageString = UDPHelper.stringFromBytes(message);
  //println(client + " sent you this message: " + messageString);
  process_received_string(messageString);
}


void serialEvent(Serial myPort) 
{  
  String myString = myPort.readStringUntil('\n'); 
  bp.process_received_string(myString);
}
}
