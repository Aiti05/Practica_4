//Ejercicio 1


/*
#include <Arduino.h>

void anotherTask(void * parameter) {
  for(;;) { 
    Serial.println("this is another Task");
    delay(1000);
  }
  vTaskDelete(NULL);  // Esta línea nunca se ejecutará, pero es buena práctica incluirla
}

void setup() {
  Serial.begin(115200);

  xTaskCreate(
    anotherTask,  // Función de la tarea
    "another Task",  // Nombre de la tarea
    10000,  // Tamaño de la pila en bytes
    NULL,  // Parámetro de la tarea
    1,  // Prioridad de la tarea
    NULL  // Manejador de la tarea
  );
}

void loop() {
  Serial.println("this is ESP32 Task");
  delay(1000);
}






// Ejercicio 2




#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

const int ledPin = 2;  // Pin del LED
SemaphoreHandle_t xSemaphore;  // Semáforo para sincronizar las tareas

// Tarea para encender el LED
void TaskTurnOn(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY)) { // Toma el semáforo
      digitalWrite(ledPin, HIGH);
      Serial.println("LED ENCENDIDO");
      delay(1000);  // Espera un segundo antes de liberar el semáforo
      xSemaphoreGive(xSemaphore); // Libera el semáforo
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Pequeña espera antes de intentar tomar el semáforo nuevamente
  }
}

// Tarea para apagar el LED
void TaskTurnOff(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY)) { // Toma el semáforo
      digitalWrite(ledPin, LOW);
      Serial.println("LED APAGADO");
      delay(1000);  // Espera un segundo antes de liberar el semáforo
      xSemaphoreGive(xSemaphore); // Libera el semáforo
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  xSemaphore = xSemaphoreCreateBinary();  // Crear el semáforo

  xTaskCreate(TaskTurnOn, "Encender LED", 1000, NULL, 1, NULL);
  xTaskCreate(TaskTurnOff, "Apagar LED", 1000, NULL, 1, NULL);

  xSemaphoreGive(xSemaphore); // Inicializa el semáforo en "libre"
}

void loop() {
  // No se necesita código en loop(), ya que todo ocurre en las tareas de FreeRTOS
}



*/

