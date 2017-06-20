
//*** DHT AM2302 *****************************
#include "DHT.h"
#define DHTPIN 2     // what pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302)
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

//*** BH1750 *********************************
#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;


//*** HC-05 **********************************
#include <SoftwareSerial.h>   // Incluimos la librería  SoftwareSerial  
SoftwareSerial BT(10,11);    // Definimos los pines RX y TX del Arduino conectados al Bluetooth

//*** Parametros **********************************
#define LED 13
#define WAT 8
#define FAN 7

byte pinEstado = 0; //Para ver si se activa un pin porque se detecta una medida limite (todavia no se usa)
byte pinEstadoApp = 0; //Para ver si se activa un pin porque se dio la orden desde la App (todavia no se usa)

#define limiteLuz 30 //Limite de luz
#define limiteHumedad 50 //Limite de humedad
#define limiteTemperatura 25 //Limite de temperatura
#define tiempo_lectura 5000 //Cada cuantos milisegundos lee los sensores para activar o no los pines
int tiempo = tiempo_lectura; // Para que lea los sensores cuando prende la placa

void setup() {
  BT.begin(9600);       // Inicializamos el puerto serie BT (Para Modo AT 2)
  BT.flush();           // Para borrar la chache del puerto serie
  delay(500);           // Para darle tiempo que inicie el modulo BT
  
  Serial.begin(9600);   // Inicializamos  el puerto serie 

  pinMode(LED, OUTPUT); //Define el pin para prender la luz
  digitalWrite(LED,LOW);
  pinMode(WAT, OUTPUT); //Define el pin para prender la bomba de agua
  digitalWrite(WAT,LOW);
  pinMode(FAN, OUTPUT); //Define el pin para prender el ventilador
  digitalWrite(FAN,LOW);
      
  Serial.println("Inicio Plantuino");
 
  dht.begin();
  lightMeter.begin();
}

void loop() {

  loop_bluetooth(); //Lee si se reciben datos de bluetooth desde la App
  
  delay(100);
  tiempo += 100; //Contador de tiempo, porque la frecuencia de letura de bluetooth es mayor que la frecuencia de lectura de los sensores

  
  if (tiempo > tiempo_lectura){ //Lee los sensores cuando se cumple el tiempo seteado
 
    tiempo = 0; //Reinicia el tiempo

    uint16_t lux = lightMeter.readLightLevel(); //Lee sensor luz
    float h = dht.readHumidity(); //Lee sensor humedad
    float t = dht.readTemperature(); //Lee sensor temperatura
    
    Serial.print("Luz: ");
    Serial.print(lux);
    Serial.print(" lx\t");
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" C\t");
   
    //Activa o desactiva pines (prende o apaga actuadores) de acuerdo a los limites establecidos
    digitalWrite(LED, (lux<limiteLuz));
    digitalWrite(WAT, (h<limiteHumedad));
    digitalWrite(FAN, (t>limiteTemperatura));

  }

}

//Funcion para cambiar de esado un pin
void toggle(int pinNum)
{
  pinEstadoApp = !pinEstadoApp;
  digitalWrite(pinNum, pinEstadoApp);
}

void loop_bluetooth()
{

  if(BT.available()) //Verifica si se recibieron datos de bluetooth
  {
    char command = BT.read();
    BT.flush();
    Serial.println(command);

    switch(command){ //Lee el comando recibido
      case 'V':
        //Encender Ventilacion
        break;
      case 'v':
        //Apagar Ventilacion
        break;
      case 'I':
        //Encender iluminacion
        toggle(LED); //Enienda/Apaga LED
        break;
      case 'i':
        //Apagar iluminacion
        toggle(LED); //Enienda/Apaga LED
        break;
      case 'B':
        //Encender bomba de agua
        break;
      case 'b':
        //Apagar bomba de agua
        break;
      case 'L':
        loop_light(); //Lee sensor luz y envia datos por bluetooth a la App
        break;
      case 'T':
        loop_temperature(); //Lee sensor temperatura y envia datos por bluetooth a la App
        break;  
      case 'H':
        loop_humidity(); //Lee sensor humedad y envia datos por bluetooth a la App
        break;
      case 'O':
        toggle(LED); //Enienda/Apaga LED
        break;           
      case 'A':  //Lee todos los sensores y envia datos por bluetooth a la App
        loop_light();
        loop_temperature();
        loop_humidity();
        break;           
    }
  }
      
  if(Serial.available())  // Si llega un dato por el monitor serial se envía al puerto BT
  {
     BT.write(Serial.read());
  } 
     
}

void loop_light() {
  uint16_t lux = lightMeter.readLightLevel();
  BT.print("Sensor Luz: ");
  BT.print(lux);
  BT.println(" lx");
}

void loop_temperature() {
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  BT.print("Sensor Temperatura: "); 
  BT.print(t);
  BT.println(" C");

}

void loop_humidity() {
  float h = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  BT.print("Sensor Humedad: "); 
  BT.print(h);
  BT.println(" %\t");

}
