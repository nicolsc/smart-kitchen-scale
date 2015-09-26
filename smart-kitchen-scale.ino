#include <Wire.h>
#include <SoftwareSerial.h>

#include "rgb_lcd.h"
#include "HX711.h"

#define DOUT  3
#define CLK  2
#define CALIBRATION -420 

#define BUZZER 8

#define SIGFOX_RX 5
#define SIGFOX_TX 4

#define LED_PIN 13

#define RED 0
#define GREEN 1
#define BLACK 2
#define WHITE 3

#define WEIGHT_EVERY 60000

SoftwareSerial sigfox(SIGFOX_RX,SIGFOX_TX);
rgb_lcd lcd;
HX711 scale(DOUT, CLK);

long zero_factor;
float weight;


void setup() {
  Serial.begin(9600);
  sigfox.begin(9600);
     
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  lcd.begin(16,2,1);
  setScreenColour(BLACK);
  lcd.write("INIT THE MIGHTY SCALE");
  lcd.setCursor(0,1);
  lcd.write("PLEASE WAIT");
  scale.set_scale();
  scale.tare();	//Reset the scale to 0

  zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  lcd.clear();
  lcd.write("ALL SET ! READY TO USE");
  setScreenColour(WHITE);
  delay(500);
  
    sigfox.print("AT&V\n");
  delay(3500);
}

void loop() {  
  while (sigfox.available()){
    Serial.write(sigfox.read());
  }

  digitalWrite(BUZZER, LOW);
  lcd.clear();
  lcd.setCursor(0,0);
  
  scale.set_scale(CALIBRATION);
  weight = scale.get_units();
  Serial.print(weight);
  Serial.print(" grams");
  Serial.println();

  lcd.print(weight);
  lcd.setCursor(0,1);
  
  sendSigfox(weight);
  
  if (weight < 10){
    lowAlert();
  }
  else{
   fine();

  }
  
  delay(WEIGHT_EVERY);
}

void sendSigfox(double value){
  int i = (int)(weight*100);
  String frame = String(i, HEX);
  
  if (frame.length() % 2 == 1){
    frame = "0"+frame;
  }
  Serial.print("AT$SF=");
  Serial.print(frame);
  Serial.println();
  
  sigfox.print("AT$SF=");
  sigfox.print(frame);
  sigfox.print("\n"); 
}
void setScreenColour(int colour){
  switch (colour){
    case RED:
      lcd.setRGB(255,0,0);
      break;
    case GREEN:
      lcd.setRGB(0,255,0);
      break;
    case BLACK:
      lcd.setRGB(0,0,0);
      break;
    case WHITE:
      lcd.setRGB(200,200,200);
      break;
    default:
      lcd.setRGB(0,100,200);
  }
}
void lowAlert(){
  setScreenColour(RED);
  buzz();
  lcd.setCursor(0,1);
  lcd.write("~~~ LOW LOW LOW ~~~");
}
void fine(){
  setScreenColour(GREEN);
  lcd.setCursor(0,1);
  lcd.write(":)");
}
void buzz(){
  digitalWrite(BUZZER, HIGH);
}
