## **Practica 4: SISTEMAS OPERATIVOS EN TIEMPO REAL**
Esta práctica se centra en los sistemas operativos en tiempo real, especialmente en la ejecución de tareas, donde se gestiona y distribuye el tiempo de uso entre ellas para su realización.

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
El código proporcionado crea dos tareas utilizando el sistema operativo en tiempo real FreeRTOS.

La tarea principal, ejecutada en la función loop(), imprime repetidamente el mensaje ("this is ESP32 Task") en el puerto serie, con un intervalo de 1 segundo entre cada impresión.
La segunda tarea, creada en la función setup() y llamada anotherTask, también imprime repetidamente el mensaje ("this is another Task") en el puerto serie, con un intervalo de 1 segundo entre cada impresión.

Esto sale por pantalla al ejecutarse:
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
En este programa, se utilizan dos tareas con la ayuda de un semáforo: una para encender el LED y otra para apagarlo. El tiempo de delay es de 1 segundo, lo que provoca que las tareas se alternen cada segundo.
Se utilizan varias funciones como la función setup() que se inicia la comunicación serial y se configura el pin del LED como salida, tambien esta la función loop() qu no realiza ninguna acción en el bucle principal y por ultimo las funciones encenderLED() y apagarLED() que son las tareas que se ejecutan de forma concurrente. Ambas funciones son ciclos infinitos que alternan entre encender y apagar el LED con un intervalo de 1 segundo.
Por pantalla se imprime "LED ENCENDIDO" cuando el LED se activa y "LED APAGADO" cuando se desactiva.

```
LED ENCENDIDO
LED APAGADO
LED ENCENDIDO
```

## **Ejercicios de mejora de nota**

### **Reloj:**
**Codigo main.cpp:**
```
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"


// Definición de pines
#define LED_SEGUNDOS 2  
#define LED_MODO 40    
#define BTN_MODO 48    
#define BTN_INCREMENTO 36


// Variables globales del reloj
volatile int horas = 0, minutos = 0, segundos = 0;
volatile int modo = 0;  


// Recursos FreeRTOS
QueueHandle_t botonQueue;
SemaphoreHandle_t relojMutex;


// Estructura para eventos de botón
typedef struct {
    uint8_t boton;
    uint32_t tiempo;
} EventoBoton;


void IRAM_ATTR ISR_Boton(void *arg) {
    uint8_t numeroPulsador = (uint32_t)arg;
    EventoBoton evento = {numeroPulsador, xTaskGetTickCountFromISR()};
    xQueueSendFromISR(botonQueue, &evento, NULL);
}


void TareaReloj(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);
   
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        if (xSemaphoreTake(relojMutex, portMAX_DELAY)) {
            if (modo == 0) {
                segundos++;
                if (segundos == 60) {
                    segundos = 0;
                    minutos++;
                    if (minutos == 60) {
                        minutos = 0;
                        horas = (horas + 1) % 24;
                    }
                }
            }
            xSemaphoreGive(relojMutex);
        }
    }
}


void TareaLecturaBotones(void *pvParameters) {
    EventoBoton evento;
    for (;;) {
        if (xQueueReceive(botonQueue, &evento, portMAX_DELAY)) {
            if (xSemaphoreTake(relojMutex, portMAX_DELAY)) {
                if (evento.boton == BTN_MODO) {
                    modo = (modo + 1) % 3;
                } else if (evento.boton == BTN_INCREMENTO) {
                    if (modo == 1) horas = (horas + 1) % 24;
                    if (modo == 2) minutos = (minutos + 1) % 60;
                }
                xSemaphoreGive(relojMutex);
            }
        }
    }
}


void TareaActualizacionDisplay(void *pvParameters) {
    int last_h = -1, last_m = -1, last_s = -1, last_modo = -1;
    for (;;) {
        if (xSemaphoreTake(relojMutex, portMAX_DELAY)) {
            if (horas != last_h || minutos != last_m || segundos != last_s || modo != last_modo) {
                Serial.printf("Hora: %02d:%02d:%02d  | Modo: %d\n", horas, minutos, segundos, modo);
                last_h = horas; last_m = minutos; last_s = segundos; last_modo = modo;
            }
            xSemaphoreGive(relojMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void TareaControlLEDs(void *pvParameters) {
    for (;;) {
        digitalWrite(LED_SEGUNDOS, segundos % 2);
        digitalWrite(LED_MODO, modo != 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void setup() {
    Serial.begin(115200);
    pinMode(LED_SEGUNDOS, OUTPUT);
    pinMode(LED_MODO, OUTPUT);
    pinMode(BTN_MODO, INPUT_PULLUP);
    pinMode(BTN_INCREMENTO, INPUT_PULLUP);


    relojMutex = xSemaphoreCreateMutex();
    botonQueue = xQueueCreate(10, sizeof(EventoBoton));


    attachInterrupt(BTN_MODO, ISR_Boton, FALLING);
    attachInterrupt(BTN_INCREMENTO, ISR_Boton, FALLING);


    xTaskCreate(TareaReloj, "Tarea Reloj", 2048, NULL, 2, NULL);
    xTaskCreate(TareaLecturaBotones, "Lectura Botones", 2048, NULL, 2, NULL);
    xTaskCreate(TareaActualizacionDisplay, "Actualización Display", 2048, NULL, 1, NULL);
    xTaskCreate(TareaControlLEDs, "Control LEDs", 2048, NULL, 1, NULL);
}


void loop() {
    vTaskDelay(portMAX_DELAY);
}
```
Este código implementa un reloj digital en un ESP32 con FreeRTOS, permitiendo la visualización de la hora, el cambio de modo y el ajuste manual mediante botones. Utiliza colas para gestionar eventos de botones y un mutex para proteger las variables compartidas.

