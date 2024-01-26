#include <arduino.h>
#include "baboi_protocol.h"
#include "baboi_sensors.h"
#include "baboi_led.h"
#include "settings.h"
#include "main.h"

#ifdef WIFI
#include <AsyncUDP.h>

    AsyncUDP udp;
#endif

extern setup_t settings;

t_comm_channel currentCommChannel = COMM_SERIAL;
IPAddress comm_host_ip = IPAddress(0,0,0,0);


/*Combined use of Serial and Wifi
//-------------------------------

- Baboi listens both on Serial and Wifi on itintation of contact via 'Q' command
- If a serial command is received all engotiation happens via Serial
- If a Q command is received the 'I' Command will be send as a broadcast command
- Following D commands are send as a Unicast command to the adress of the host where the Q command originated from
*/ 


char send_data[SEND_DATA_LENGTH];
char receive_data[RECEIVE_DATA_LENGTH];

extern t_state state;
extern bool but_a_state;
extern bool but_b_state;
extern bool but_c_state;

extern String devicename;
extern int maj_ver;
extern int min_ver;

//Build Data string to send.
//Indivudal fields are seperated by ':'
//Format for Controller Data (C) commands is
//Command ID 'C'
//Device Name 'BABOI'
//Device FW Major
//Device FW Minor
//Unique Device ID (0..255), 0 = Single, 1 = left, 2 = right



void build_processing_data(bool senddata)
{
  float yaw_val = mpu_GetCurrentYaw();
  float pitch_val = mpu_GetCurrentPitch();
  float roll_val = mpu_GetCurrentRoll();  
  float ax_val = mpu_GetCurrentAX();
  float ay_val = mpu_GetCurrentAY();  
  float az_val = mpu_GetCurrentAZ();
  int16_t tension_ch1 = -1;
  int16_t tension_ch2 = -1;
  int16_t tension_ch3 = -1;
  int16_t tension_ch4 = -1;

  if (checkForGlove())
  {
    tension_ch1 = tension_get_ch(0);
    tension_ch2 = tension_get_ch(1);
    tension_ch3 = -1; //tension_get_ch(2);
    tension_ch4 = -1; //tension_get_ch(3);          
  }


  if(senddata)
    snprintf(send_data,SEND_DATA_LENGTH,"%c:%.2f:%.2f:%.2f:%.2f:%.2f:%.2f:%d:%d:%d:%d",ID_DATA,yaw_val,pitch_val,roll_val,ax_val,ay_val,az_val,tension_ch1,tension_ch2,tension_ch3,tension_ch4);
  else
    snprintf(send_data,SEND_DATA_LENGTH,"%c:0:0:0:0:0:0:0:0:0:0",ID_DATA);

    //#TODO Combine into one bitmap
    if (state == STATE_LIVE)
      strcat(send_data,":1");
    else
      strcat(send_data,":0");

    if (but_a_state)
      strcat(send_data,":1");
    else
      strcat(send_data,":0");

    if (but_b_state)
      strcat(send_data,":1");
    else
      strcat(send_data,":0");        

    if (but_c_state)
      strcat(send_data,":1");
    else
      strcat(send_data,":0");   
}

void send_processing_data(bool senddata)
{
  build_processing_data(senddata);
  if (currentCommChannel == COMM_SERIAL)
  {  
    Serial.println(send_data);  
  }
  else
  {
#ifdef WIFI
    AsyncUDPMessage message;
    message.printf("%s",send_data);

    //Sending everything as broadcast right now. Replace with unicast later    
    //udp.broadcastTo(message, UDP_BROADCAST_PORT);
    udp.sendTo(message, comm_host_ip, UDP_BROADCAST_PORT);
#endif  
  }
}

void build_info_data(void)
{
  snprintf(send_data,SEND_DATA_LENGTH,"%c:%s:%d:%d",ID_INFO,devicename,maj_ver,min_ver);
}

void send_info_data(void)
{
  build_info_data();  
  if (currentCommChannel == COMM_SERIAL)
  {  
    //We send this 10 times....
    for (int ii=0;ii<10;ii++)
    {
        Serial.println(send_data);
        delay(20);
    }
  }
  else
  {
#ifdef WIFI
    //We send this 10 times....
    for (int ii=0;ii<10;ii++)
    {
        AsyncUDPMessage message;
        message.printf("%s",send_data);    
        udp.broadcastTo(message, UDP_BROADCAST_PORT);
        delay(20);
    }
#endif 
  }
  //Mark State as connected
  setLED(0,0,64,0);
  setState(STATE_LIVE);
}


bool process_incoming_data(t_comm_channel commChannel)
{
    if (receive_data[0] == ID_QUERY)
    {
        //Set the comm channel active we got the message on
        currentCommChannel = commChannel;
        //Send back info data on comm channel
        send_info_data();
        return true;
    }
    else if (receive_data[0] == ID_SETUP)
    {
        return true;
    }
    return false;
}

void incoming_protocol_request(void)
{
  static uint8_t len = 0;  
  char incomingByte;
  static bool complete = false;

  if (Serial.available() > 0) 
  {
    // read the incoming byte:
    incomingByte = Serial.read();
    receive_data[len++] = incomingByte;

    //resetting
    if (len == RECEIVE_DATA_LENGTH-2) 
    {
      complete = true;
      receive_data[RECEIVE_DATA_LENGTH-1] = 0;
    }

    if (incomingByte == '\n')
    {
        complete = true;   
        receive_data[len++] = 0;
    }

    if (complete)
    {
        process_incoming_data(COMM_SERIAL);
        len = 0;
        complete = false;
    }
  } 
}

#ifdef WIFI

void setup_udp_broadcast_receiver()
{
    udp.onPacket([](AsyncUDPPacket packet) 
    {
      //if (packet.isBroadcast())
      {
        int ii=0;
        //Well... The packet made it here.. lets try to parse it
        uint8_t *pdata = packet.data();

        if (packet.length()< RECEIVE_DATA_LENGTH-1)
        {
            for (ii=0;ii<packet.length();ii++)
                receive_data[ii] = (char)pdata[ii];

            //Only store adress and use it if its a packet for us
            if (true == process_incoming_data(COMM_WIFI))
                comm_host_ip = packet.remoteIP();
        }
      }
        
    /*
      //Debug only
      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", To: ");
      Serial.print(packet.localIP());
      Serial.print(":");
      Serial.print(packet.localPort());
      Serial.print(", Length: ");
      Serial.print(packet.length());
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();
    */
    
    });

    #ifdef DEBUG
    Serial.println("UDP Receiver started");
    #endif
}

#endif

void init_protocol(void)
{
#ifdef WIFI
    //This should be the Broadcast IP address
    if(udp.listen(UDP_BROADCAST_PORT)) 
    {
      setup_udp_broadcast_receiver();
    }
#endif    
}