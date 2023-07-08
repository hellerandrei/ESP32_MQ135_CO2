#include <Arduino.h>
#include "MQ135.h"
#include "data.h"
#include <udp.h>


MQ135 mq135_sensor = MQ135(PIN_MQ135);
TMyUdp MyUdp1 ( udpCore, udpPriority);

bool init_WIFI();
bool G_Upgrading = false;

float temperature = 25.0; // assume current temperature. Recommended to measure with DHT22
float humidity = 75.0; // assume current humidity. Recommended to measure with DHT22

void setup() {
  Serial.begin(115200);

 if ( !InitQueueLog() )
  {    
      Serial.println("LOG: InitSerial : Error -> rebooting...");

      delay(10000);
      Esp_reset();
  }

  

  if ( !init_WIFI() )
  {
    Serial.println("WIFI: init_WIFI : ERROR, ESP32 Send reset signal");
    delay(10000);
    Esp_reset();
  }
  delay(5000);
  MyUdp1.TaskStart();

  delay(2000);
  DHT_Init();
  delay(2000);
  DHT_TaskStart();
}

void loop() {
  char txt_ppm[556]; 
  
  float rzero           = mq135_sensor.getRZero();
  float correctedRZero  = mq135_sensor.getCorrectedRZero(temperature, humidity);
  float resistance      = mq135_sensor.getResistance();
  float ppm             = mq135_sensor.getPPM();
  float correctedPPM    = mq135_sensor.getCorrectedPPM(temperature, humidity);

  sprintf(
        txt_ppm, 
        "%s : %d\t%s : %d\t%s : %d\t%s : %d\t%s : %d\t%s : %d\t%s : %d", 
        "MQ135 RZero: ",  (int)rzero,
        "CorRZero: ",     (int)correctedRZero, 
        "R: ",   (int)resistance, 
        "PPM: ",          (int)ppm, 
        "CorPPM: ",       (int)correctedPPM/100, 
        "Temp: ",         (int)temperature, 
        "Hum: ",          (int)humidity
        );  

 
  Add_Log(txt_ppm);
  sprintf(txt_ppm, "ppm:%d,t:%d,h:%d",(int)correctedPPM/100, (int)temperature, (int)humidity);
  Add_Udp(txt_ppm);

  delay(1000);
}

void Esp_reset()
{
  Add_Log((char *)" WARNING : ESP32 Software Reset...");
  delay(500);
  ESP.restart();
}   