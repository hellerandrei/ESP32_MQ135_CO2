 #include "data.h"

DHT_Unified     dht(DHTPIN, DHTTYPE);
uint32_t        delayMS;
TaskHandle_t    THandle_DHT;

void dht_loop(void * parameter);

 void DHT_Init()
{ 
    dht.begin(); 
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
    // Set delay between sensor readings based on sensor details.
    delayMS = sensor.min_delay / 1000;    
}

bool DHT_TaskStart()
{       
    portBASE_TYPE TaskResult; 
    TaskResult = xTaskCreatePinnedToCore(                /* Core where the task should run */  
            dht_loop,                                    /* Function to implement the task */
            "DHTTask",                                   /* Name of the task */
            4096,                                        /* Stack size in words */
            NULL,                                        /* Task input parameter */
            1,                                           /* Priority of the task */
            &THandle_DHT,                                /* Task handle. */
            1
    );

    if ( TaskResult == pdTRUE )
    {
        
        return true; 
    } 
    else return false;  
}


void dht_loop(void * parameter)
{  
    char TempDhpMsg[500];
    sprintf(TempDhpMsg, "%s : %d", "DHT: dht_loop: Starting on Core ", xPortGetCoreID());  
    Add_Log(TempDhpMsg);
    
    DHT_Init();
    
    for(;;)
    { 
        // Delay between measurements.
        delay(5000);
        // Get temperature event and print its value.
        sensors_event_t event;
        dht.temperature().getEvent(&event);
        if (isnan(event.temperature)) {
            Serial.println(F("Error reading temperature!"));
        }
        else {
            // Serial.print(F("Temperature: "));
            // Serial.print(event.temperature);
            // Serial.println(F("°C"));
            temperature = event.temperature;
        }
        // Get humidity event and print its value.
        dht.humidity().getEvent(&event);
        if (isnan(event.relative_humidity)) {
            Serial.println(F("Error reading humidity!"));
        }
        else {
            // Serial.print(F("Humidity: "));
            // Serial.print(event.relative_humidity);
            // Serial.println(F("%"));
            humidity = event.relative_humidity;
        }    
    }
    
    
}