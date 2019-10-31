int pin = 7;
int ledPin = 13;
unsigned long pulse_length = 0;

byte sensorValue[16];
byte pulse_value;

byte distLL= 0;
byte distL = 0;
byte distR = 0;
byte distRR= 0;

int values[4];


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

short sensorId = 0;
sensorId |= sensorValue[9];
sensorId |= sensorValue[8] << 1;


short cane = 0;
int count = 13;

for(i=0; i<8; i++){
  cane |= sensorValue[i] << count;
  count--;
  }
for(i=10; i<16; i++){
  cane |= sensorValue[i] << count;
  count--;
  }

short gatto = 0;
count = 13;

for(i=10; i<16; i++){
  gatto |= sensorValue[i] << count;
  count--;
  }
for(i=0; i<8; i++){
  gatto |= sensorValue[i] << count;
  count--;
  }


values[ sensorId ] = cane;

Serial.println(values[2], BIN);

/*
for(i=0; i<4; i++){
  Serial.print( values[i], BIN);
  Serial.print( " ");
  }
Serial.println("");
*/

///////////////////////////////////////////////////////////
/* 
  for (i = 8; i < 16; i++) {
    if(i % 4 == 0)
      Serial.print(" ");
    Serial.print(sensorValue[i], BIN);
  }

  for (i = 0; i < 8; i++) {
    if(i % 4 == 0)
      Serial.print(" ");
    Serial.print(sensorValue[i], BIN);
  }
  
  Serial.println(" ");
*/
}
