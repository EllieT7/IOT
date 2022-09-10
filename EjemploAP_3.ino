#include <WiFi.h> // Incluye la librería ESP32WiFi, utilizar todas las funciones WiFi.
#include "config.h"
#include "ESP32_Utils.hpp"
WiFiServer server(80);

//Bluetooth
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

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
// Contador 
int contador = 0;
void setup() {
  Serial.begin(115200);
  pinMode(AZUL, OUTPUT);   pinMode(VERDE, OUTPUT);     pinMode(ROJO, OUTPUT);
  // Set outputs to LOW
  digitalWrite(AZUL, LOW);    digitalWrite(VERDE, LOW);   digitalWrite(ROJO, LOW);
  ConnectWiFi_AP();
  server.begin();
  SerialBT.begin("NaoMica"); //Bluetooth nombre dispositivo
  Serial.println("Dispositivo ESPBluetooth iniciado");
}
void loop(){
  //Bluetooth
  if (Serial.available()){
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()){
    String command = "";
    command = SerialBT.readStringUntil('\n'); // Obteniendo string del Bluetooth
    // LED 1
    if (command.indexOf("azul on")>=0){
      SerialBT.println("GPIO 21 on");
      AZULState = "on";
      digitalWrite(AZUL, HIGH);
      contador++;
    } else if (command.indexOf("azul off")>=0){
      SerialBT.println("GPIO 21 off");
      AZULState = "off";
      digitalWrite(AZUL, LOW);
      contador++;
    } else if (command.indexOf("verde on")>=0){
      SerialBT.println("GPIO 22 on");
      VERDEState = "on";
      digitalWrite(VERDE, HIGH);
      contador++;
    } else if (command.indexOf("verde off")>=0){
      SerialBT.println("GPIO 22 off");
      VERDEState = "off";
      digitalWrite(VERDE, LOW);
      contador--;
    } else if (command.indexOf("rojo on")>=0){
      SerialBT.println("GPIO 23 on");
      ROJOState = "on";
      digitalWrite(ROJO, HIGH);
      contador++;
    } else if (command.indexOf("rojo off")>=0){
      SerialBT.println("GPIO 23 off");
      ROJOState = "off";
      digitalWrite(ROJO, LOW);
      contador--;
    } else {
      SerialBT.println("Comando no reconocido");
    }
  Serial.println("Contador: "+String(contador));
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
              digitalWrite(AZUL, HIGH);
              contador++;
            } else if (header.indexOf("GET /21/off") >= 0) {
              Serial.println("GPIO 21 off");
              AZULState = "off";
              digitalWrite(AZUL, LOW);
              contador--;
            } else if (header.indexOf("GET /22/on") >= 0) {
              Serial.println("GPIO 22 on");
              VERDEState = "on";
              digitalWrite(VERDE, HIGH);
              contador++;
            } else if (header.indexOf("GET /22/off") >= 0) {
              Serial.println("GPIO 22 off");
              VERDEState = "off";
              digitalWrite(VERDE, LOW);
              contador--;
            }  else if (header.indexOf("GET /23/on") >= 0) {
              Serial.println("GPIO 23 on");
              ROJOState = "on";
              digitalWrite(ROJO, HIGH);
              contador++;
            } else if (header.indexOf("GET /23/off") >= 0) {
              Serial.println("GPIO 23 off");
              ROJOState = "off";
              digitalWrite(ROJO, LOW);
              contador--;
            }

            // Página web
            client.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"></head>");
            client.println("<body><div><h1>ESP32 Servidor WEB</h1></div><div><h2>LED 1</h2></div>");
            client.println("<div style='display: grid; grid-template-columns: repeat(3, 1fr); margin: 3% 30%; text-align: center;'>");
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
            client.println("</div></div><div><h2>Contador: "+String(contador)+"</h2></div>");
            client.println("</body></html>");          
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
}
