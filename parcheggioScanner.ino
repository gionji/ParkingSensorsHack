// Included libraries
#include <Wire.h>

// #define SERIAL_OUTPUT 1

#define MAX_DIST 300
#define MAX_VALUE 13283
#define MIN_DIST 0
#define MIN_VALUE 16361
 

// Device address
#define I2C_ADDR             0x70

// I2C registers descriptions
#define EVENT_GET_RIGHT_EXTERNAL      0x41
#define EVENT_GET_RIGHT_INTERNAL     0x42
#define EVENT_GET_LEFT_EXTERNAL     0x40
#define EVENT_GET_LEFT_INTERNAL     0x43

#define OUT_TYPE uint8_t

// Output variables
OUT_TYPE VALUE_RIGHT_EXTERNAL = 0;
OUT_TYPE VALUE_RIGHT_INTERNAL = 0;
OUT_TYPE VALUE_LEFT_EXTERNAL  = 0;
OUT_TYPE VALUE_LEFT_INTERNAL  = 0;

uint8_t EVENT = 0;

int pin = 7;
int ledPin = 13;
unsigned long pulse_length = 0;

byte sensorValue[16];
byte pulse_value;

#define RIGHT_EXTERNAL 1
#define RIGHT_INTERNAL 2
#define LEFT_INTERNAL 3
#define LEFT_EXTERNAL 0

int values[4];
int distance[4];


int initialPulseDuration  = 900 ;// microseconds
int initialPulseTolerance = 100 ;// microseconde


#include <ArduinoJson.h>

void setup() {
  pinMode(pin, INPUT);
  pinMode(ledPin, OUTPUT);

  // I2c slave mode enabling
  Wire.begin(I2C_ADDR);
  Wire.onRequest(requestEvent); // data request to slave
  Wire.onReceive(receiveEvent); // data slave received

  #ifdef SERIAL_OUTPUT
    Serial.begin(115200);
  #endif
}

void loop() {

  // Aspetto il segnale di inizio che e' lungo 900 uS
  while (pulse_length < 850 || pulse_length > 1000) {
    pulse_length = pulseIn(pin, HIGH);
  }

  int i, j;
  for (i = 0; i < 16; i = i + 1) {
    pulse_length = pulseIn(pin, HIGH);
    if (pulse_length > 150 && pulse_length < 270) {
      pulse_value = B0;
    }
    else {
      pulse_value = B1;
    }
    sensorValue[i] = pulse_value;
  }

  /* get sensor number fro bites 8 and 9
  9<<0
  8<<1
  0 = a
  1 = d
  2 = c
  3 = b 
  */
  
  short sensorId = 0;
  sensorId |= sensorValue[9] << 1;
  sensorId |= sensorValue[8] << 0;
  
  int count = 13;
  
  short data = 0;
  count = 0;
  
  for(i=10; i<16; i++){
    data |= sensorValue[i] << count;
    count++;
    }  
  for(i=0; i<8; i++){
    data |= sensorValue[i] << count;
    count++;
    }
  
  values[ sensorId ] = data;
  
  for(i = 0; i<4; i++){
    distance[i] = map(values[ i ], MIN_VALUE, MAX_VALUE, MIN_DIST, MAX_DIST);
  }
  // Output variables
  VALUE_RIGHT_EXTERNAL = distance[RIGHT_EXTERNAL] > 255 ? (OUT_TYPE) 255 : (OUT_TYPE) distance[RIGHT_EXTERNAL];
  VALUE_RIGHT_INTERNAL = distance[RIGHT_INTERNAL] > 255 ? (OUT_TYPE) 255 : (OUT_TYPE) distance[RIGHT_INTERNAL];
  VALUE_LEFT_EXTERNAL  = distance[LEFT_EXTERNAL] > 255 ? (OUT_TYPE) 255 : (OUT_TYPE) distance[LEFT_EXTERNAL];
  VALUE_LEFT_INTERNAL  = distance[LEFT_INTERNAL] > 255 ? (OUT_TYPE) 255 : (OUT_TYPE) distance[LEFT_INTERNAL];

#ifdef SERIAL_OUTPUT
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
    
  root["right-ext"] = VALUE_RIGHT_EXTERNAL;
  root["right-int"] = VALUE_RIGHT_INTERNAL;
  root["left-ext"] = VALUE_LEFT_EXTERNAL;
  root["left-int"] = VALUE_LEFT_INTERNAL;

  root.prettyPrintTo(Serial);
#endif

}



// I2C management
void receiveEvent(int countToRead) {
  byte x;
  while (0 < Wire.available()) {
    x = Wire.read();
    //Serial.println(x, HEX);
  }
  //String message = "Receive event: ";
  //String out = message + x;
  
  EVENT = x;
}

void requestEvent() {
  String event_s = "0xFF";
  switch (EVENT) {
    case EVENT_GET_RIGHT_EXTERNAL: 
      Wire.write(VALUE_RIGHT_EXTERNAL);
      break;
    case EVENT_GET_RIGHT_INTERNAL: 
      Wire.write(VALUE_RIGHT_INTERNAL);
      break;
    case EVENT_GET_LEFT_EXTERNAL: 
      Wire.write(VALUE_LEFT_EXTERNAL);
      break;
    case EVENT_GET_LEFT_INTERNAL: 
      Wire.write(VALUE_LEFT_INTERNAL);
      break;
    default:
      Wire.write(0xFF);
      //event_s = String(0xFF,HEX);
      //Serial.println("Request event: " + event_s);
      break;
    }
  //Serial.println("Request event OUT: " + event_s);
}
