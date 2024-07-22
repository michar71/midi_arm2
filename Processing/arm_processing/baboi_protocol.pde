
//Really all the comm init and comm methods for the baboi should go in here...


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



baboi_protocol()
{
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
}

void sendSetupRequest(int setupID)
{
    String Setup = String.format("%c:%d\n", ID_SETUP,setupID);
    myPort.write(Setup);
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
}
  
}
