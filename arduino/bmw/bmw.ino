// Included libraries
#include <Wire.h>

#define SERIAL_OUTPUT              1

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
OUT_TYPE VALUE_RIGHT_EXTERNAL = 0;
OUT_TYPE VALUE_RIGHT_INTERNAL = 0;
OUT_TYPE VALUE_LEFT_EXTERNAL  = 0;
OUT_TYPE VALUE_LEFT_INTERNAL  = 0;

uint8_t EVENT = 0;

int pin    = 7;
int ledPin = 13;
unsigned long pulse_length = 0;

byte sensorValue[PULSES_NUMBER];
byte pulse_value;

int values[4];
int distance[4];


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
    Serial.println("Started!");
  #endif
}

void loop() {

  // Aspetto il segnale di inizio che e' lungo 900 uS
  while (pulse_length < 2500 || pulse_length > 3500) {
    pulse_length = pulseIn(pin, HIGH);
  }

  int i, j;
  int bitCount = 0;
  
  pulse_length = pulseIn(pin, LOW);
  while ( pulse_length < 400 ) {
    pulse_length = pulseIn(pin, LOW);
    if (pulse_length < 200) {
      pulse_value = B1;
    }
    else {
      pulse_value = B0;
    }
    sensorValue[bitCount] = pulse_value;
    bitCount++;
  }


  Serial.print(bitCount);
  Serial.print( "   " );

  for(int i=0; i<PULSES_NUMBER; i++){
    if(i%8==0) Serial.print( ' ' );
    Serial.print( sensorValue[i] );
  }
    
  Serial.print( "\n" );


  for(i=0, j=8; i<8, j<8; i++, j++){
    VALUE_RIGHT_EXTERNAL |= sensorValue[i] << j;
    }  
  
  for(i=8, j=8; i<16, j<8; i++, j++){
    VALUE_RIGHT_INTERNAL |= sensorValue[i] << j;
    }  
  
  for(i=16, j=8; i<24, j<8; i++, j++){
    VALUE_LEFT_EXTERNAL |= sensorValue[i] << j;
    }  
  
  for(i=24, j=8; i<32, j<8; i++, j++){
    VALUE_LEFT_INTERNAL |= sensorValue[i] << j;
    }  
/*
#ifdef SERIAL_OUTPUT
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["right-ext"] = VALUE_RIGHT_EXTERNAL;
  root["right-int"] = VALUE_RIGHT_INTERNAL;
  root["left-ext"] = VALUE_LEFT_EXTERNAL;
  root["left-int"] = VALUE_LEFT_INTERNAL;

  root.prettyPrintTo(Serial);
#endif
*/
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
