#include <SoftwareSerial.h>
SoftwareSerial debugPort(2, 3);

// debug messages
// set as 1 so as to receive debug messages
// set as 0 to not receive debug messages
#define DEBUG 1

// estimated pallete weight threshold
// least weight of pallete set to 0 kg
#define WEIGHT_THRESH 0

// pallete detected when sensor analog reading is 400
// can be adjusted when calibrating system
#define DETECTION_THRESHOLD 300

//IR sensor pins
// IR1 & IR2 are on first weighing platform
// IR3, IR4 & IR5 are on the second weighing platform
#define IR1_PIN 0
#define IR2_PIN 1

//LED pins
#define LED1_PIN 8

// ENQ character
// should be followed by a <CR> character
#define ENQ 5

// decimal places
// how many decimal places to read
#define DP 1

//String for holding the incoming data
String incomingData = "";

// weight measurement data
float weight = 0;

//flags
boolean fullReadingTaken = false;

void setup() {
  // initialize serial communication.
  Serial.begin(9600);
  debugPort.begin(9600);
  // configure pins
  pinMode(LED1_PIN, OUTPUT);

  // intialize pin states
  
  digitalWrite(LED1_PIN, LOW);
}

void loop() {
  if(palleteOnPlatform(IR1_PIN, IR2_PIN)){
    digitalWrite(LED1_PIN, HIGH); // glow LED 1
    if(DEBUG)debugPort.println("Platform sensors triggered");
    if(!fullReadingTaken)requestWeight(); // ask for weight data from indicator 1
    if(Serial.available()&& !fullReadingTaken){ // measurement available and not previously taken
      incomingData = Serial.readString(); //  read incoming data

      if(DEBUG){
        debugPort.println("Reading Indicator 1");
        debugPort.println(incomingData);
      }
      
      weight = stringToFloat(incomingData); 
      if(weight >= WEIGHT_THRESH){ // positive weight
        debugPort.println(weight);
        fullReadingTaken = true; // prevent same reading from being saved twice
      }
      delay(1000);
    }
  }
  clearFlags(); // clear the flags when pallete leaves weighing scale
  delay(100);
  digitalWrite(LED1_PIN, LOW); // turn off LED 1
  delay(100);
}

/* 
 *  clear flags when pallete leaves scale
 */
void clearFlags(){
  if(analogRead(IR1_PIN) < DETECTION_THRESHOLD && analogRead(IR2_PIN) < DETECTION_THRESHOLD && fullReadingTaken){
    fullReadingTaken = false;
  }
}

/*
 * check status of IR sensors to determine if there is a pallete on the scale
 */
boolean palleteOnPlatform(int IR1, int IR2){
  if(analogRead(IR1) > DETECTION_THRESHOLD && analogRead(IR2) > DETECTION_THRESHOLD)return true;
  else return false;
}

/*
 * convert string to float
 */
float stringToFloat(String s){
  // prepare string by trimming off the unwanted characters
  s = s.substring(s.indexOf("Net") + 3, s.indexOf('.', s.indexOf("Net"))+(DP+1));
  s.trim(); // remove spaces
  
  String whole = ""; // holds whole part
  String decimal = ""; // holds decimal part
  float f = 0; // will hold the number
  
  if(s.indexOf('.')!= -1){
    whole = s.substring(0,s.indexOf('.')); // get whole number
    decimal = s.substring(s.indexOf('.')+ 1,s.indexOf('\n')); // get decimal
    f = stringtoInt(whole); // assign the whole part of the number to the float variable
    f += float(stringtoInt(decimal)) / float(pow(10,decimal.length())); // get decimal part
  }
  else{
    f = stringtoInt(s); // assign the whole part of the number to the float variable
  }
  return f;
}

/*
 * convert from string to int
 */
int stringtoInt(String buff){
  int r = 0;
  for (int i = 0; i < buff.length(); i++){
   r = (r*10) + (buff[i] - '0');
  }
  return r;
}
 /*
  * trigger the indicator to send weight data
  */
void requestWeight(){
  Serial.write(0x05);
  Serial.write(0x0D);
  delay(10); // short delay
}

/*
 * send difference to printer
 */
void printerOutput(int i){
  debugPort.print(weight);
}

