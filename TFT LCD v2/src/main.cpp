#include "SPI.h"
#include "Adafruit_GFX.h" 
#include "Adafruit_ILI9341.h"
#include "Sprite.h"

#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

#define botonleft 19
#define botonRight 18
#define botonStart 17 // Botón de inicio conectado al pin digital 17

const int XMAX = 240; // Alto del display
const int YMAX = 320; // Ancho del display
int barrax = XMAX / 2 - 36; // Posición x de la barra 
int barraY = YMAX - 20;     // Posición y de la barra

const int barraWidth = 75;
const int barraHeight = 11;

int ball_X = XMAX / 2; // Posición inicial de la pelota en x 
int ball_Y = YMAX / 2; // Posición inicial de la pelota en y 
int ballSpeedX = 15;    // Velocidad de la pelota en X
int ballSpeedY = 12;    // Velocidad de la pelota en Y

const uint8_t RIGHT = 2;
const uint8_t LEFT = 3;

Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Declaración de funciones
void setBarraPosition(int barrax, int barraY);
void animatejuego(void);
void moverbarra(uint8_t direccion);
void moverbarraDerecha(void);
void moverbarraIzquierda(void);
void moverbola(void);
void esperarInicio(void);
void actualizarTemporizador(void);
void actualizarVidas(void);
void mostrarPerdiste(void);
void mostrarGanaste(void);

unsigned long lastBallUpdate = 0; // Tiempo de la última actualización de la bola
const unsigned long ballUpdateInterval = 1; // Intervalo de actualización en milisegundos

unsigned long startTime; // Tiempo inicial del juego
unsigned long currentTime; // Tiempo actual
int vidas = 3; // Contador de vidas

// Configuración
void setup() {
    Serial.begin(1000);
    Serial.println("juego inicializado");

    attachInterrupt(digitalPinToInterrupt(botonRight), moverbarraDerecha, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonleft), moverbarraIzquierda, HIGH);

      // Inicialización con SPI más rápido
    screen.begin(SPI_CLOCK_DIV2); // Configuración para comunicación SPI más rápida
    screen.fillScreen(ILI9341_BLUE);
  

    esperarInicio();

    screen.fillScreen(ILI9341_BLUE);
    startTime = millis(); // Inicializar el tiempo al comenzar el juego
    sei();
}

// Lazo infinito
void loop() {
    unsigned long currentMillis = millis(); // Obtiene el tiempo actual

    // Actualiza la posición de la bola según el intervalo
    if (currentMillis - lastBallUpdate >= ballUpdateInterval) {
        moverbola();
        lastBallUpdate = currentMillis; // Actualiza el tiempo de la última actualización
    }

    animatejuego();  
    actualizarTemporizador();
    actualizarVidas();
}

// Función para esperar a que se presione el botón de inicio
void esperarInicio(void) {
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(4);
    screen.setCursor(XMAX / 2 - 40, YMAX / 2);
    screen.print("PONG");
    delay(200); 
    screen.fillScreen(ILI9341_BLUE); // Pinta la pantalla de azul al inicio
    screen.setTextSize(2);
    screen.setCursor(XMAX / 2 - 50, YMAX / 4);
    screen.print("PRESIONE");
    screen.setCursor(XMAX / 2 - 35, YMAX / 3);
    screen.print("ENTER");
    screen.setCursor(XMAX / 2 - 70, YMAX / 2);
    screen.print("PARA INICIAR");

    // Espera a que se presione el botón de inicio
    while (digitalRead(botonStart) != HIGH) {
        // Esperar
    }
    delay(200); 
    // Ya no se borra la pantalla, ya está azul
    Serial.println("Juego iniciado");
}

// Fija la posición x, y de la barra
void setBarraPosition(int x1, int y1) {
    barrax = x1;
    barraY = y1;
}

void actualizarTemporizador(void) {
    currentTime = (millis() - startTime) / 1000; // Calcula el tiempo transcurrido en segundos
    screen.fillRect(0, 0, XMAX, 20, ILI9341_BLUE); // Borra el temporizador anterior
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    screen.setCursor(10, 5);
    screen.print("Tiempo: ");
    screen.print(currentTime);
    screen.print("s");

    // Verifica si han pasado 30 segundos
    if (currentTime >= 30) {
        mostrarGanaste(); // Muestra el mensaje de "Ganaste" y detiene el juego
    }
}

// Actualiza el contador de vidas
void actualizarVidas(void) {
    screen.fillRect(0, 20, XMAX, 20, ILI9341_BLUE); // Borra el texto de vidas anterior
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    screen.setCursor(10, 25);
    screen.print("Vidas: ");
    screen.print(vidas);
}

