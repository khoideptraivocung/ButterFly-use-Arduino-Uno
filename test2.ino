#include <Servo.h>

Servo servo1;
Servo servo2;

// ==== Chân RX ====
const int ch1Pin = 2; // trái/phải
const int ch2Pin = 3; // lên/xuống
const int ch3Pin = 4; // ga

volatile unsigned long ch1Start = 0;
volatile unsigned long ch2Start = 0;

int ch1Value = 1500;
int ch2Value = 1500;
int ch3Value = 1000;

int servo1Pos = 90;
int servo2Pos = 90;

int dir1 = 1;
int dir2 = -1;

unsigned long lastUpdate = 0;

// ==== Tham số dễ chỉnh ====
// Bước vỗ cánh
int flapStepLow = 6;  // ga thấp
int flapStepHigh = 5; // ga cao

// Thời gian giữa các bước (ms)
int speedDelayLow = 20; // ga thấp → vừa phải
int speedDelayHigh = 2; // ga cao → cực nhanh

// Hành trình flap
int flapLowMin = 30;   // ga thấp → góc thấp nhất
int flapLowMax = 50;   // ga cao → góc thấp nhất khi hẹp
int flapHighMin = 150; // ga thấp → góc cao nhất
int flapHighMax = 130; // ga cao → góc cao nhất khi hẹp

// Hành trình CH1/CH2 (trái/phải, lên/xuống)
int steerRange = 35;

void setup() {
  servo1.attach(9);
  servo2.attach(10);

  pinMode(ch1Pin, INPUT);
  pinMode(ch2Pin, INPUT);
  pinMode(ch3Pin, INPUT);

  attachInterrupt(digitalPinToInterrupt(ch1Pin), ch1Pulse, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ch2Pin), ch2Pulse, CHANGE);

  servo1.write(servo1Pos);
  servo2.write(servo2Pos);
}

void ch1Pulse() {
  if (digitalRead(ch1Pin)) ch1Start = micros();
  else ch1Value = constrain(micros() - ch1Start, 1000, 2000);
}

void ch2Pulse() {
  if (digitalRead(ch2Pin)) ch2Start = micros();
  else ch2Value = constrain(micros() - ch2Start, 1000, 2000);
}

void loop() {
  unsigned long now = millis();

  int pulse = pulseIn(ch3Pin, HIGH, 25000);
  if (pulse > 0) ch3Value = constrain(pulse, 1000, 2000);

  // ==== Tính tốc độ và step flap theo ga ====
  int speedDelay = map(ch3Value, 1000, 2000, speedDelayLow, speedDelayHigh);
  int flapStep = map(ch3Value, 1000, 2000, flapStepLow, flapStepHigh);

  // ==== Tính hành trình flap theo ga ====
  int flapLow = map(ch3Value, 1000, 2000, flapLowMin, flapLowMax);     // thấp nhất
  int flapHigh = map(ch3Value, 1000, 2000, flapHighMin, flapHighMax); // cao nhất
  
  if (ch3Value > 1800) {
    speedDelay = speedDelayHigh;
  }
  
  if (ch3Value > 1800) {
    flapLow  -= 8;   // mở xuống thêm
    flapHigh += 8;   // mở lên thêm
  }
  
  flapLow  = constrain(flapLow,  10, 170);
  flapHigh = constrain(flapHigh, 10, 170);

  
  // ==== Vỗ cánh khi ga > 1050 ====
  if (ch3Value > 1050) {
    if (now - lastUpdate >= speedDelay) {
      servo1Pos += dir1 * flapStep;
      servo2Pos += dir2 * flapStep;

      if (servo1Pos >= flapHigh || servo1Pos <= flapLow) dir1 *= -1;
      if (servo2Pos >= flapHigh || servo2Pos <= flapLow) dir2 *= -1;

      lastUpdate = now;
    }
  } else {
    // Ga = 0 → dừng trung tâm
    servo1Pos = 90;
    servo2Pos = 90;
  }

  // ==== Điều hướng CH1/CH2 ====
  int ch1Adj = map(ch1Value, 1000, 2000, -steerRange, steerRange);
  int ch2Adj = map(ch2Value, 1000, 2000, -steerRange, steerRange);

  servo1.write(servo1Pos + ch2Adj);
  servo2.write(servo2Pos + ch1Adj);
}