El reloj avanza cada segundo gracias a una tarea que actualiza los segundos, minutos y horas cuando corresponde. Otra tarea procesa los botones: uno cambia el modo (reloj, ajuste de horas o ajuste de minutos), y el otro incrementa el valor según el modo seleccionado. Además, una tarea muestra la hora por el puerto serie solo cuando hay cambios, y otra controla los LEDs, encendiendo uno cada segundo y otro cuando se está en modo ajuste.

Gracias a la multitarea, el reloj se mantiene actualizado mientras los botones y la visualización funcionan en paralelo sin interrupciones. 

La salida que se ve en la pantalla es:
```
Hora: 12:34:56  | Modo: 0
Hora: 12:34:57  | Modo: 0
Hora: 12:34:58  | Modo: 0
...

```



### **Juego Web:**
**Codigo main.cpp:**
```
#include <Arduino.h>
 #include <WiFi.h>
 #include <AsyncTCP.h>
 #include <ESPAsyncWebServer.h>
 #include <SPIFFS.h>
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "freertos/queue.h"
 #include "freertos/semphr.h"
 
 // Configuración de red WiFi
 const char* ssid = "ESP32_Game";
 const char* password = "12345678";
 
 // Definición de pines
 #define LED1 2
 #define LED2 4
 #define LED3 5
 #define BTN1 16
 #define BTN2 17
 #define BTN3 18
 #define LED_STATUS 19
 
 // Variables del juego
 volatile int puntuacion = 0;
 volatile int tiempoJuego = 30;
 volatile int ledActivo = -1;
 volatile bool juegoActivo = false;
 volatile int dificultad = 1;
 
 // Recursos RTOS
 QueueHandle_t botonQueue;
 SemaphoreHandle_t juegoMutex;
 TaskHandle_t tareaJuegoHandle = NULL;
 AsyncWebServer server(80);
 AsyncEventSource events("/events");
 
 // Estructura para eventos de botones
 typedef struct {
   uint8_t boton;
   uint32_t tiempo;
 } EventoBoton;
 
 // Función de interrupción para los botones
 void IRAM_ATTR ISR_Boton(void *arg) {
   EventoBoton evento;
   evento.boton = (uint32_t)arg;
   evento.tiempo = xTaskGetTickCountFromISR();
   xQueueSendFromISR(botonQueue, &evento, NULL);
 }
 
 // Configuración inicial del ESP32
 void setup() {
   Serial.begin(115200);
   if(!SPIFFS.begin(true)) Serial.println("Error al montar SPIFFS");
 
   pinMode(LED1, OUTPUT);
   pinMode(LED2, OUTPUT);
   pinMode(LED3, OUTPUT);
   pinMode(LED_STATUS, OUTPUT);
   pinMode(BTN1, INPUT_PULLUP);
   pinMode(BTN2, INPUT_PULLUP);
   pinMode(BTN3, INPUT_PULLUP);
 
   botonQueue = xQueueCreate(10, sizeof(EventoBoton));
   juegoMutex = xSemaphoreCreateMutex();
 
   attachInterruptArg(BTN1, ISR_Boton, (void*)BTN1, FALLING);
   attachInterruptArg(BTN2, ISR_Boton, (void*)BTN2, FALLING);
   attachInterruptArg(BTN3, ISR_Boton, (void*)BTN3, FALLING);
 
   WiFi.softAP(ssid, password);
   Serial.print("Dirección IP: "); Serial.println(WiFi.softAPIP());
 
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(SPIFFS, "/index.html", "text/html");
   });
 
   server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
     juegoActivo = !juegoActivo;
     request->send(200, "text/plain", "OK");
   });
 
   server.on("/difficulty", HTTP_GET, [](AsyncWebServerRequest *request){
     if (request->hasParam("value")) {
       int valor = request->getParam("value")->value().toInt();
       if (valor >= 1 && valor <= 5) dificultad = valor;
     }
     request->send(200, "text/plain", "OK");
   });
 
   events.onConnect([](AsyncEventSourceClient *client){
     client->send("{}", "update", millis());
   });
   server.addHandler(&events);
   server.begin();
 
   xTaskCreate(TareaJuego, "JuegoTask", 2048, NULL, 1, &tareaJuegoHandle);
   xTaskCreate(TareaLecturaBotones, "BotonesTask", 2048, NULL, 2, NULL);
   xTaskCreate(TareaTiempo, "TiempoTask", 2048, NULL, 1, NULL);
 }
 
 void loop() {
   vTaskDelay(portMAX_DELAY);
 }
 
 void TareaJuego(void *pvParameters) {
   int ultimoLed = -1;
   for (;;) {
     if (juegoActivo) {
       int nuevoLed;
       do { nuevoLed = random(0, 3); } while (nuevoLed == ultimoLed);
       ledActivo = nuevoLed;
       ultimoLed = nuevoLed;
       digitalWrite(LED1, ledActivo == 0);
       digitalWrite(LED2, ledActivo == 1);
       digitalWrite(LED3, ledActivo == 2);
     }
     vTaskDelay(pdMS_TO_TICKS(1000 - (dificultad * 150)));
   }
 }
 
 void TareaLecturaBotones(void *pvParameters) {
   EventoBoton evento;
   for (;;) {
     if (xQueueReceive(botonQueue, &evento, portMAX_DELAY)) {
       if (juegoActivo) {
         int botonPresionado = (evento.boton == BTN1) ? 0 : (evento.boton == BTN2) ? 1 : 2;
         if (botonPresionado == ledActivo) puntuacion++;
         else if (puntuacion > 0) puntuacion--;
       }
     }
   }
 }
 
 void TareaTiempo(void *pvParameters) {
   for (;;) {
     if (juegoActivo && tiempoJuego > 0) {
       tiempoJuego--;
       if (tiempoJuego == 0) juegoActivo = false;
     }
     vTaskDelay(pdMS_TO_TICKS(1000));
   }
 }

```

Este código implementa un juego de reacción en un ESP32 con WiFi y FreeRTOS. Un servidor web permite iniciar/detener el juego y ajustar la dificultad, mientras que tareas en paralelo controlan la activación de LEDs y la detección de pulsaciones de botones.

El ESP32 crea un punto de acceso ESP32_Game y maneja eventos web y físicos con colas e interrupciones. Una tarea enciende aleatoriamente un LED y el jugador debe presionar el botón correspondiente; si acierta, suma puntos, si falla, los pierde. Otra tarea gestiona el tiempo restante, finalizando el juego al llegar a cero.

El puerto serie muestra la IP del servidor y actualiza la puntuación y el tiempo restante en tiempo real. Con WiFi y multitarea, el sistema permite una experiencia de juego fluida y rápida, respondiendo a la interacción del usuario sin bloqueos.

Y la salida en este caso es:
```
Dirección IP: 192.168.4.1
Juego iniciado
Puntuación: 5
Tiempo restante: 20

```
























