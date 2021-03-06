//Declaracion de variables globales
const byte numChars = 19;
char receivedChars[numChars]; //Este array almacena los datos recibidos
boolean newData = false;
char command;
char limLuzChar[6];
char limTempChar[6];
char limHumChar[6];
int flagFAN = 0;
int flagWAT = 0;
int flagLED = 0;

//*** Sensor de temperatura y humedad - DHT AM2302 ***//
#include "DHT.h"
#define DHTPIN 2                // Definimos PIN al que nos vamos a conectar
#define DHTTYPE DHT22           // Definimos tipo de sensor que tenemos DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);       // Inicializamos sensor DHT para Arduino en 16mhz

//*** Sensor de luz - BH1750 ***//
#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;

//*** Sensor Bluetooth - HC-05 ***//
#include <SoftwareSerial.h>     // Incluimos la librería  SoftwareSerial  
SoftwareSerial BT(10,11);       // Definimos los pines RX y TX del Arduino conectados al Bluetooth

//*** Parametros ***//
#define LED 13
#define WAT 8                   // Definimos PIN para activar/desactivar bomba de agua
#define FAN 7                   // Definimos PIN para activar/desactivar ventilador

byte pinEstado = 0;             // Para ver si se activa un pin porque se detecta una medida limite (todavia no se usa)
byte pinEstadoApp = 0;          // Para ver si se activa un pin porque se dio la orden desde la App (todavia no se usa)
byte sensarLuz = 1;             // Para ver si usa el sensor de luz de arduino o no

//*** Definición de límites ***//
//#define limiteLuz 300            // Limite de luz, medido en lx
//#define limiteHumedad 65        // Limite de humedad, medido en %
//#define limiteTemperatura 25    // Limite de temperatura, medido en °C
#define tiempo_lectura 5000       // Cada cuantos milisegundos lee todos los sensores (5 s = 5000 ms)
#define tiempo_humedad 30000//3600000    // Cada cuantos milisegundos comprueba si la humedad es baja (1 h = 3600000 ms)
float limiteLuz = 300;
float limiteTemperatura = 25;
float limiteHumedad = 65;

int tiempo = tiempo_lectura;    // Para que lea los sensores cuando prende la placa
int tiempH = tiempo_humedad;    // Para que lea los sensores cuando prende la placa

void setup() {
  
  BT.begin(9600);               // Inicializamos el puerto serie BT (Para Modo AT 2)
  BT.flush();                   // Para borrar la chache del puerto serie
  delay(500);                   // Para darle tiempo que inicie el modulo BT
  
  Serial.begin(9600);           // Inicializamos  el puerto serie 

  pinMode(LED, OUTPUT);         //Define el pin para prender la luz
  digitalWrite(LED,LOW);
  pinMode(WAT, OUTPUT);         //Define el pin para prender la bomba de agua
  digitalWrite(WAT,LOW);
  pinMode(FAN, OUTPUT);         //Define el pin para prender el ventilador
  digitalWrite(FAN,LOW);
      
  Serial.println("Inicio Plantuino");
 
  dht.begin();
  lightMeter.begin();
  
}

void loop() {

  //recvWithEndMarker();
  showNewData();
  loop_bluetooth();             //Lee si se reciben datos de bluetooth desde la App
  
  delay(100);
  tiempo += 100;                //Contador de tiempo, porque la frecuencia de letura de bluetooth es mayor que la frecuencia de lectura de los sensores
  tiempH += 100;

  if (tiempo > tiempo_lectura){ //Lee los sensores cuando se cumple el tiempo seteado
 
    tiempo = 0;                 //Reinicia el tiempo

    uint16_t lux = lightMeter.readLightLevel(); //Lee sensor luz
    float t = dht.readTemperature();            //Lee sensor temperatura
    float h = dht.readHumidity();               //Lee sensor humedad
    
    Serial.print("Luz: ");
    Serial.print(lux);
    Serial.print(" lx\t");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.print(" C\t");
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.println(" %\t");
       
    //Activa o desactiva pines (prende o apaga actuadores) de acuerdo a los limites establecidos

    if(flagLED == 0 && sensarLuz) {
      if(lux<(int)limiteLuz)
        digitalWrite(LED,HIGH);
      else
        digitalWrite(LED,LOW);
    }

    if(flagFAN == 0) {
      if(t>limiteTemperatura)
        digitalWrite(FAN,HIGH);
      else
        digitalWrite(FAN,LOW);
    }
    
    if(flagWAT == 0) {
      if(tiempH>=tiempo_humedad){
        tiempH = 0;
        if(h<limiteHumedad)
        {
          digitalWrite(WAT,HIGH);
          delay(2000); // 2 seg con la bomba activa
          digitalWrite(WAT,LOW);
        }
        else
          digitalWrite(WAT,LOW);
      }
    }

  }

}

