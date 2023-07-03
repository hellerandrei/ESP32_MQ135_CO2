#ifndef MYUDP_h
#define MYUDP_h

#include <WiFiUdp.h>

#define udpCore         0
#define udpPriority     2
#define UDP_QUEUE_MAX   20            // Максимальное количество элементов очереди отправки по UDP

void UDPGatekeeperTask( void *pvParameters );

// Функции  работы с очередью сообщений UDP

bool InitQueueUdp();
void QUdpStore(char *q);
char * QUdpRetrive(void);
void QUdpDelete(void);
void QUdpReview(void);

class TMyUdp
{
private:
    int _priority     = 1;
    int _core         = 0;     
public:
    TMyUdp(int Core  = 1, int Priority = 1);

    TaskHandle_t THandle_UDP;
        
    bool        Connected             = false;
    boolean     BroadcastExist        = false;    // Есть данные для отправки в широковещательном пакете
    char        BroadcastData[5000]   = "";       // Данные для отправки широковещательного пакета   
     
    bool TaskStart();    
    void DHTPowerReset();
    void WakeUp();
    void Suspend(); 
    void SendBroadcast( char * Msg );
};


bool udp_SendMessage(IPAddress ipAddr, char * udpMsg, int udpPort);
void udp_loop(void * parameter);
char * Analiz_UDP_Data( char * UdpMessage );
extern TMyUdp MyUdp1;

char* GetUdpPacketHeader();
char* GetSuccessResult(char *action,  char *value);

#endif