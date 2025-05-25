#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define RING_PIN 7
#define RING_NUM_LEDS 16

#define TRIG_PIN 8
#define ECHO_PIN A0

#define BUZZER_PIN 4

#define SERVO_PIN1 5
#define SERVO_PIN2 6

#define BUTTON_PIN 3

#define PWM 2

Servo myServo1;
Servo myServo2;
Adafruit_NeoPixel ring(RING_NUM_LEDS, RING_PIN, NEO_GRB + NEO_KHZ800);

String currentColor = "";

unsigned long lastTriggerTime = 0;
const unsigned long cooldown = 200;
bool firstTrigger = true;

bool redState = false;
unsigned long redLastToggle = 0;
const unsigned long redBlinkInterval = 350;
unsigned long redStartTime = 0;

// 按钮闪烁控制变量
bool buttonActive = false;
unsigned long buttonLastToggle = 0;
const unsigned long buttonBlinkInterval = 300;
bool buttonLedOn = false;
int buttonBlinkCount = 0;
const int buttonBlinkMax = 4; // 闪烁次数（开关共10次，即5次闪烁）

// 上次按钮状态，用于检测按下事件
bool lastButtonState = HIGH;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  ring.begin();
  ring.show();

  pinMode(PWM, OUTPUT);
  analogWrite(PWM, 255);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  myServo1.attach(SERVO_PIN1);
  myServo2.attach(SERVO_PIN2);
  myServo1.write(0);
  myServo2.write(0);

  Serial.println("System ready. Type 'detected' to simulate card detection.");
}

void loop() {
  unsigned long currentTime = millis();

  // 串口输入处理
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("detected")) {
      if (firstTrigger || (currentTime - lastTriggerTime > cooldown)) {
        firstTrigger = false;
        lastTriggerTime = currentTime;

        Serial.println("Card detected!");

        lightRing(0, 255, 0);
        doServo();
        playToneSequence();
        delay(500);
        lightRing(0, 0, 0);
      }
    } else if (input.equalsIgnoreCase("green")) {
      currentColor = "green";
      lightRing(255, 110, 200);
      playToneSequence();
      redState = false;
    } else if (input.equalsIgnoreCase("red")) {
      currentColor = "red";
      lightRing(255, 0, 0);
      redState = true;
      redStartTime = currentTime;
    } else if (input.equalsIgnoreCase("pat")) {
      if (redState) {
        redState = false;
        noTone(BUZZER_PIN);
        doServo();
      }
    }

    //propagate to container
    else if (input.equalsIgnoreCase("on")){ //fridge open
      Serial1.print("on");
    }
    else if (input.equalsIgnoreCase("stop")){
      Serial1.print("stop");
    }
    else if (input.equalsIgnoreCase("0")){
      Serial1.print("0");
    }
    else if (input.equalsIgnoreCase("1")){
      Serial1.print("1");
    }
    else if (input.equalsIgnoreCase("2")){
      Serial1.print("2");
    }
    else if (input.equalsIgnoreCase("3")){
      Serial1.print("3");
    }
    else if (input.equalsIgnoreCase("4")){
      Serial1.print("4");
    }
  }

  if (Serial1.available() > 0){
    String input = Serial1.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("lid_open")){
      //do something when the lid is opened
    }
    else if (input.equalsIgnoreCase("lid_closed")){
      //do something when the lid is closed
    }
  }

  float distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 200) {
    lightRing(0, 0, 0);
    noTone(BUZZER_PIN);
    redState = false;
  }

  // 红灯报警状态处理
  if (redState) {
    if (currentTime - redStartTime > 300000) {
      redState = false;
      noTone(BUZZER_PIN);
    } else if (currentTime - redLastToggle >= redBlinkInterval) {
      redLastToggle = currentTime;
      static bool buzzerOn = false;
      buzzerOn = !buzzerOn;
      if (buzzerOn) {
        tone(BUZZER_PIN, 100);
      } else {
        noTone(BUZZER_PIN);
      }
    }
  }

  // 按钮按下检测（下降沿触发）
  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    // 按钮刚按下，启动闪烁
    buttonActive = true;
    buttonBlinkCount = 0;
    buttonLedOn = false;  // 从关闭开始
    buttonLastToggle = currentTime - buttonBlinkInterval; // 让闪烁立即开始
  }
  lastButtonState = currentButtonState;

  // 按钮闪烁逻辑
  if (buttonActive) {
    if (currentTime - buttonLastToggle >= buttonBlinkInterval) {
      buttonLastToggle = currentTime;
      buttonLedOn = !buttonLedOn;

      if (buttonLedOn) {
        lightRing(0, 0, 255);    // 蓝色点亮
        tone(BUZZER_PIN, 500);   // 500Hz 蜂鸣音
      } else {
        lightRing(0, 0, 0);      // 灯灭
        noTone(BUZZER_PIN);
      }

      buttonBlinkCount++;
      if (buttonBlinkCount >= buttonBlinkMax) {
        // 完成闪烁，关闭效果
        buttonActive = false;
        lightRing(0, 0, 0);
        noTone(BUZZER_PIN);
      }
    }
  }
}

void lightRing(int r, int g, int b) {
  for (int i = 0; i < RING_NUM_LEDS; i++) {
    ring.setPixelColor(i, ring.Color(r, g, b));
  }
  ring.show();
}

void doServo() {
  myServo1.write(45);
  myServo2.write(45);
  delay(500);
  myServo1.write(0);
  myServo2.write(0);
  delay(500);
}

void playToneSequence() {
  tone(BUZZER_PIN, 1000);
  delay(150);
  noTone(BUZZER_PIN);
  delay(50);
  tone(BUZZER_PIN, 1500);
  delay(150);
  noTone(BUZZER_PIN);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}
