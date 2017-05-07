/*****Cabeceras para DHT22*****/
#include "DHT.h"
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

/*****Cabeceras para BMP180*****/
#include <SFE_BMP180.h>
#include <Wire.h>
// You will need to create an SFE_BMP180 object, here called "pressure":
SFE_BMP180 pressure;
#define ALTITUDE 1655.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters

/*****Cabeceras para BH1750*****/
#include <Wire.h>
#include <BH1750.h>
/*
  BH1750 can be physically configured to use two I2C addresses:
    - 0x23 (most common) (if ADD pin had < 0.7VCC voltage)
    - 0x5C (if ADD pin had > 0.7VCC voltage)

  Library use 0x23 address as default, but you can define any other address.
  If you had troubles with default value - try to change it to 0x5C.
*/
BH1750 lightMeter(0x23);

void setup() {

  Serial.begin(9600); 
  Serial.println("Bienvenido a Plantuino! ;)"); 
  
  /*Setup para DHT22*/  
  if(dht.begin())
    Serial.println("DHT22 init exitoso!");
  else
  {
    Serial.println("DHT22 init falló!");
    while(1);  // Pause forever.
  }

  /*Setup para BMP180*/
  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init exitoso!");
  else
  {
    Serial.println("BMP180 init falló!");
    while(1); // Pause forever.
  }

  /*Setup para BH1750*/
  if(lightMeter.begin(BH1750_CONTINUOUS_HIGH_RES_MODE))
    Serial.println("BH1750 init exitoso!");
  else
  {
    Serial.println("BH1750 init falló!");  
  }  
  
}

void loop() {
  
  /*Funcionalidad para DHT22*/  
  // Esperamos un tiempo para tomar las medidas.
  delay(5000);

  // La lectura de la temperatura y humedad toma alrededor de 250 milisegundos.
  // Las lecturas del sensor también pueden ser de hasta 2 segundos.
  // Leo humedad
  float h = dht.readHumidity();
  // Leo temperatura en grados Celsius
  float t = dht.readTemperature();
  // Leo temperatura en grados Fahrenheit
  float f = dht.readTemperature(true);
  
  // Chequeo si alguna lectura fallo y salgo del loop para intentar nuevamente.
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Falla al leer desde Sensor DHT22!");
    return;
  }

  // Calculo el indice de calor, parametro para expresar la sensación térmica
  // Utilizamos temperatura en grados Celsius
  float hi = dht.computeHeatIndex(t, h);

  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: "); 
  Serial.print(t);
  Serial.print(" °C ");
  Serial.print(f);
  Serial.print(" °F\t");
  Serial.print("Indice de Calor: ");
  Serial.print(hi);
  Serial.println(" °C");

  //TODO: Definir rango de valores limiteas para manejar activadores
  if(t>30){
    //Encender ventilación y apagar LED  
  }else{
    //Apagar ventilación y prender LED para calentar ambiente.  
  }

  //TODO: Definir rango de valores limites para manejar activadores
  if(h>50){
    //Encendemos ventilación y apagamos LED
  }else{
    //Apagamos ventilación
  }  
  /*Fin funcionalidad DHT22*/

  /*Funcionalidad para BMP180*/
  char status;
  double T,P,p0,a;

  // Si desea una presión compensada por el nivel del mar, tal como se utiliza en los informes meteorológicos,
  // necesitará saber la altitud en la que se toman sus medidas.
  // Estamos usando una constante llamada ALTITUD en este sketch:
  
  Serial.println();
  Serial.print("Altitud proveida: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" metros, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" pies");

  // Para mejorar la lectura de presión, primero se debe obtener una medida de temperatura.
  // Iniciar una medición de temperatura:
  // Si la solicitud es satisfactoria, se devuelve el número de ms (milisegundos) a esperar.
  // Si la solicitud no tiene éxito, se devuelve 0.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Esperamos para completar la medida de temperatura
    delay(status);

    // Retrieve the completed temperature measurement:
    // La medida de temperatura es almacenada en la variable T.
    // La función retorna 1 si fue exitosa o 0 si falló.
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      Serial.print("Temperatura: ");
      Serial.print(T,2);
      Serial.print(" °C, ");
      
      // Iniciamos medición de presión:
      // El parámetro es el ajuste de sobremuestreo, de 0 a 3 (más alto, más largo).
      // Si la solicitud es satisfactoria, se devuelve el número de ms (milisegundos) a esperar.
      // Si la solicitud no tiene éxito, se devuelve 0.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Esperamos para completar la medida de presión:
        delay(status);

        // Obtenemos la medición de presión completada:
        // Observe que la medición se almacena en la variable P.
        // Observe también que la función requiere la medición de temperatura anterior (T).
        // (Si la temperatura es estable, puede realizar una medición de temperatura para una serie de mediciones de presión.)
        // Función devuelve 1 si tiene éxito, 0 si falla.
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          Serial.print("Presión Absoluta: ");
          Serial.print(P,2);
          Serial.print(" mb (milibares), ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg (pulgadas de Mercurio)");

          // El sensor de presión devuelve una presión absoluta, que varía con la altitud.
          // Para eliminar los efectos de la altitud, utilice la función de nivel de mar y su altitud actual.
          // Este número se utiliza comúnmente en los informes meteorológicos.
          // Parámetros: P = presión absoluta en mb (milibares), ALTITUD = altitud actual en m (metros).
          // Resultado: p0 = presión compensada en el nivel del mar en mb (milibares)

          p0 = pressure.sealevel(P,ALTITUDE);
          Serial.print("Presión relativa a nivel del mar: ");
          Serial.print(p0,2);
          Serial.print(" mb (milibares), ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg (pulgadas de Mercurio)");

          // Por otro lado, si se quiere determinar la altitud a través de la lectura de presión
          // usar la función de altitud a través de una presión base (por ejemplo nivel del mar).
          // Parametros: P = presión absoluta en mb (milibares), p0 = presión base en mb (milibares).
          // Resultado: a = altitud en metros.

          a = pressure.altitude(P,p0);
          Serial.print("Altitud determinada: ");
          Serial.print(a,0);
          Serial.print(" metros, ");
          
          //TODO: Definir rango de valores limites para manejar activadores
          //Si altura es mayor a tantos metros dar mas oxigeno prendiendo ventilación sino apagar ventilacion

          //Si presion es mayor a tantos milibares prender ventilación para enfriar ambiente disminuyendo presión, si es menor a un determinado nivel prender LED
          
        }
        else Serial.println("Error obteniendo medida de presión\n");
      }
      else Serial.println("Error iniciando medida de presión\n");
    }
    else Serial.println("Error obteniendo medida de temperatura\n");
  }
  else Serial.println("Error iniciando medida de temperatura\n");
  /*Fin funcionalidad BMP180*/

  /*Funcionalidad BH1750*/
  uint16_t lux = lightMeter.readLightLevel();
  Serial.print("Luz: ");
  Serial.print(lux);
  Serial.println(" lx");

  //TODO: Definir rango de valores limites para manejar actuadores
  if(lux == 0){
    //Encendemos LED, habría que verificar que ya no se encuentren encendidos por tema de calentar ambiente.
  }else{
    //Si hay luz y los LED están prendidos se deberían apagar para ahorrar energía. Para este caso se debería considerar la temperatura en el ambiente.  
  }
  /*Fin funcionalidad BH1750*/

  delay(30000); //Esperamos 30 segundos para realizar el próximo ciclo
}
