#define RGB_RED_PIN        9
#define RGB_GREEN_PIN      10
#define RGB_BLUE_PIN       11

// uncomment this line if using a Common Anode LED
//#define COMMON_ANODE


void setup() {
  // put your setup code here, to run once:
  pinMode(15, INPUT); // Set A1 as digital input
  pinMode(16, INPUT); // set A2 as input
  pinMode(17, INPUT); // set a3 as input

  pinMode(RGB_RED_PIN,   OUTPUT); // Red
  pinMode(RGB_GREEN_PIN, OUTPUT); // Green
  pinMode(RGB_BLUE_PIN,  OUTPUT); // BLue

}



void loop() {
  setColor(255, 0, 0);  // red
  delay(1000);
  setColor(0, 255, 0);  // green
  delay(1000);
  setColor(0, 0, 255);  // blue
  delay(1000);
  setColor(255, 255, 0);  // yellow
  delay(1000);
  setColor(80, 0, 80);  // purple
  delay(1000);
  setColor(0, 255, 255);  // aqua
  delay(1000);
}



void setColor(int red, int green, int blue) {
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);
}
