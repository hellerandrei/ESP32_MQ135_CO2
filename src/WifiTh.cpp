#include <WiFi.h>
#include <IPAddress.h>
#include <ArduinoOTA.h>
#include "data.h"



TaskHandle_t    THandle_WIFI;   

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
void WIFI_loop( void * parameter );

bool Wifi_connected        = false;
const char * networkName   = NETWORK_NAME;     // Сеть
const char * networkPswd   = NETWORK_PASS;     // Пароль


bool init_WIFI()
{  
  Add_Log((char*)"\n------------------- WIFI -----------------");
  xTaskCreatePinnedToCore( WIFI_loop, "WIFI_loop ", 8192, NULL, 2, &THandle_WIFI, 1);

  if ( THandle_WIFI == NULL )
  {
    Add_Log((char*)"WIFI: ERROR : Failed to start WIFI_loop");
    return false;
  }
  else
  return true;
}


void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  //Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) 
  {
      case SYSTEM_EVENT_WIFI_READY:         
      Add_Log((char*)  "\n  WIFI: EVENT : ESP32 WiFi ready ");
      break;
      case SYSTEM_EVENT_SCAN_DONE:              
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 finish scanning AP ");
      break;
      case SYSTEM_EVENT_STA_START:              
      Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station start ");      
      break;
      case SYSTEM_EVENT_STA_STOP:               
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station stop ");
      break;
      case SYSTEM_EVENT_STA_CONNECTED:          
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station connected to AP ");
      break;
      case SYSTEM_EVENT_STA_DISCONNECTED:       
      {
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station disconnected from AP ");
         // char fullMac[30] = ""; 
         // strcpy(fullMac, " WIFI: Mac=");
         // for(int i = 0; i< 6; i++)
         // {     
         //    char mac[3] = "";
         //    sprintf( mac, "%02X", info.sta_connected.mac[i] );  
         //    strcat( fullMac, mac );
         //    if( i<5 )
         //    {
         //       strcat( fullMac, ":");
         //    }
         // }      
         // Add_Log(fullMac);     
         Wifi_connected = false;
      }   
      break;
      case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:    
         Add_Log((char*)  "\n  WIFI: EVENT : the auth mode of AP connected by ESP32 station changed ");
      break;
      case SYSTEM_EVENT_STA_GOT_IP:             
      {
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station got IP from connected AP ");
         // char fullMac[30] = ""; 
         // strcpy(fullMac, " WIFI: Mac=");
         // for(int i = 0; i< 6; i++)
         // {     
         //    char mac[3] = "";
         //    sprintf( mac, "%02X", info.sta_connected.mac[i] );  
         //    strcat( fullMac, mac );
         //    if( i<5 )
         //    {
         //       strcat( fullMac, ":");
         //    }
         // }      
         // Add_Log(fullMac);         
      }
      break;
      case SYSTEM_EVENT_STA_LOST_IP:            
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station lost IP and the IP is reset to 0 ");
      break;
      case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:     
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station wps succeeds in enrollee mode ");
      break;
      case SYSTEM_EVENT_STA_WPS_ER_FAILED:      
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station wps fails in enrollee mode ");
      break;
      case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:     
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station wps timeout in enrollee mode ");
      break;
      case SYSTEM_EVENT_STA_WPS_ER_PIN:         
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station wps pin code in enrollee mode ");
      break;
      case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP: 
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station wps overlap in enrollee mode ");
      break;
      case SYSTEM_EVENT_AP_START:               
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 soft-AP start ");
      break;
      case SYSTEM_EVENT_AP_STOP:                
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 soft-AP stop ");
      break;
      case SYSTEM_EVENT_AP_STACONNECTED:        
         Add_Log((char*)  "\n  WIFI: EVENT : a station connected to ESP32 soft-AP ");
      break;
      case SYSTEM_EVENT_AP_STADISCONNECTED:     
         Add_Log((char*)  "\n  WIFI: EVENT : a station disconnected from ESP32 soft-AP ");
      break;
      case SYSTEM_EVENT_AP_STAIPASSIGNED:       
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 soft-AP assign an IP to a connected station ");
      break;
      case SYSTEM_EVENT_AP_PROBEREQRECVED:      
         Add_Log((char*)  "\n  WIFI: EVENT : Receive probe request packet in soft-AP interface ");
      break;
      case SYSTEM_EVENT_GOT_IP6:                
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 station or ap or ethernet interface v6IP addr is preferred ");
      break;
      case SYSTEM_EVENT_ETH_START:              
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 ethernet start ");
      break;
      case SYSTEM_EVENT_ETH_STOP:               
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 ethernet stop ");
      break;
      case SYSTEM_EVENT_ETH_CONNECTED:          
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 ethernet phy link up ");
      break;
      case SYSTEM_EVENT_ETH_DISCONNECTED:       
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 ethernet phy link down ");
         Wifi_connected = false;
      break;
      case SYSTEM_EVENT_ETH_GOT_IP:             
         Add_Log((char*)  "\n  WIFI: EVENT : ESP32 ethernet got IP from connected AP ");
      break;    
      case SYSTEM_EVENT_MAX:             
         Add_Log((char*)  "\n  WIFI: EVENT : SYSTEM_EVENT_MAX");
      break;        
  }
}


