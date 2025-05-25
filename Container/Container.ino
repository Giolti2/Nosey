#include <Adafruit_NeoPixel.h>

#define VIBPIN 3
#define VIBTIME 5000

#define LEDPIN 7
#define LEDN 16

#define LID 4

Adafruit_NeoPixel strip(LEDN, LEDPIN, NEO_GRB + NEO_KHZ800);

bool isOn = false;
bool wasOff = true;
bool wasOn = false;
bool vibrate = false;
bool changeLed = false;
bool lidOpen = false;
bool lidWasOpen = false;

bool recipe = false;

int intensity = 0;
uint32_t ledColor;

//colors list
uint32_t colorList[6] ={
  strip.Color(150, 100, 80), // less 1 led
  strip.Color(200, 121, 150),
  strip.Color(255, 110, 197), 
  strip.Color(255, 0, 55), // most intensive
  strip.Color(40, 0, 0), // spoiled
  strip.Color(255,0,0)
};

long timer = 0;

void setup() {
  Serial.begin(9600);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  pinMode(LID, INPUT_PULLUP);
}

void loop() {
  if(Serial.available()>0){
    String msg = Serial.readString();
    msg.trim();

    //read from other arduino
    if(msg == "on"){
      isOn = true;
      wasOff = true;
      timer = millis();
    }
    else if(msg == "stop"){
      isOn = false;
      wasOn = true;

      vibrate = false;
    }

    //intensity flag
    else if(msg == "0"){
      intensity = 0;
      changeLed = true;
    }
    else if(msg == "1"){
      intensity = 1;
      changeLed = true;
    }
    else if(msg == "2"){
      intensity = 2;
      changeLed = true;
    }
    else if(msg == "3"){
      intensity = 3;
      changeLed = true;
    }
    else if(msg == "4"){
      intensity = 4;
      changeLed = true;
    }
    else if(msg == "5"){
      intensity = 5;
      changeLed = true;
    }
  }

  //lid status
  if(digitalRead(LID) == LOW){ //closed
    if(lidOpen){
      lidWasOpen = true;
      Serial.print("lid_close");
    }
    else
      lidWasOpen = false;

    lidOpen = false;
  }
  else{ //open
    if(lidOpen){
      lidWasOpen = true;
    }
    else{
      lidWasOpen = false;
      Serial.print("lid_open");
    }

    lidOpen = true;

    vibrate = false;
  }

  if(isOn){
    if(wasOff){
      wasOff = false;

      startLed();
      startVibration();
    }

    if(changeLed){
      changeLed = false;
      startLed();
    }

    vibrationRoutine();
  }
  else if(wasOn){
    wasOn = false;

    stopLed();
    stopVibration();
  }
}

void startVibration(){
  if(intensity == 3){
    vibrate = true;
    timer = millis();
  }
  return;
}

void stopVibration(){
  analogWrite(VIBPIN, 0);
  return;
}

void vibrationRoutine(){
  if(vibrate){
    if((millis() - timer) < 3000)
    analogWrite(VIBPIN, 255);
    else if((millis() - timer) < 3500)
      analogWrite(VIBPIN, 0);
    else if((millis() - timer) < 6500)
      analogWrite(VIBPIN, 255);
    else if((millis() - timer) < 7000)
      analogWrite(VIBPIN, 0);
    else if((millis() - timer) < 10000)
      analogWrite(VIBPIN, 255);
    else if((millis() - timer) < 10500)
      analogWrite(VIBPIN, 0);
    else if((millis() - timer) < 13500)
      analogWrite(VIBPIN, 255);
    else{
      analogWrite(VIBPIN, 0);
      vibrate = false;
    }
  }
  else {
    analogWrite(VIBPIN, 0);
  }
}

void startLed(){
  strip.clear();
  for(int i = 0; i < ceil(map(intensity, 0, 3, 1, LEDN)); i++){
   strip.setPixelColor(i, colorList[intensity]);
  }
  strip.show();
  return;
}

void stopLed(){
  strip.clear();
  strip.show();
  return;
}