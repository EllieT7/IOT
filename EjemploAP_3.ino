#include <WiFi.h> // Incluye la librería ESP32WiFi, utilizar todas las funciones WiFi.
#include "config.h"
#include "ESP32_Utils.hpp"
WiFiServer server(80);

//Bluetooth
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

// Constantes
#define PWM1_Ch    0
#define PWM1_Res   8
#define PWM1_Freq  1000
#define PWM2_Ch    1
#define PWM2_Res   8
#define PWM2_Freq  1000
#define PWM3_Ch    2
#define PWM3_Res   8
#define PWM3_Freq  1000
int pwm1=0,pwm2=0,pwm3=0;
bool modoRGB = false;


// Variable to store the HTTP request
String header; // Cabecera tipo String
// Variables auxiliares del estado de las salidas, para el pin
String AZULState = "off";
String ROJOState = "off";
String VERDEState = "off";
// Asignando los piones GPIO, para los tres pines
const int AZUL = 21;
const int ROJO = 22;
const int VERDE = 23;
// Contadores
int contador = 255;
int contadorAzul = 255;
int contadorVerde = 255;
int contadorRojo = 255;
int aux = 0;
int aux2 = 0;

//---------------------------------------------------------------
// Potenciometro en el GPIO 32 (Analog ADC1_CH0) 
const int potPin = 32; //ADC0 es el pin 32 -- Canal q vamos a usar
// Variable para asignar la lectura
int potValue = 0;
//---------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  configPWM(); //Configurando PWM 
  ConnectWiFi_AP();
  server.begin();
  SerialBT.begin("NaoMica"); //Bluetooth nombre dispositivo
  Serial.println("Dispositivo ESPBluetooth iniciado");
  delay(1000);
}

