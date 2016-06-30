/*******************************************************/
/*!
     MONITOREO DE TEMPERATURA Y pH CON DATA LOGGER 
     ©2016   DNSSMART  DOMOTICS & AUTOMATION    
     @file zenemyj_00xFC.ino 
     @author Andres Flores 
     @license BSD (see license.txt) 
     Este es un programa para la placa ATMEGA 328 
     Este programa trabaja tambien con la placa JIMA 
     Para informacion referente a este proyecto  
     ----> https://github.com/starkus92/zenemyj_24 
                                                   
     @section HISTORY 
     
     v2.3 -Exportar Data hacia txt
        
     v2.2 -Incorporacion modulo SD
          -Funcion de DataLogger hacia SD
     v2.1 -Conexion hacia LCD 16x2
          -Control de contraste de HMI
     v2.0 -Incorporacion modulo pH
          -Interfaz LabView I/O
          -Monitoreo desde PC
     v1.5 -Immplementacion rutina con tres salidas y dos entradas
          - Interfaz USB hacia LabView
     v1.4 -Implementacion rutina con dos salidas y una entrada
     v1.3 -Temporizacion de una segunda salida 
     v1.2 -Establecimiento rango temperatura medida
          -Conexion a una salida para controlar lamparas
     v1.1 -Interfaz DS18D20 
          -Monitor serial
*/
/*******************************************************/


//Declaramos Librerias
 #include <LiquidCrystal.h> //Libreroa LCD
 #include <SoftwareSerial.h> //Libreria SoftwareSerial
 #include <OneWire.h> // Libreria para sensores OneWire
 #include <DallasTemperature.h> //Libreria para el sensor DS18D20
 #include <SPI.h> //Libreria SPI para utilizar la SD
 #include <SD.h> //Librera para tarjetas SD

//Declaramos puertos seriales
 #define phrx 14 // Definimos el pin Rx para el sensor pH
 #define phtx 15 //Definimos el pin Tx para el sensor pH
 SoftwareSerial phserial(phrx, phtx); //Declaramos el puerto serial para el sensor pH

 //Sensor de temperatura
 #define ONE_WIRE_BUS 19 // Conectamos al pin 19 de la placa el sensor de temperatura
 OneWire oneWire(ONE_WIRE_BUS); // Configuramos un bus de comunicacion para nuestro(s) dispositivo(s) OneWire
 DallasTemperature sensors(&oneWire); // Usamos la libreria DallasTemperature del fabricante del sensor
 DeviceAddress insideThermometer = { 0x28, 0xB4, 0x6B, 0xC8, 0x04, 0x00, 0x00, 0x1F }; // Le asignamos direcciones a nuestro(s) sensor(es) 

 //Declaramos las variables para el pH
 char ph_data[20]; // Arreglo de 20 bytes para guardas los valores de pH
 char ph_computerdata[20]; // Arreglo de 20 bytes  para guardar los valores desde el PC
 //byte pc_debug=0; //if you would like to debug the pH Circuit through the serial monitor(pc/mac/other). if not set this to 0.
 byte ph_received_from_computer=0; // Caracteres recibidos desde el PC 
 byte ph_received_from_sensor=0; // Caracteres recibidos del sensor de pH 
 byte ph_startup=0; // Comprobamos la conexion de la placa con el modulo pH
 float ph=0; // Asignamos un float al valor del pH registrado
 byte ph_string_received=0; // Este byte es usado como flag para indicar que hemos recibido el string del modulo pH
 
 //Configuracion del LCD
 LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // Pines utilizados para conectar la LCD

 void setup(){
 Serial.begin(38400); // Habilitamos el puerto serial de la LCD
 phserial.begin(38400); //Habilitamos el puerto serial del pH
 sensors.begin(); // Llamamos a los sensores
 sensors.setResolution(insideThermometer, 10); // Configuramos una resolucion de 10 bits para la temperatura
 lcd.begin(16, 2); // Llamamos la libreria del LCD
 SD.begin(16); // Llamamos la libreria del SD
 pinMode(10, OUTPUT);
 }
 void loop() {
 sensors.requestTemperatures(); // Leemos el sensor de temperatura
 printTemperature(insideThermometer);
 
 phserial.listen();
 delay(100);
 if(phserial.available() > 0){ //Solo si queremos que el moudulo pH envie char´s
  //Leemos los datos del modulo pH hasta detectar un <CR> al mismo tiempo que contamos cuantos char´s recibimos
 ph_received_from_sensor=phserial.readBytesUntil(13,ph_data,20);
 /// Agregamos un 0 en el arreglo despues de haber recibido el ultimo char. Interrumpiendo la transmision y evitando 
 // llenar el buffer de datos 
 ph_data[ph_received_from_sensor]=0; 
 // Usamos una flag cuando la placa controla el ORP para ahcernos saber que el string a sido recibido.
 ph_string_received=1; 
 }
 }
 void printTemperature(DeviceAddress deviceAddress)
 {
 int decPlaces = 0; // Reseteamos 
 float tempC = sensors.getTempC(deviceAddress);
 if (tempC == -127.00) {
 lcd.print("Error al obtener Datos");
 } else {
 lcd.setCursor(0,0); // Configuramos la posicion del pH en el LCD
 lcd.print("pH:");
 lcd.print(ph, 1); // Enviamos valores de pH al LCD
 lcd.setCursor(0,1); //Configuramos la posicion de la temperatura en el LCD
 lcd.print("Temp:");
 lcd.print("C ");
 lcd.print(tempC,decPlaces); // Mostramos tempertura en centigrados
 lcd.print(" F ");
 //lcd.print(DallasTemperature::toFahrenheit(tempC),decPlaces); // Convertimos de celsius to farenheit
 delay(10000); // Tomamos una lectura cada 10 segundos

 phserial.print("R\r"); //Obtenemos una lectura de pH.
 if(ph_string_received==1){ // Comprobamos que hayamos tomado un dato
 ph=atof(ph_data); // Convertimos el string "ph_data" hacia float
 if(ph>=7.5){Serial.println("high\r");} // Comprobamos que haya sido convertido al string.
 if(ph<7.5){Serial.println("low\r");} // Comprobamos que haya sido convertido al string.
 ph_string_received=0;} // Reseteamos el flag del string.
 }

 long currentTime = millis(); // Obtenermos el tiempo en ms (desde que el programa inicia)
 File dataFile = SD.open("marth_report.txt", FILE_WRITE); // Abrimos el archivo
 if (dataFile) { // Si el txt esta disponible escribimos en el :
 dataFile.println(currentTime); // Registra el tiempo en milisegundos desde que inicia el programa
 dataFile.print(","); // Separamos con una coma
 dataFile.println(ph); // Registramos el pH
 dataFile.print(","); // Separamos con otra coma
  dataFile.println(tempC); // Registramos la temperatura en Celcius
 dataFile.print("\r"); // Terminamos el renglon
 dataFile.close();
 }
 }
