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

void setup() {
  pinMode(pin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
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
/////////////////////////////////////////////////////////////


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
  distance[i] = map(values[ i ], 16361, 13280, 0, 300);
}


Serial.println("  ");

for(i = 0; i<4; i++){
  Serial.print(i);
  Serial.print("  ");
  Serial.print( values[ i ]);
  Serial.print("  ");
  Serial.println( distance[ i ]);
}


}
