// Included libraries
#include <Wire.h>

#define SERIAL_OUTPUT              1
#define SERIAL_DEBUG               1
#define KITT_ENABLED               1

#ifdef SERIAL_OUTPUT
  #include <ArduinoJson.h>
#endif

#include <Adafruit_NeoPixel.h>


#define MAX_DIST                   250
#define MAX_VALUE                  48
#define MIN_DIST                   0
#define MIN_VALUE                  0

// Device address
#define I2C_ADDR                   0x70

// ppins
#define PARK_PIN                   7
#define LED_PIN                    13
#define KITT_PIN                   6
#define RGB_RED_PIN                9
#define RGB_GREEN_PIN              10
#define RGB_BLUE_PIN               11

#define KITT_NUM_PIXELS            55

// I2C registers descriptions
#define EVENT_GET_RIGHT_EXTERNAL   0x41
#define EVENT_GET_RIGHT_INTERNAL   0x42
#define EVENT_GET_LEFT_EXTERNAL    0x40
#define EVENT_GET_LEFT_INTERNAL    0x43

#define EVENT_DISABLE_LED          0x50
#define EVENT_ENABLE_LED           0x51
#define EVENT_SET_LED_RED          0x52
#define EVENT_SET_LED_GREEN        0x53
#define EVENT_SET_LED_BLUE         0x54

#define EVENT_DISABLE_LED          0x60
#define EVENT_ENABLE_LED           0x61
#define EVENT_SET_LED_RED          0x62



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

unsigned long pulse_length       = 0;

byte sensorValue[PULSES_NUMBER];
byte pulse_value;

int values[4];
int distance[4];

#ifdef KITT_ENABLED
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
#endif

int kittEnabled = 0;


void setup() {
  pinMode(PARK_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // I2c slave mode enabling
  Wire.begin( I2C_ADDR );
  Wire.onRequest( requestEvent ); // data request to slave
  Wire.onReceive( receiveEvent ); // data slave

#ifdef KITT_ENABLED
  strip.begin();
  clearStrip(); // Initialize all pixels to 'off'
#endif

  #ifdef SERIAL_OUTPUT
    Serial.begin(115200);
    Serial.println("Started!");
  #endif
}




void loop() {
  // bmw
  while ( pulse_length < 2500 || pulse_length > 3500 ) {
    pulse_length = pulseIn( PARK_PIN, HIGH );
  }

  int bitCount = 0;

  pulse_length = pulseIn( PARK_PIN, LOW );
  while ( pulse_length < 400 ) {
    pulse_length = pulseIn( PARK_PIN, LOW );
    if ( pulse_length < 200 ) {
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
  VALUE_LEFT_EXTERNAL = 0;

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

  VALUE_RIGHT_EXTERNAL = map(VALUE_RIGHT_EXTERNAL, MIN_VALUE, MAX_VALUE, MAX_DIST, MIN_DIST );
  VALUE_RIGHT_INTERNAL = map(VALUE_RIGHT_INTERNAL, MIN_VALUE, MAX_VALUE, MAX_DIST, MIN_DIST );
  VALUE_LEFT_INTERNAL  = map(VALUE_LEFT_INTERNAL,  MIN_VALUE, MAX_VALUE, MAX_DIST, MIN_DIST );
  VALUE_LEFT_EXTERNAL  = map(VALUE_LEFT_EXTERNAL,  MIN_VALUE, MAX_VALUE, MAX_DIST, MIN_DIST );


#ifdef SERIAL_OUTPUT
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["right-ext"] = VALUE_RIGHT_EXTERNAL;
  root["right-int"] = VALUE_RIGHT_INTERNAL;
  root["left-ext"]  = VALUE_LEFT_EXTERNAL;
  root["left-int"]  = VALUE_LEFT_INTERNAL;

  root.prettyPrintTo(Serial);
#endif

  /// Kitt
  if(kittEnabled){
  // knightRider(3, 32, 4, 0xFF1000); // Cycles, Speed, Width, RGB Color (original orange-red)
  }


}



// I2C management
void receiveEvent(int countToRead) {
  byte x;
  while (0 < Wire.available()) {
    x = Wire.read();
  }
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


/*
 *  KITT
 */

 // Cycles - one cycle is scanning through all pixels left then right (or right then left)
 // Speed - how fast one cycle is (32 with 16 pixels is default KnightRider speed)
 // Width - how wide the trail effect is on the fading out LEDs.  The original display used
 //         light bulbs, so they have a persistance when turning off.  This creates a trail.
 //         Effective range is 2 - 8, 4 is default for 16 pixels.  Play with this.
 // Color - 32-bit packed RGB color value.  All pixels will be this color.
 // knightRider(cycles, speed, width, color);
 void knightRider(uint16_t cycles, uint16_t speed, uint8_t width, uint32_t color) {
   uint32_t old_val[NUM_PIXELS]; // up to 256 lights!
   // Larson time baby!
   for(int i = 0; i < cycles; i++){
     for (int count = 1; count<NUM_PIXELS; count++) {
       strip.setPixelColor(count, color);
       old_val[count] = color;
       for(int x = count; x>0; x--) {
         old_val[x-1] = dimColor(old_val[x-1], width);
         strip.setPixelColor(x-1, old_val[x-1]);
       }
       strip.show();
       delay(speed);
     }
     for (int count = NUM_PIXELS-1; count>=0; count--) {
       strip.setPixelColor(count, color);
       old_val[count] = color;
       for(int x = count; x<=NUM_PIXELS ;x++) {
         old_val[x-1] = dimColor(old_val[x-1], width);
         strip.setPixelColor(x+1, old_val[x+1]);
       }
       strip.show();
       delay(speed);
     }
   }
 }

 void clearStrip() {
   for( int i = 0; i<NUM_PIXELS; i++){
     strip.setPixelColor(i, 0x000000); strip.show();
   }
 }

 uint32_t dimColor(uint32_t color, uint8_t width) {
    return (((color&0xFF0000)/width)&0xFF0000) + (((color&0x00FF00)/width)&0x00FF00) + (((color&0x0000FF)/width)&0x0000FF);
 }
