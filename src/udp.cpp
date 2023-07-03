#include <udp.h>
#include "data.h"

/** WiFiUDP class for creating UDP communication */

WiFiUDP  UdpRead;

char TempUdpMsg[256];
char G_ReadData[3000]; 
char udpHeader[50]      = "";
char udpResponse[5500]  = "";

extern bool Wifi_connected;

long    read_UDP_timer = 0,
        send_UDP_timer = 0;

TMyUdp::TMyUdp(int Core, int Priority)
{
    _core     = Core;
    _priority = Priority;
    Connected = false;    
}


void TMyUdp::WakeUp()
{
  if ( THandle_UDP != NULL ) 
  {
    G_Upgrading = false;
    xTaskResumeFromISR(THandle_UDP);
  }
}

void TMyUdp::Suspend()
{
  if ( THandle_UDP != NULL ) 
  {
    Add_Log( (char*)"UDP: udp_loop: Suspending " );
    vTaskSuspend(THandle_UDP);
  } 
}


void TMyUdp::SendBroadcast( char * Msg )
{
  strcpy(TMyUdp::BroadcastData, Msg);
  TMyUdp::BroadcastExist = true;
}


bool TMyUdp::TaskStart()
{       
    portBASE_TYPE TaskResult; 
    TaskResult = xTaskCreatePinnedToCore(                /* Core where the task should run */  
            udp_loop,                                    /* Function to implement the task */
            "UdpTask",                                   /* Name of the task */
            20480,                                        /* Stack size in words */
            NULL,                                        /* Task input parameter */
            _priority,                                   /* Priority of the task */
            &THandle_UDP,                                /* Task handle. */
            _core
    );

    if ( TaskResult == pdTRUE )
    {
        
        return true; 
    } 
    else return false;  
}


void udp_loop(void * parameter)
{  
  
  sprintf(TempUdpMsg, "%s : %d", "UDP: udp_loop: Starting on Core ", xPortGetCoreID());  
  Add_Log(TempUdpMsg);
  
  UdpRead.begin(UDP_PORT); 
  InitQueueUdp();

  MyUdp1.Connected = true;
  for(;;)
  {     
    //Serial.print("-");
    if ( G_Upgrading )
    {
      Add_Log((char*)"UDP: udp_loop: Suspended, Uploading new firemware...");
      MyUdp1.Suspend();
    }

    if ( Wifi_connected )
    {     
      // Serial.print("*");
      int packetSize = UdpRead.parsePacket();
      if (packetSize) 
      {
        // Serial.printf("Received - size %d from %s:%d\n", packetSize, UdpRead.remoteIP().toString().c_str(), UdpRead.remotePort());

        // read the packet into packetBufffer
        int n = UdpRead.read(G_ReadData, sizeof(G_ReadData));
        G_ReadData[n] = 0;
        
        char s1[ n + 1 ];
        strcpy(s1, "");
        strcpy(s1, G_ReadData);
        
        int findDelim = strchr(s1, ';') - s1;
        if ( findDelim > 0 )
        {
          strncpy(G_ReadData, s1, findDelim + 1);

          char * UdpResponse = Analiz_UDP_Data( G_ReadData );
          memset(G_ReadData, 0, strlen(G_ReadData));
          if ( strlen(UdpResponse) > 0 ) 
          {   
            uint8_t buffer[strlen( UdpResponse) ];
            memcpy(buffer, UdpResponse, strlen( UdpResponse));
            UdpRead.beginPacket( UdpRead.remoteIP().toString().c_str(), UdpRead.remotePort() );
            UdpRead.write(buffer, sizeof(buffer));    
            if  ( UdpRead.endPacket() != 1 ) 
            {
              Add_Log((char*)"UDP: UdpRead.endPacket() - Error ");
            }                            
          }                                 
        } 
      } 

      // if ( MyUdp1.BroadcastExist )
      // {
      //   MyUdp1.BroadcastExist = false;
        
      //   char tempMsg[5000] = "";
      //   strcpy( tempMsg, GetUdpPacketHeader() );
      //   strcat( tempMsg, GetSuccessResult("log", MyUdp1.BroadcastData) );
      //   strcat( tempMsg, "}" );
      //   uint8_t buffer[strlen(tempMsg) ];
      //   memcpy(buffer, tempMsg, strlen(tempMsg) );

      //   UdpRead.beginPacket( UdpRead.remoteIP().toString().c_str(), UdpRead.remotePort() );
      //   UdpRead.write(buffer, sizeof(buffer));    
      //   UdpRead.endPacket();                   
      // }

    } 
    vTaskDelay( 250 / portTICK_RATE_MS ); 
  }  
}




