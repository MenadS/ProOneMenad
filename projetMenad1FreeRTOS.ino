// Library for working with DHT series sensors
#include <TroykaDHT.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h> 

// Create an object of the DHT class
// Pass the pin number to which the sensor is connected and the sensor type
// Sensor types: DHT11, DHT21, DHT22
DHT dht(4, DHT11);

TaskHandle_t readDHTTask_Handle;

void readDHTTask(void *pvParameters) {
  (void) pvParameters;

  // Reading data from the sensor
  dht.read();
  // Check data state
  switch(dht.getState()) {
    // All is OK
    case DHT_OK:
      // Display humidity and temperature readings
      Serial.print("Temperature = ");
      Serial.print(dht.getTemperatureC());
      Serial.print(" C \t");
      Serial.print("Temperature = ");
      Serial.print(dht.getTemperatureK());
      Serial.print(" K \t");
      Serial.print("Humidity = ");
      Serial.print(dht.getHumidity());
      Serial.println(" %");
      break;
    // Checksum error
    case DHT_ERROR_CHECKSUM:
      Serial.println("Checksum error");
      break;
    // Timeout error
    case DHT_ERROR_TIMEOUT:
      Serial.println("Time out error");
      break;
    // No data, sensor not responding or absent
    case DHT_ERROR_NO_REPLY:
      Serial.println("Sensor not connected");
      break;
  }
  
  // Wait for two seconds
  delay(2000);
  
}
void setup()
{
  // Open the serial port for monitoring program actions
  Serial.begin(9600);
  dht.begin();

  // Création de la tâche pour lire la température et l'humidité
  //xTaskCreate(readDHTTask, "ReadDHTTask", 128, NULL, 1, NULL);
  xTaskCreate(readDHTTask, "ReadDHTTask", 128, NULL, 1, &readDHTTask_Handle);
  vTaskStartScheduler();
}

void loop()
{

}
