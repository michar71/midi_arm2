#ifndef baboi_protocol_h
#define baboi_protocol_h

//Data Sentences
#define ID_DATA 'D'   //Mition Data
#define ID_QUERY 'Q'  //Info Query
#define ID_INFO 'I'   //Answer to Query
#define ID_SETUP 'S'  //Setup Parameters
#define ID_PING 'P'   //Timeout Ping from Host 

typedef enum{
    COMM_SERIAL,
    COMM_WIFI
}t_comm_channel;

#define SEND_DATA_LENGTH 255
#define RECEIVE_DATA_LENGTH 255
#define UDP_BROADCAST_PORT 2255

void build_processing_data(bool senddata);
void send_processing_data(bool senddata);
void build_info_data(void);
void send_info_data(void);
bool process_incoming_data(t_comm_channel commChannel);
void incoming_protocol_request(void);
bool checkCommTimeout(void);
void init_protocol(void);

#endif