## **Practica 4: SISTEMAS OPERATIVOS EN TIEMPO REAL**
En esta práctica va enfocada en los sistemas operativos en tiempo real, especialmente 
la ejecución de tareas, dónde se dividirán entre ellas el tiempo de uso para realizarlas.

## **Ejercicio Practico 1**
**Codigo main.cpp 1r parte:**
```
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


```
El funcionamiento de el código proporcionado, se basa en que crea 2 tareas donde se utliza un un sistema de operativo de tiempo real FreeRTOS.

La tarea principal: se ejecuta en la función loop(), imprime repetidamente un mensaje ("this is ESP32 Task") en el puerto serie y espera 1 segundo entre cada impresión.
La segunda tarea: creada en la función setup() y llamada anotherTask, también imprime repetidamente un mensaje ("this is another Task") en el puerto serie y espera 1 segundo entre cada impresión.
Las salidas que se muestran ppor el puerto serie son las siguientes:
```
this is another Task
this is ESP32 Task
```


## **Ejercicio Practico 2**
**Codigo main.cpp:**
```
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


```
En el anterior código tenemos un programa donde con la ayuda de un semáforo, se pueden 
utilizar dos tareas, (una que enciende el led ) y otra tarea ( que apaga el Led), en 
el programa se puede ver que el tiempo del DELAY es de 1 segundo, lo cual cada 1 segundo se van alternando.

Funciones / Subprogramas utilizados:
En este codigo tenemos 4 funciones para llevarlo a cabo.

Función setup() :
se inicia la comunicación serial y se configura el pin del LED como salida.

Función loop():
no es necesario realizar ninguna acción en el bucle principal.

Funciones : encenderLED() y apagarLED():
son las tareas que se ejecutarán concurrentemente. Ambas funciones son ciclos infinitos (for (;;)) 
que alternan entre encender y apagar el LED con un intervalo de un segundo.

Salida puerto serie:
Alternando las 2 tareas, se imprime "LED HIGH" cuando el LED se enciende, y en la función apagarLED(), 
se imprime "LED LOW" cuando el LED se apaga.

```
LED ENCENDIDO
LED APAGADO
LED ENCENDIDO
```

## **Ejercicios de mejora de nota**































