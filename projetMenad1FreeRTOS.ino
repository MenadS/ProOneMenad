#include <TroykaDHT.h>
#include <Arduino_FreeRTOS.h>

#define DHT_PIN 4  // Pin for DHT sensor
#define DHT_TYPE DHT11
#define MOTOR_PIN 2  // Pin for motor control

DHT dht(DHT_PIN, DHT_TYPE);

TaskHandle_t readDHTTask_Handle, controlMotorTask_Handle;

void readDHTTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
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
        Serial.print("Humidity = ");
        Serial.print(dht.getHumidity());
        Serial.println(" %");
        // If temperature exceeds 28°C, notify the motor control task
        if (dht.getTemperatureC() > 28) {
          xTaskNotify(controlMotorTask_Handle, 1, eSetValueWithOverwrite);
        }
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
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void controlMotorTask(void *pvParameters) {
  (void) pvParameters;

  pinMode(MOTOR_PIN, OUTPUT);
  
  for (;;) {
    // Wait for notification from readDHTTask
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    // Control the motor
    digitalWrite(MOTOR_PIN, HIGH);
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Run motor for 5 seconds
    digitalWrite(MOTOR_PIN, LOW);
  }
}

void setup()
{
  // Open the serial port for monitoring program actions
  Serial.begin(9600);
  dht.begin();

  // Create task to read temperature and humidity
  xTaskCreate(readDHTTask, "ReadDHTTask", 128, NULL, 1, &readDHTTask_Handle);
  
  // Create task to control the motor
  xTaskCreate(controlMotorTask, "ControlMotorTask", 128, NULL, 2, &controlMotorTask_Handle);

  vTaskStartScheduler();
}

void loop()
{
  // The loop() function is empty because we are using FreeRTOS to manage tasks.
}

/*// Library for working with DHT series sensors
#include <TroykaDHT.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h> 

// Create an object of the DHT class
// Pass the pin number to which the sensor is connected and the sensor type
// Sensor types: DHT11, DHT21, DHT22
DHT dht(4, DHT11);
#define MOTOR_PIN 2  // Pin for motor control

TaskHandle_t readDHTTask_Handle, controlMotorTask_Handle;

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
      // If temperature exceeds 20°C, notify the motor control task
      if (dht.getTemperatureC() > 20) {
        xTaskNotify(controlMotorTask_Handle, 1, eSetValueWithOverwrite);
      }
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
void controlMotorTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    // Wait for notification from readDHTTask
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    // Control the motor
    digitalWrite(MOTOR_PIN, HIGH);
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Run motor for 5 seconds
    digitalWrite(MOTOR_PIN, LOW);
  }
}
void setup()
{
  // Open the serial port for monitoring program actions
  Serial.begin(9600);
  dht.begin();

  // Création de la tâche pour lire la température et l'humidité
  //xTaskCreate(readDHTTask, "ReadDHTTask", 128, NULL, 1, NULL);
  xTaskCreate(readDHTTask, "ReadDHTTask", 128, NULL, 1, &readDHTTask_Handle);

  // Create task to control the motor
  xTaskCreate(controlMotorTask, "ControlMotorTask", 128, NULL, 2, &controlMotorTask_Handle);
  
  vTaskStartScheduler();
}

void loop()
{

}*/
