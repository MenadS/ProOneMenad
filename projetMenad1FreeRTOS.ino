#include <TroykaDHT.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h> 

#define DHT_PIN 4  // Pin for DHT sensor
#define DHT_TYPE DHT11
#define MOTOR_PIN 2  // Pin for motor control
#define TRIG_PIN 12 
#define ECHO_PIN 11 // defines variables 

long duration; 
int distance; 


DHT dht(DHT_PIN, DHT_TYPE);

TaskHandle_t readDHTTask_Handle, controlMotorTask_Handle, HC_SR04Task_Handle;

SemaphoreHandle_t xHC_SR04Semaphore, xDHT11Semaphore;

void HC_SR04(void *pvParameters){
  (void) pvParameters;

  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an Output 
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input 

  if (xSemaphoreTake(xHC_SR04Semaphore, portMAX_DELAY) == pdTRUE) {
    for(;;)
    {
      digitalWrite(TRIG_PIN, LOW); 
      delayMicroseconds(2); // Sets the trigPin on HIGH state for 10 micro seconds 
      digitalWrite(TRIG_PIN, HIGH); 
      delayMicroseconds(10); 
      digitalWrite(TRIG_PIN, LOW); // Reads the echoPin, returns the sound wave travel time in microseconds 
      duration = pulseIn(ECHO_PIN, HIGH); // Calculating the distance 
      distance = duration * 0.034 / 2; // Prints the distance on the Serial Monitor 
      Serial.print("Distance: "); 
      Serial.println(distance);
  
      // Wait for 3 seconds
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      xSemaphoreGive(xDHT11Semaphore);
    }
  } 
}

void readDHTTask(void *pvParameters) {
  (void) pvParameters;
  
  if (xSemaphoreTake(xDHT11Semaphore, portMAX_DELAY) == pdTRUE) {
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
          Serial.print("Temperature = ");
          Serial.print(dht.getTemperatureK());
          Serial.print(" K \t");
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
      
      // Wait for 3 seconds
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      xSemaphoreGive(xHC_SR04Semaphore);
    }
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

  xHC_SR04Semaphore = xSemaphoreCreateBinary();
  xDHT11Semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xHC_SR04Semaphore);
  
  // Create task to read temperature and humidity
  xTaskCreate(readDHTTask, "ReadDHTTask", 128, NULL, 1, &readDHTTask_Handle);
  
  // Create task to control the motor
  xTaskCreate(controlMotorTask, "ControlMotorTask", 128, NULL, 2, &controlMotorTask_Handle);

  // Create task to read distance
  xTaskCreate(HC_SR04, "HC_SR04Task", 128, NULL, 1, &HC_SR04Task_Handle);

  vTaskStartScheduler();
}

void loop()
{
  // The loop() function is empty because we are using FreeRTOS to manage tasks.
}
