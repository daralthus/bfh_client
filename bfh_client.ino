/*
  Wifi sensor client
 
  This sketch connects an analog temperature, humidity, light sensors and the "secret voltage" to Cosm (http://www.cosm.com) and another optional server
  using an Arduino Wifi shield.

  It's based on the Arduino Wifi example by Tom Igoe and Scott Fitzgerald
  additionally it contains code from:
  -humidity code from: http://itp.nyu.edu/physcomp/sensors/Reports/HIH-4030
  -temp code from: http://bildr.org/2011/01/tmp102-arduino/
  -hidden voltage code from: http://code.google.com/p/tinkerit/wiki/SecretVoltmeter

  This is written for a network using WPA encryption. For 
  WEP or WPA, change the Wifi.begin() call accordingly.

  Circuit:
    http://budapestfarmershack.fictionlab.hu/?p=25
    * i2c tmp102 temperature sensor attached to analog in 4,5
    * light sensor to analog 0
    * HIH-4030 humidity sensor to analog 1
    * Wifi shield attached to pins 10, 11, 12, 13

  This code is in the public domain.
 
 */
 
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include "config.h"

// SETUP
// init
char ssid[] = NETWORK;
char pass[] = NETWORKPASS;


int status = WL_IDLE_STATUS;

// initialize the library instance:
WiFiClient client;

unsigned long lastConnectionTime = 0;  // last time you connected to the server, in milliseconds
boolean lastConnected = false;  // state of the connection last time through the main loop


char data[100]; //buffer for data
char dataJSON[100]; 

unsigned int i = 0; //number of updates

// temp setup
int tmp102Address = 0x48;

// light setup
int lightPin = LIGHTPIN;
int light = 0;

// temperature setup
float celsius = 0.0;

// humidity setup
int humidityPin = HUMIDTYPIN;
int humidityReading = 0;
float humidityVoltage = 0.0;
float humidityPercentage = 0.0;

//voltage
int voltage = 0;


void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600); 
  // while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  // }
  
  Wire.begin();
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(NETWORK);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(NETWORK, NETWORKPASS);

    // wait 10 seconds for connection:
    delay(10000);
  } 
  // you're connected now, so print out the status:
  printWifiStatus();
}



//LOOP
void loop() {
  delay(TIMETOUPDATE);

  //updates
  i++;

  //light
  light = analogRead(lightPin);
  
  //temp
  celsius = getTemperature();
  
  //humidity
  humidityReading = analogRead(humidityPin); 
  humidityVoltage = humidityReading * 5;
  humidityVoltage /= 1024.0;
  // convert to percentage
  humidityPercentage = humidityVoltage * 100;
  humidityPercentage /= 5;
  
  //voltage
  voltage = readVcc();
  
  printSerial();

  sprintf(data,"updates,%d\nlight,%d\ntemperature,%d\nhumidity,%d\nvoltage,%d",i,light,(int)celsius,(int)humidityPercentage,voltage);
  sprintf(dataJSON,"{\"updates\":%d,\"light\":%d,\"temperature\":%d,\"humidity\":%d,\"voltage\":%d}",i,light,(int)celsius,(int)humidityPercentage,voltage);

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected()) {
    sendData(SERVERHOST, SERVERPATH, SERVERAPIKEY, "application/json", dataJSON);
    sendData(COSMHOST, COSMPATH, COSMAPIKEY, "text/cvs", data);
   
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}



// METHODS
// print
void printSerial() {
  Serial.println("=======================");
  
  // number of updates
  Serial.print("Number of updates: ");
  Serial.println(i);

  // light
  Serial.print("Light: ");
  Serial.println(light, DEC);

  // temperature
  Serial.print("Celsius: ");
  Serial.println(celsius);

  // humidty
  Serial.print("Humidity: ");
  Serial.print(humidityPercentage);
  Serial.println("%");

  //hiddenvoltage
  Serial.print("Voltage in: ");
  Serial.println(voltage, DEC);
  
  Serial.println("=======================");
}


// this method makes a HTTP connection to the server:
void sendData(char *host, char *path, char *apikey, char *contentype, char *data) {
  // if there's a successful connection:
  if (client.connect(host, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("PUT ");
    client.print(path);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(host);
    client.print("X-APIKEY: ");
    client.println(COSMAPIKEY);
    client.print("User-Agent: ");
    client.println(USERAGENT);
    client.print("Content-Length: ");
    client.println(strlen(data));

    // last pieces of the HTTP PUT request:
    client.print("Content-Type: ");
    client.println(contentype);
    
    client.println("Connection: close");
    client.println();
    
    Serial.println(data);

    // here's the actual content of the PUT request:
    client.println(data);
  
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
   // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


// temperature
float getTemperature() {
  Wire.requestFrom(tmp102Address,2); 

  byte MSB = Wire.read();
  byte LSB = Wire.read();

  int TemperatureSum = ((MSB << 8) | LSB) >> 4; 

  float celsius = TemperatureSum*0.0625;
  return celsius;
}


// hiddenvoltage
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}