void loop(){
  //Configuracion canal pwm
  ledcWrite(PWM1_Ch, pwm1);
  ledcWrite(PWM2_Ch, pwm2);
  ledcWrite(PWM3_Ch, pwm3);
 
  //---------------------------------------------------------------------------
  //Puerto serial
  if (Serial.available()){
    String mensaje = Serial.readStringUntil('\n');
    //SerialBT.write(Serial.read()); 
    Serial.println(aux);
    Serial.println(mensaje);
    if(mensaje=="rgb" && aux==0){
      aux = 1;
      modoRGB = true;
      Serial.println("Ingrese los valores rgb (ej: 97 97 97)");
    }else{
      aux=0;      
      int r = split(mensaje,' ',0).toInt();
      int g = split(mensaje,' ',1).toInt();
      int b = split(mensaje,' ',2).toInt();
      cambiarColor(r,g,b);
      contadorRojo = r;
      contadorVerde = g;
      contadorAzul = b;
      Serial.println("Se configuro "+mensaje);
    }
  }
  //---------------------------------------------------------------------------
  //Bluetooth
  if (SerialBT.available()){
    String command = "";
    command = SerialBT.readStringUntil('\n'); // Obteniendo string del Bluetooth
    Serial.println("Mensaje recibido BT: " + command);
    SerialBT.println("Comando rgb");
    // LED 1
    if (command.indexOf("rgb")>=0 && aux2 == 0){
      SerialBT.println("Ingrese los valores rgb (ej: 97 97 97)");
      aux2 = 1;
    } else {
      aux2 = 0;      
      int r = split(command,' ',0).toInt();
      int g = split(command,' ',1).toInt();
      int b = split(command,' ',2).toInt();
      cambiarColor(r,g,b);
      contadorRojo = r;
      contadorVerde = g;
      contadorAzul = b;
      SerialBT.println("Se configuro "+command);
    }
  }
  delay(20);
  //---------------------------------------------------------------------------
  //Página web
  WiFiClient client = server.available();   // Escuchar a las clientes entrantes
  if (client) {                             // Si una nueva cliente se conecta,
    Serial.println("Nuevo Cliente.");       // imprime un mensaje en el puerto serieport
    String currentLine = "";                // crea una cadena para contener los datos entrantes del cliente
    while (client.connected()) {            // Loop mientras la cliente está conectado
      if (client.available()) {             // si hay bytes para leer del cliente,
        char c = client.read();             // lee un byte, luego
        Serial.write(c);                    // imprimirlo en el monitor serial
        header += c;
        if (c == '\n') {                    // si el byte es un carácter de nueva línea                                 // si la línea actual está en blanco, tienes dos caracteres de nueva línea seguidos.                                        // ese es el final de la solicitud HTTP del cliente, así que envíe una respuesta:
          if (currentLine.length() == 0) {
                                            // Los encabezados HTTP siempre comienzan con un código de respuesta (por ejemplo, HTTP/1.1 200 OK)
                                            // y un tipo de contenido para que el cliente sepa lo que viene, luego una línea en blanco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // Cambiando estado de los GPIO
            // LED 1
            if (header.indexOf("GET /21/on") >= 0) {
              Serial.println("GPIO 21 on");
              AZULState = "on";
              cambiarColor(pwm3,pwm2,contadorAzul);
            } else if (header.indexOf("GET /21/off") >= 0) {
              Serial.println("GPIO 21 off");
              AZULState = "off";
              cambiarColor(pwm3,pwm2,0);
            } else if (header.indexOf("GET /22/on") >= 0) {
              Serial.println("GPIO 22 on");
              VERDEState = "on";
              cambiarColor(pwm3,contadorVerde,pwm1);
            } else if (header.indexOf("GET /22/off") >= 0) {
              Serial.println("GPIO 22 off");
              VERDEState = "off";
              cambiarColor(pwm3,0,pwm1);
            }  else if (header.indexOf("GET /23/on") >= 0) {
              Serial.println("GPIO 23 on");
              ROJOState = "on";
              cambiarColor(contadorRojo,pwm2,pwm1);
            } else if (header.indexOf("GET /23/off") >= 0) {
              Serial.println("GPIO 23 off");
              ROJOState = "off";
              cambiarColor(0,pwm2,pwm1);
            } else if(header.indexOf("GET /contador/mas") >= 0){
              if(contador<255){
                contador = contador+1;
              }
            } else if (header.indexOf("GET /contador/menos") >= 0){
              if(contador>0){
                contador = contador-1;
              }
            } else if(header.indexOf("GET /azul/mas") >= 0){
              if(contadorAzul<255){
                contadorAzul = contadorAzul+1;
                cambiarColor(pwm3,pwm2,contadorAzul);
              }
            } else if (header.indexOf("GET /azul/menos") >= 0){
              if(contadorAzul>0){
                contadorAzul = contadorAzul-1;
                cambiarColor(pwm3,pwm2,contadorAzul);
              }
            } else if(header.indexOf("GET /verde/mas") >= 0){
              if(contadorVerde<255){
                contadorVerde = contadorVerde+1;
                cambiarColor(pwm3,contadorVerde,pwm1);
              }
            } else if (header.indexOf("GET /verde/menos") >= 0){
              if(contadorVerde>0){
                contadorVerde = contadorVerde-1;
                cambiarColor(pwm3,contadorVerde,pwm1);
              }
            } else if(header.indexOf("GET /rojo/mas") >= 0){
              if(contadorRojo<255){
                contadorRojo = contadorRojo+1;
                cambiarColor(contadorRojo,pwm2,pwm1);
              }
            } else if (header.indexOf("GET /rojo/menos") >= 0){
              if(contadorRojo>0){
                contadorRojo = contadorRojo-1;
                cambiarColor(contadorRojo,pwm2,pwm1);
              }
            }
            
            // Página web
            client.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"></head>");
            client.println("<body style='margin: 3% 10%;'><div><h1>Lab 4: ESP32 como Servidor WEB en modo Access Point</h1></div>");
            client.println("<div style='grid-column-start: 1; grid-column-end: 4;'><center><p>Contador: </p>");
            client.println("<a href=\"/contador/menos\"><button class=\"button\">-</button></a>"+String(contador));
            client.println("<a href=\"/contador/mas\"><button class=\"button\">+</button></a>");
            client.println("</center></div><div style='display: grid; grid-template-columns: repeat(3, 1fr); margin: 3% 30%; text-align: center;'>");
            client.println("<div style='grid-column-start: 1; grid-column-end: 4;'><h2>LED | Controladores</h2></div>");
            client.println("<div><p>AZUL - State " + AZULState + "</p></div>");
            client.println("<div><p>VERDE - State " + VERDEState + "</p></div>");
            client.println("<div><p>ROJO - State " + ROJOState + "</p></div>"); 
            client.println("<div style='display: grid; grid-template-columns: repeat(2, 1fr);'>");  
            if (AZULState == "off") {
              client.println("<p><a href=\"/21/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/21/off\"><button class=\"button button2\">OFF</button></a></p>");
            }   
            client.println("</div><div style='display: grid; grid-template-columns: repeat(2, 1fr);'>");  
            if (VERDEState=="off") {
              client.println("<p><a href=\"/22/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/22/off\"><button class=\"button button2\">OFF</button></a></p>");
            }  
            client.println("</div><div style='display: grid; grid-template-columns: repeat(2, 1fr);'>"); 
            if (ROJOState=="off") {
              client.println("<p><a href=\"/23/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/23/off\"><button class=\"button button2\">OFF</button></a></p>");
            }		
            client.println("</div><div style='grid-column-start: 1; grid-column-end: 4;'><p>Valores:</p></div><div>");
            client.println("<a href=\"/azul/menos\"><button class=\"button\">-</button></a>"+String(contadorAzul));
            client.println("<a href=\"/azul/mas\"><button class=\"button\">+</button></a>");
            client.println("</div><div>");
            client.println("<a href=\"/verde/menos\"><button class=\"button\">-</button></a>"+String(contadorVerde));
            client.println("<a href=\"/verde/mas\"><button class=\"button\">+</button></a>");
            client.println("</div><div>");
            client.println("<a href=\"/rojo/menos\"><button class=\"button\">-</button></a>"+String(contadorRojo));
            client.println("<a href=\"/rojo/mas\"><button class=\"button\">+</button></a>");
            client.println("</div></div></body></html>");          
            // La respuesta HTTP termina con otra línea en blanco
            client.println();
            // Salir del bucle while
            break;
          } else { // si obtuvo una nueva línea, borre la línea actual
            currentLine = "";
             }
        } else if (c != '\r') {  // si obtuvo algo más que un carácter de retorno de carro,
          currentLine += c;      // agregarlo al final de la línea actual
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  //---------------------------------------------------------------------------------------------
  // ADC
  // Leyendo valores del ADC
  potValue = analogRead(potPin);
  Serial.println(potValue);
  delay(50); // Darle tiempo para ejecutar la seria
}


//-----------------------------------------------------------------------------------------------
// Funciones
void configPWM(){
  ledcAttachPin(AZUL, PWM1_Ch);
  ledcSetup(PWM1_Ch, PWM1_Freq, PWM1_Res);

  ledcAttachPin(VERDE, PWM2_Ch);
  ledcSetup(PWM2_Ch, PWM2_Freq, PWM2_Res);

  ledcAttachPin(ROJO, PWM3_Ch);
  ledcSetup(PWM3_Ch, PWM3_Freq, PWM3_Res);
}

String split(String data, char separator, int index){
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void cambiarColor(int r, int g, int b){
  if(r>=0&&r<=255 && r>=0&&r<=255 && r>=0&&r<=255){
    if(r>0){
      ROJOState = "on";
    }else{
      ROJOState = "off";
    }
    if(g>0){
      VERDEState = "on";
    }else{
      VERDEState = "off";
    }
    if(b>0){
      AZULState = "on";
    }else{
      AZULState = "off";
    }
    pwm1=b;
    pwm2=g;
    pwm3=r;
  }else{
    Serial.println("Intente nuevamente");
  }
}