void recvWithEndMarker(){
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
    //http://forum.arduino.cc/index.php?topic=288234.0
    while(BT.available() > 0 && newData == false){
      rc = BT.read();
      BT.flush();
      if(rc != endMarker){
        receivedChars[ndx] = rc;
        ndx++;
        if(ndx >= numChars){
          ndx = numChars - 1;
          }
        }
        else {
          receivedChars[ndx] = '\0'; //Ya leimos todos los datos recibidos
          ndx = 0;
          newData = true;
          }
      }     
      
}

void showNewData(){
  
  if(newData == true){    
    Serial.print("Datos recibidos ... ");
    Serial.println(receivedChars);
    newData = false;  
  }
    
}

// Función para leer indicaciones desde Aplicacion Android
void loop_bluetooth()
{
  if(BT.available())            // Verifica si se recibieron datos de bluetooth
  {
    for(int i=0; i < 19; i++){
      receivedChars[i] = '\0';
    }
    recvWithEndMarker();
    //char command = BT.read();
    //BT.flush();
    //Serial.println("Contenido de ReceivedChars: ", receivedChars);

    //Obtengo primer paramero pasado que indica que el caso solicitado por la App
    command = receivedChars[0];
    Serial.print("Comando: ");
    Serial.println(command);
    
    switch(command){            //Lee el comando recibido
      case 'P':

        //Armo limite de luz
        limLuzChar[0] = receivedChars[2];
        limLuzChar[1] = receivedChars[3];
        limLuzChar[2] = receivedChars[4];
        limLuzChar[3] = receivedChars[5];
        limLuzChar[4] = receivedChars[6];
        limLuzChar[5] = '\0';
        Serial.print("Lim Luz Char: ");
        Serial.println(limLuzChar);

        //Armo limite de temperatura
        limTempChar[0] = receivedChars[8];
        limTempChar[1] = receivedChars[9];
        limTempChar[2] = receivedChars[10];
        limTempChar[3] = receivedChars[11];
        limTempChar[4] = receivedChars[12];
        limTempChar[5] = '\0';      
        Serial.print("Lim Temp Char: ");
        Serial.println(limTempChar);

        //Armo limite de humedad
        limHumChar[0] = receivedChars[14];
        limHumChar[1] = receivedChars[15];
        limHumChar[2] = receivedChars[16];
        limHumChar[3] = receivedChars[17];
        limHumChar[4] = receivedChars[18];
        limHumChar[5] = '\0';      
        Serial.print("Lim Hum Char: ");
        Serial.println(limHumChar);

        //Obtengo valores flotantes de los limites armados
        //http://forum.arduino.cc/index.php?topic=42139.0
        limiteLuz = atof(limLuzChar);
        Serial.print("Limite luz: ");
        Serial.println(limiteLuz);
        
        limiteTemperatura = atof(limTempChar);
        Serial.print("Limite temperatura: ");
        Serial.println(limiteTemperatura);
        
        limiteHumedad = atof(limHumChar);        
        Serial.print("Limite humedad: ");
        Serial.println(limiteHumedad);
        
        break;
      case 'V':
        Serial.println("Encender Ventilacion");
        flagFAN = 1;
        digitalWrite(FAN,HIGH);       
        break;
      case 'v':
        Serial.println("Apagar Ventilacion");
        flagFAN = 0;
        digitalWrite(FAN,LOW);
        break;
      case 'I':
        Serial.println("Encender iluminacion");
        //toggle(LED);            //Enienda/Apaga LED
        flagLED = 1;
        digitalWrite(LED,HIGH);
        break;
      case 'i':
        Serial.println("Apagar iluminacion");
        //toggle(LED);            //Enienda/Apaga LED
        flagLED = 0;
        digitalWrite(LED,LOW);
        break;
      case 'B':
        Serial.println("Encender bomba de agua");
        flagWAT = 1;          
        digitalWrite(WAT,HIGH);
        break;
      case 'b':
        Serial.println("Apagar bomba de agua");
        flagWAT = 0;
        digitalWrite(WAT,LOW);
        break;
      case 'L':
        loop_light();           //Lee sensor luz y envia datos por bluetooth a la App
        break;
      case 'T':
        loop_temperature();     //Lee sensor temperatura y envia datos por bluetooth a la App
        break;  
      case 'H':
        loop_humidity();        //Lee sensor humedad y envia datos por bluetooth a la App
        break;
      case 'O':
        toggle(LED);            //Enienda/Apaga LED
        break;
      case 'M':
        sensarLuz = 0;
        break;
      case 'm':
        sensarLuz = 1;
        break;
      case 'Z':
        if(!sensarLuz)digitalWrite(LED,HIGH); 
        break;           
      case 'z':
        if(!sensarLuz)digitalWrite(LED,LOW);
        break;
      case 'S':
        Serial.print("S");
        Serial.print(limiteLuz);
        Serial.print("|");
        Serial.print(limiteTemperatura);
        Serial.print("|");
        Serial.println(limiteHumedad);
        break;
      case 'A':                 //Lee todos los sensores y envia datos por bluetooth a la App
        Serial.println("Entro a A!");
        /*
        loop_light();
        loop_temperature();
        loop_humidity();
        */
        loop_all();
        break;
       default:
        Serial.println("Se rompio todo!");        
        break;
    }  
  }
  
  /* 
  if(Serial.available())       // Si llega un dato por el monitor serial se envía al puerto BT
  {
     BT.write(Serial.read());
  } 
  */
  
}