// Anima el juego
void animatejuego(void) {
    // Borra la posición anterior de la barra y la pelota
    screen.fillRect(0, barraY, XMAX, barraHeight, ILI9341_BLUE);
    screen.fillRect(ball_X - 1, ball_Y - 1, 13, 13, ILI9341_BLUE);

     // Dibuja la barra
    screen.drawBitmap(barrax, barraY, pALO, 75, 11, ILI9341_WHITE);

       // Dibuja la bola
    screen.drawBitmap(ball_X, ball_Y, bola, 11, 11, ILI9341_YELLOW);
}

// Funciones para mover la barra
void moverbarraDerecha() {
    moverbarra(RIGHT);
}

void moverbarraIzquierda() {
    moverbarra(LEFT);
}

// Variables para almacenar la posición previa de la bola
int prev_ball_X = ball_X; 
int prev_ball_Y = ball_Y;

unsigned long lastSpeedUpdate = 5; // Tiempo de la última actualización de velocidad

bool velocidadDuplicada = false; // Variable para verificar si ya se duplicó la velocidad

void moverbola() {
    // Borra la posición anterior de la bola
    screen.fillRect(prev_ball_X, prev_ball_Y, 11, 11, ILI9341_BLUE);

    // Duplica la velocidad después de 10 segundos si no se ha hecho aún
    if (!velocidadDuplicada && millis() - startTime >= 10000) { // 10 segundos desde el inicio
        ballSpeedX *= 2; // Duplica la velocidad en X
        ballSpeedY *= 2; // Duplica la velocidad en Y
        velocidadDuplicada = true; // Marca como duplicada
    }

    // Actualizar posición de la bola
    ball_X += ballSpeedX;
    ball_Y += ballSpeedY;

    // Rebote en los bordes de la pantalla
    if (ball_X <= 0 || ball_X >= XMAX - 11) {
        ballSpeedX = -ballSpeedX;
    }
    if (ball_Y <= 65) { // El límite superior es 65
        ballSpeedY = -ballSpeedY;
    }

    // Rebote en la barra del jugador
    if (ball_Y + 11 >= barraY && ball_X >= barrax && ball_X <= barrax + barraWidth) {
        ballSpeedY = -ballSpeedY;
    }

    // Reiniciar la pelota si cae al fondo
    if (ball_Y >= YMAX) {
        vidas--; // Reduce una vida
        if (vidas <= 0) {
            mostrarPerdiste(); // Llama a la función de "Perdiste"
        } else {
            ball_X = XMAX / 2;
            ball_Y = YMAX / 2;
            ballSpeedX = 6; // Reinicia la velocidad en X
            ballSpeedY = 9; // Reinicia la velocidad en Y
            velocidadDuplicada = false; // Reinicia la bandera de velocidad
            startTime = millis(); // Reinicia el temporizador de inicio
        }
    }

    // Dibuja la nueva posición de la bola
    screen.drawBitmap(ball_X, ball_Y, bola, 11, 11, ILI9341_YELLOW);

    // Actualizar posición previa
    prev_ball_X = ball_X;
    prev_ball_Y = ball_Y;
}
// Mueve la barra
void moverbarra(uint8_t direccion) {
    int delta = 5; // Cambiar este valor si deseas una distancia mayor (por ejemplo, 2).
    static unsigned long lastMoveTime = 0; // Variable para guardar el tiempo del último movimiento
    unsigned long currentTime = millis(); // Tiempo actual en milisegundos

    // Mueve la barra solo si ha pasado un intervalo de tiempo
    if (currentTime - lastMoveTime > 10) { // Cambia 50 para ajustar la velocidad (en milisegundos)
        if (direccion == RIGHT && barrax + barraWidth + delta <= XMAX) {
            barrax += delta;
        } else if (direccion == LEFT && barrax - delta >= 0) {
            barrax -= delta;
        }
        lastMoveTime = currentTime; // Actualiza el tiempo del último movimiento
    }
}

// Función para mostrar el mensaje de "Perdiste"
void mostrarPerdiste() {
    screen.fillScreen(ILI9341_BLUE); // Limpia la pantalla
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(3);
    screen.setCursor(XMAX / 2 - 70, YMAX / 2 - 20);
     Serial.println("uppss, has cometido un error, sigue intentando");
    screen.print("PERDISTE");
    delay(2000); // Muestra el mensaje por 2 segundos
    while (true) {
        // Detiene el juego para que no se reinicie ni se mueva nada
    }
}


void mostrarGanaste() {
    screen.fillScreen(ILI9341_BLUE); // Limpia la pantalla
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(3);
    screen.setCursor(XMAX / 2 - 70, YMAX / 2 - 20); // Posición centrada
    screen.print("GANASTE");
     Serial.println("felicidades has completado el juego");
    delay(2000); // Muestra el mensaje por 2 segundos
    while (true) {
        // Detener el juego indefinidamente
    }
}