void WIFI_loop( void * parameter )
{   
  char tempText1[50] = "";
  sprintf( tempText1, "WIFI: WIFI_loop : Wifi running on Core: %d", xPortGetCoreID());
  Add_Log(tempText1);
    
  Wifi_connected = false; 
  IPAddress  G_MyIP;                      // Присваеваем IP адрес нашему ESP32.
  IPAddress  G_Gateway;                   // Адрес WiFi маршрутизатора 
  IPAddress  G_Subnet;                    // По умолчанию (см настройки модема).

  G_MyIP.fromString( LOCAL_IP );
  G_Gateway.fromString( GATEWAY );
  G_Subnet.fromString( SUBNET );

  WiFi.onEvent(WiFiEvent);
  WiFi.config( G_MyIP, G_Gateway, G_Subnet );
  WiFi.begin( networkName, networkPswd );

  for (;;)
  {
    ArduinoOTA.handle(); 


    if ( WiFi.status() ==  WL_CONNECTED )

    {
      if ( Wifi_connected == false )
      {       
        ArduinoOTA.begin();
        Add_Log((char*)"Ready for buisness!");

        ArduinoOTA.onStart([]()
        {
          char type[25];
          if (ArduinoOTA.getCommand() == U_FLASH)
          {
            strcpy(type, "sketch");

          } else
          { // U_FS
            strcpy(type, "filesystem");
          }

          // NOTE: if updating FS this would be the place to unmount FS using FS.end()
          char TempText[256];
          memset(TempText, 0, strlen(TempText));
          strcpy(TempText, " WIFI: WIFI_loop : Start updating: ");
          strcat(TempText, type);
          Add_Log(TempText);
          if ( !G_Upgrading ) G_Upgrading = true; 
        });

        ArduinoOTA.onEnd([]()
        {
           Add_Log((char*)"\nEnd");
        });

        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
        {          
          Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });

        ArduinoOTA.onError([](ota_error_t error)
        {
          MyUdp1.WakeUp();
          Serial.printf("Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR) {
             Add_Log((char*)"Auth Failed");
          } else if (error == OTA_BEGIN_ERROR) {
             Add_Log((char*)"Begin Failed");
          } else if (error == OTA_CONNECT_ERROR) {
             Add_Log((char*)"Connect Failed");
          } else if (error == OTA_RECEIVE_ERROR) {
             Add_Log((char*)"Receive Failed");
          } else if (error == OTA_END_ERROR) {
             Add_Log((char*)"End Failed");
          }
        });
        Wifi_connected = true;
      }
     //Serial.print("cconnected = true;"); 
    }
    else // wifi down, reconnect here
    {
      Wifi_connected = false;
      
      char TempText[150] = "";
      sprintf( TempText, " WIFI: WIFI_loop : Connecting to WiFi network: %s", networkName );      
      Add_Log(TempText);

      int UpCount = 0;
      while ( WiFi.status() != WL_CONNECTED )
      {
        Wifi_connected = false;
        //Serial.print("WiFi  status: ");
        // Add_Log((char*)WiFi.status());

        vTaskDelay( 1000 / portTICK_RATE_MS );

        Serial.printf(".");
        if (UpCount >= 100)  // just keep terminal from scrolling sideways
        {
            UpCount = 0;
            Serial.printf("\n");

            Esp_reset();
        }
        ++UpCount;
      }
      Serial.println("\n");
      
    }
    vTaskDelay( 5000 / portTICK_RATE_MS );
  }
}