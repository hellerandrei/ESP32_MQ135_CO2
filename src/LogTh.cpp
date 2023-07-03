#include <arduino.h>
#include <freertos/queue.h>
#include <udp.h>
#include "data.h"

// Функции  работы с очередью сообщений Log

#define QUEUE_MAX 20            // Максимальное количество элементов очереди

int spos = 0;                   // Позиция qstore - вставки в очередь 
int rpos = 0;                   // Позиция qstore - вставки в очередь 
char *Q_List[QUEUE_MAX];             // Массив указателей локальной очереди

int iter = 0;

xQueueHandle xPrintQueue; 


void QLogStore(char *q);
char * QLogRetrive(void);
void QLogDelete(void);
void QLogReview(void);

static void prvStdioGatekeeperTask( void *pvParameters );

// Добавление в очередь
void QLogStore(char *q)
{
    if( spos + 1 == rpos || (spos+1 == QUEUE_MAX  && !rpos ) )
    {
        Serial.println("QUEUE: Error : Queue overflow");
        QLogReview();
        return;
    }

    Q_List[spos] = q;
    xQueueSend( xPrintQueue, &q, portMAX_DELAY );
    spos++;

    if ( spos == QUEUE_MAX )     // Установить на начало
    {
        spos = 0;
    }
}

// Убрать из очереди
char * QLogRetrive(void)
{
    if( rpos == QUEUE_MAX )     // Установить на начало
    {
        rpos = 0;
    }
    if( rpos == spos )
    {
        //Serial.println("LOG_QUEUE: Error : Queue is empty");
        return (char*)'\0';
    }

    rpos++;
    return Q_List[rpos-1];
}

void QLogReview(void)
{
    register int t;

    Serial.println("");
    Serial.printf("----------------- spos = %d; rpos = %d -------------------------\n", spos, rpos);
    for( t=rpos; t < spos; t++ )
    {
        
        Serial.printf("%d. %s\n", t+1, Q_List[t]);
        
    }
    Serial.println("------------------------- Queue ------------------------------\n");
    Serial.println("");
}

void QLogDelete(void)
{
    char *p;
    if ( (p = QLogRetrive()) == NULL ) 
    {
        return;
    }
    //Serial.printf("%d. %s - Забрали из очереди \n", rpos, p );
}        




bool InitQueueLog()
{
  xPrintQueue = xQueueCreate( QUEUE_MAX, sizeof( char * ) ); 

  if (xPrintQueue == NULL) 
  {
    Serial.println("LOG_QUEUE: Error : Queue can not be created");
    return false;
  }
  else
  {      
    xTaskCreate( prvStdioGatekeeperTask, "Gatekeeper", 2048, NULL, 0, NULL ); 
    return true;     
  }     
}


static void prvStdioGatekeeperTask( void *pvParameters )
{
  char * pReciveMesg;
  Serial.println("LOG_QUEUE: GATEKEEPER : prvStdioGatekeeperTask : Created.");
    
  for( ;; )
  {
    
    if (xQueueReceive( xPrintQueue, &pReciveMesg, portMAX_DELAY ) == pdPASS)
    {       
      Serial.println(pReciveMesg);
      QLogDelete();
    }        
  }
}


// Добавление элемента - указатель на строку в очередь локальную и передача в GATEKEEPER
void Add_Log(char *text)
{   
    char *p1;          
    p1 = (char *)malloc(strlen(text)+1);
    strcpy(p1, text);
    if(*text) QLogStore(p1); 
    
    if ( MyUdp1.Connected )
    {      
      
              
      char tempText[1000] = "";
      sprintf( tempText,  text ); 
      Add_Udp(tempText);
    }
}