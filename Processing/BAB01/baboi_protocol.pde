
//Really all the comm init and comm methods for the baboi should go in here...
import com.bckmnn.udp.*;
import java.net.InetSocketAddress;
import java.net.InetAddress;
import java.net.SocketAddress;



class baboi_protocol{
  
//Filter  
kalman KalFilterP;
kalman KalFilterR;
kalman KalFilterY;
kalman KalFilterT0;
kalman KalFilterT1;
kalman KalFilterAccX;
kalman KalFilterAccY;
kalman KalFilterAccZ;  
  
  
//Data Sentences
char ID_DATA = 'D';   //Mition Data
char ID_QUERY = 'Q';  //Info Query
char ID_INFO = 'I';   //Answer to Query
char ID_SETUP = 'S';  //Setup Parameters (S:0 = Acc/Gyro Cal, S:1 = Glove Cal
char ID_PING = 'P';  //PING to reset timeout counter in uC 
char ID_LED = 'L';   //Send LED Data
char ID_REQUEST = 'R'; //Start Requesting streaming data after intial comm setup
  
//Put all parsed values here

private float p, r, y;
private float accx,accy,accz;
private int tension_ch1,tension_ch2;
float cp,cr,cy;
float caccx, caccy,caccz;
int ctension_ch1,ctension_ch2;

boolean b_A_state = false;
boolean b_B_state = false;
boolean b_C_state = false;
String UARTOverride = "";

String ID = "";
int pos = 0;
int min_ver = 0;
int maj_ver = 0;
int HW_ver = 0;
String deviceName = "UNKNOWN";

boolean writerActive = false;
PrintWriter outputFile;
boolean useLogger = false;

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


baboi_protocol(PApplet sketch,String UART)
{
    //More Filtering 
    float motion_p_noise = 4;//2
    float motion_s_noise = 16;//32
    float motion_e_error = 128;//128
    KalFilterP = new kalman(motion_p_noise,motion_s_noise,motion_e_error,0.0);
    KalFilterR = new kalman(motion_p_noise,motion_s_noise,motion_e_error,0.0);  
    KalFilterY = new kalman(motion_p_noise,motion_s_noise,motion_e_error,0.0);
    
    float tension_p_noise = 4;//2
    float tension_s_noise = 16;//32
    float tension_e_error = 128;//128
    KalFilterT0 = new kalman(tension_p_noise,tension_s_noise,tension_e_error,0.0);  
    KalFilterT1 = new kalman(tension_p_noise,tension_s_noise,tension_e_error,0.0);    

    //Less filtering
    float acc_p_noise = 8;//2
    float acc_s_noise = 16;//32
    float acc_e_error = 5;//128    
    KalFilterAccX = new kalman(acc_p_noise,acc_s_noise,acc_e_error,0.0);
    KalFilterAccY = new kalman(acc_p_noise,acc_s_noise,acc_e_error,0.0);
    KalFilterAccZ = new kalman(acc_p_noise,acc_s_noise,acc_e_error,0.0);
    
    theSketch = sketch;
    UARTOverride = UART;
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
     if (isValidDevice == true)
     {
          return true;
     }
     
     
     String id_query = bp.build_query();
     myPort.write(id_query);
     println("Query Sent "+ii);
     //delay(100);  //#TODO MH Is 200 a good value? Why does it sometimes take 10 attempts to reconnect? IS that delay on processing side or uC side?

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
      
  println("Available UARTs:");
  for (int ii = 0;ii < Seriallist.size();ii++)
    println(Seriallist.get(ii));


  if (0 == UARTOverride.compareTo(""))
  {
   println("Trying to Locate BAB01");
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
  }
  else
  {
      println("?Using UART Override");
     isOpen = false;
     //Try to open Serial Port
     isOpen = try_to_open(UARTOverride);
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
    if (isValidDevice == true)
      return true;
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
  int timeout = 600;
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
  
  
public void startWriter(String filen)
{
  outputFile = null;
  filen = filen + "_"+ str(year())+"_"+str(month())+"_"+str(day())+"_"+str(hour())+"-"+str(minute())+"-"+str(second());
  outputFile = createWriter(filen+".csv"); 
  if (outputFile !=null)
    enableWriter(true);
}

public void stopWriter()
{  
    if (outputFile != null)
    {
      outputFile.flush(); // Writes the remaining data to the file
      outputFile.close(); // Finishes the file
    }
    enableWriter(false);
}

public void writeData(String data)
{
  if (isWriterActive())
  {
    outputFile.println(data);
  }
}

public boolean isWriterActive()
{
  return writerActive;
}


public void enableWriter(boolean enable)
{
  writerActive = enable;
}

public void enableLogger(boolean enable)
{
  useLogger = enable;
}
  
  
public void process_received_string(String myString)
{
  float v1 = 0;
  float v2 = 0;
  float v3 = 0;
  float v4 = 0;
  
  myString = trim(myString);
  String[] list = split(myString, ':');

  
  lastSerial = millis();
  
  if (list[0].contains(String.valueOf(ID_INFO)))
  {  
    deviceName = list[1];
    maj_ver = parseInt(list[2]);
    min_ver = parseInt(list[3]);
    //ID = list[4];
    HW_ver = parseInt(list[4]);
    pos = parseInt(list[5]);
    println(deviceName+":"+maj_ver+"."+min_ver+" ID:"+HW_ver+" POS:"+pos);
    isValidDevice = true;
    
    //We got a valid data ID, start requesting Data
    sendDataRequest();
    return;
  }
  else if (list[0].contains(String.valueOf(ID_DATA)))
  {
    float sensors[] = float(list);
    v1 = sensors[9];
    
 
    if (v1 == 0)
    {
      if (isLive == true)
        stopWriter();
      isLive = false;
      b_A_state = false;
      b_B_state = false;
      b_C_state = false;
    }
    else
    {
      if ((useLogger) && (isLive == false))
        startWriter("PROCDATALOGGER");
      isLive = true;
      
      y = sensors[1];
      p = sensors[2];
      r = sensors[3];  
      cp = KalFilterP.getFilteredRADValue(p);
      cr = KalFilterR.getFilteredRADValue(r);     
      cy = KalFilterY.getFilteredRADValue(y);
      
      accx = sensors[4];
      accy = sensors[5];
      accz = sensors[6];   
      caccx = KalFilterAccX.getFilteredValue((float)accx);
      caccy = KalFilterAccY.getFilteredValue((float)accy);
      caccz = KalFilterAccZ.getFilteredValue((float)accz);
      
      tension_ch1 = int(sensors[7]);   
      tension_ch2 = int(sensors[8]);  
      ctension_ch1 = (int)KalFilterT0.getFilteredValue((float)tension_ch1);
      ctension_ch2 = (int)KalFilterT1.getFilteredValue((float)tension_ch2);
      
      
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
  
  writeData(str(lastSerial)+","+str(y)+","+str(p)+","+str(r)+","
                               +str(cy)+","+str(cp)+","+str(cr)+","
                               +str(accx)+","+str(accy)+","+str(accz)+","
                               +str(caccx)+","+str(caccy)+","+str(caccz)+","
                               +str(tension_ch1)+","+str(tension_ch2)+","
                               +str(ctension_ch1)+","+str(ctension_ch2)+","
                               +str(v2)+","+str(v3)+","+str(v4));
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

void sendDataRequest()
{
    String Request = String.format("%c\n", ID_REQUEST);
    myPort.write(Request);
    
    if (lastUDPclient != null)
    {
      udp.sendMessage(UDPHelper.bytesFromString(Request),lastUDPclient);   
    }
    
    println("Send Data Request");
}

void sendSetupRequest(int setupID)
{
    String Setup = String.format("%c:%d\n", ID_SETUP,setupID);
    myPort.write(Setup);
    
    if (lastUDPclient != null)
    {
      udp.sendMessage(UDPHelper.bytesFromString(Setup),lastUDPclient);   
    }
    
    println("Send Setup Request "+str(setupID));
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
  //println(client + ": " + messageString);
  process_received_string(messageString);
}


void serialEvent(Serial myPort) 
{  
  String myString = myPort.readStringUntil('\n'); 
  //println("UART: "+myString);
  bp.process_received_string(myString);
}


}