char* GetUdpPacketHeader()
{  
  sprintf(udpHeader, "{\"%s\":\"%s\",\"%s\":\"%s\",", "ip", LOCAL_IP,"module", MODULE_NAME);
  
  return udpHeader; 
}


char Success_result[600];
char* GetSuccessResult(char *action,  char *value)
{    
  sprintf(Success_result, "\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"", "action", action, "value", value, "status", "success");
  return Success_result;  
}


char Ping_result[3000];
char* GetPing()
{    
  String text = "123123123";
  strcpy(Ping_result, &text[0]);

  return Ping_result;  
}

char * Analiz_UDP_Data( char * UdpMessage )
{
  memset(udpResponse, 0, strlen(udpResponse));
  if ( strlen(UdpMessage) > 0 )
  {  
    strcpy(udpResponse, GetUdpPacketHeader()); 
    
    return udpResponse;         
  }


}
int UDP_Q_spos = 0;                       // Позиция qstore - вставки в очередь 
int UDP_Q_rpos = 0;                       // Позиция qstore - вставки в очередь 
char *UDP_Q_List[UDP_QUEUE_MAX];        // Массив указателей локальной очереди

xQueueHandle xUDPQueue; 


// Добавление в очередь
void QUdpStore(char *q)
{
    if( UDP_Q_spos + 1 == UDP_Q_rpos || (UDP_Q_spos+1 == UDP_QUEUE_MAX  && !UDP_Q_rpos ) )
    {
        Serial.println("QUEUE: Error : Queue overflow");
        QUdpReview();
        return;
    }

    UDP_Q_List[UDP_Q_spos] = q;
    xQueueSend( xUDPQueue, &q, portMAX_DELAY );
    UDP_Q_spos++;

    if ( UDP_Q_spos == UDP_QUEUE_MAX )     // Установить на начало
    {
        UDP_Q_spos = 0;
    }
}

// Убрать из очереди
char * QUdpRetrive(void)
{
    if( UDP_Q_rpos == UDP_QUEUE_MAX )     // Установить на начало
    {
        UDP_Q_rpos = 0;
    }
    if( UDP_Q_rpos == UDP_Q_spos )
    {
        //Serial.println("LOG_QUEUE: Error : Queue is empty");
        return (char*)'\0';
    }

    UDP_Q_rpos++;
    return UDP_Q_List[UDP_Q_rpos-1];
}

void QUdpReview(void)
{
    register int t;

    Serial.println("");
    Serial.printf("----------------- UDP_Q_spos = %d; UDP_Q_rpos = %d -------------------------\n", UDP_Q_spos, UDP_Q_rpos);
    for( t=UDP_Q_rpos; t < UDP_Q_spos; t++ )
    {
        
        Serial.printf("%d. %s\n", t+1, UDP_Q_List[t]);
        
    }
    Serial.println("------------------------- Queue ------------------------------\n");
    Serial.println("");
}

void QUdpDelete(void)
{
    char *p;
    if ( (p = QUdpRetrive()) == NULL ) 
    {
        return;
    }
    //Serial.printf("%d. %s - Забрали из очереди \n", UDP_Q_rpos, p );
}        




bool InitQueueUdp()
{
  xUDPQueue = xQueueCreate( UDP_QUEUE_MAX, sizeof( char * ) ); 

  if (xUDPQueue == NULL) 
  {
    Serial.println("UDP_QUEUE: Error : Queue can not be created");
    return false;
  }
  else
  {      
    xTaskCreate( UDPGatekeeperTask, "Gatekeeper", 4096, NULL, 0, NULL ); 
    return true;     
  }     
}


void UDPGatekeeperTask( void *pvParameters )
{
  char * pReciveMesg;
  Add_Log((char*)"\nUDP_QUEUE: UDPGATEKEEPER : UDPGatekeeperTask : Created.");

  char tempUdpMsg[1000] = "";  

  for( ;; )
  {
    
    if (xQueueReceive( xUDPQueue, &pReciveMesg, portMAX_DELAY ) == pdPASS)
    {       
      strcpy( tempUdpMsg, GetUdpPacketHeader() );
      strcat( tempUdpMsg, GetSuccessResult((char*)"log", pReciveMesg) );
      strcat( tempUdpMsg, "}" );

      uint8_t buffer[strlen(tempUdpMsg) ];
      memcpy(buffer, tempUdpMsg, strlen(tempUdpMsg) );

      UdpRead.beginPacket( BROADCAST_IP, UDP_PORT );
      UdpRead.write(buffer, sizeof(buffer));    
      UdpRead.endPacket();  

      QUdpDelete();
    }        
  }
}   
  
void Add_Udp(char *text)
{ 
  char *p1;          
  p1 = (char *)malloc(strlen(text)+1);
  strcpy(p1, text);
  if(*text) QUdpStore(p1); 
}




   