void loop_light() {
  
  uint16_t lux = lightMeter.readLightLevel();   // Leemos nivel de luz
  
  BT.print("Light Sensor: ");
  BT.print(lux);
  BT.println(" lx");

}

void loop_temperature() {
  
  float t = dht.readTemperature();              // Leemos temperatura

  // Chequeamos si falló al leer sensor y salimos para intentar nuevamente
  if (isnan(t)) {
    Serial.println("Falló al leer desde sensor DHT!");
    return;
  }

  BT.print("Temperature Sensor: "); 
  BT.print(t);
  BT.println(" °C");
  
}

void loop_humidity() {
  
  float h = dht.readHumidity();                 // Leemos humedad

  // Chequeamos si falló al leer sensor y salimos para intentar nuevamente
  if (isnan(h)) {
    Serial.println("Falló al leer desde sensor DHT!");
    return;
  }

  BT.print("Humidity sensor: "); 
  BT.print(h);
  BT.println(" %");

}

void loop_all(){
  uint16_t lux = lightMeter.readLightLevel();   // Leemos nivel de luz
  float t = dht.readTemperature();              // Leemos temperatura
  float h = dht.readHumidity();                 // Leemos humedad

  // Chequeamos si falló al leer sensor y salimos para intentar nuevamente
  if (isnan(t)) {
    Serial.println("Falló al leer desde sensor DHT!");
    return;
  }

  // Chequeamos si falló al leer sensor y salimos para intentar nuevamente
  if (isnan(h)) {
    Serial.println("Falló al leer desde sensor DHT!");
    return;
  }

  BT.print("Light Sensor: ");
  BT.print(lux);
  BT.print(" lx|");
  BT.print("Temperature Sensor: "); 
  BT.print(t);
  BT.print(" °C|");
  BT.print("Humidity sensor: "); 
  BT.print(h);
  BT.println(" %|");
    
}

// Función para cambiar de estado un pin
void toggle(int pinNum)
{
  pinEstadoApp = !pinEstadoApp;
  digitalWrite(pinNum, pinEstadoApp);
}
