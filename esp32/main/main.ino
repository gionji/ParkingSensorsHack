#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "Vodafone-67706650";
const char* password = "c0nn3tt1m1";

WebServer server(80);

const int led = 13;

// Per fare il WifiAccessPoint
// https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiAccessPoint/WiFiAccessPoint.ino

// Per fare il BLuetooth master
// https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/examples/SerialToSerialBTM/SerialToSerialBTM.ino

// https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/debian_ubuntu.md

#define SERIAL_OUTPUT              1
//#define SERIAL_DEBUG               1

#ifdef SERIAL_OUTPUT
  #include <ArduinoJson.h>
#endif

#define MAX_DIST                   300
#define MAX_VALUE                  13283
#define MIN_DIST                   0
#define MIN_VALUE                  16361

// Device address
#define I2C_ADDR                   0x70

// I2C registers descriptions
#define EVENT_GET_RIGHT_EXTERNAL   0x41
#define EVENT_GET_RIGHT_INTERNAL   0x42
#define EVENT_GET_LEFT_EXTERNAL    0x40
#define EVENT_GET_LEFT_INTERNAL    0x43

#define OUT_TYPE                   uint8_t

#define RIGHT_EXTERNAL             1
#define RIGHT_INTERNAL             2
#define LEFT_INTERNAL              3
#define LEFT_EXTERNAL              0

#define PULSES_NUMBER              32

// Output variables
OUT_TYPE VALUE_RIGHT_EXTERNAL    = 0;
OUT_TYPE VALUE_RIGHT_INTERNAL    = 0;
OUT_TYPE VALUE_LEFT_EXTERNAL     = 0;
OUT_TYPE VALUE_LEFT_INTERNAL     = 0;

uint8_t EVENT                    = 0;

int parkPin                      = 7;
int ledPin                       = 13;
unsigned long pulse_length       = 0;

byte sensorValue[PULSES_NUMBER];
byte pulse_value;

int values[4];
int distance[4];

String output;



void handleRoot() {
  digitalWrite(led, 1);
 // server.send(200, "text/plain", "hello from esp8266!");
  
  server.send(200, "text/plain", output);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(parkPin, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();

   // bmw
  while (pulse_length < 2500 || pulse_length > 3500) {
    pulse_length = pulseIn(parkPin, HIGH);
  }

  int bitCount = 0;
  
  pulse_length = pulseIn(parkPin, LOW);
  while ( pulse_length < 400 ) {
    pulse_length = pulseIn(parkPin, LOW);
    if (pulse_length < 200) {
      pulse_value = B1;
    }
    else {
      pulse_value = B0;
    }
    sensorValue[bitCount] = pulse_value;
    bitCount++;
  }

#ifdef SERIAL_DEBUG
  Serial.print(bitCount);
  Serial.print( "   " );

  for(int i=0; i<PULSES_NUMBER; i++){
    if(i%8==0) Serial.print( ' ' );
    Serial.print( sensorValue[i] );
  }
    
  Serial.print( "\n" );
#endif


  int i, j;
  
  VALUE_RIGHT_EXTERNAL = 0;
  VALUE_RIGHT_INTERNAL = 0;
  VALUE_LEFT_INTERNAL = 0;
  VALUE_LEFT_EXTERNAL = 0;\
  
  for(i=0, j=0; i<8, j<8; i++, j++){
    VALUE_RIGHT_EXTERNAL |= sensorValue[i] << j;
    }  
  
  for(i=8, j=0; i<16, j<8; i++, j++){
    VALUE_RIGHT_INTERNAL |= sensorValue[i] << j;
    }  
  
  for(i=16, j=0; i<24, j<8; i++, j++){
    VALUE_LEFT_INTERNAL |= sensorValue[i] << j;
    }  
  
  for(i=24, j=0; i<32, j<8; i++, j++){
    VALUE_LEFT_EXTERNAL |= sensorValue[i] << j;
    }  

#ifdef SERIAL_DEBUG
  Serial.print(" ");
  Serial.print(VALUE_RIGHT_EXTERNAL);
  Serial.print(" ");
  Serial.print(VALUE_RIGHT_INTERNAL);
  Serial.print(" ");
  Serial.print(VALUE_LEFT_INTERNAL);
  Serial.print(" ");
  Serial.print(VALUE_LEFT_EXTERNAL);
  
  Serial.print(" ");
#endif
  
  VALUE_RIGHT_EXTERNAL = map(VALUE_RIGHT_EXTERNAL, 0, 48, 250, 0 );  
  VALUE_RIGHT_INTERNAL = map(VALUE_RIGHT_INTERNAL, 0, 48, 250, 0 ); 
  VALUE_LEFT_INTERNAL  = map(VALUE_LEFT_INTERNAL,  0, 48, 250, 0 ); 
  VALUE_LEFT_EXTERNAL  = map(VALUE_LEFT_EXTERNAL,  0, 48, 250, 0 ); 

  

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["right-ext"] = VALUE_RIGHT_EXTERNAL;
  root["right-int"] = VALUE_RIGHT_INTERNAL;
  root["left-ext"]  = VALUE_LEFT_EXTERNAL;
  root["left-int"]  = VALUE_LEFT_INTERNAL;

  //root.prettyPrintTo(Serial);
  output = "";
  root.printTo(output);
  Serial.println(output);

}
