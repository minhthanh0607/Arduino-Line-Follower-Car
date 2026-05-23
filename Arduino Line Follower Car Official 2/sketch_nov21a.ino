//motor 1: bên phải
//motor 2: bên trái
//nếu muốn xe đi thẳng thì trái > phải 2 đơn vị

#define MIN_SPEED -255
#define MAX_SPEED  255

//button
#define Red_Button 12
#define Yellow_Button 13
#define Green_Button A0
bool isRunning = false;
unsigned long start = 0;
unsigned long lastLoopTime = 0;
int sensor[5] = {A1, A2, A3, A4, A5};
int sensor0 = A1;
int sensor1 = A2;
int sensor2 = A3;
int sensor3 = A4;
int sensor4 = A5;

//sensor[0]: ngoài cùng bên phải
//sensor[1]: tiếp theo...

int trig = 3;
int echo = 2;

//motor 1
#define M1_PWM 6
#define M1_DIR 7

//motor 2
#define M2_PWM 5
#define M2_DIR 4

unsigned long time;
unsigned long previousTime =0;

int baseSpeedL = 255;
int baseSpeedR = 255;

//error value and PID
float error = 0.0;
float previous_error = 0.0;
float P; int Kp = 90;
float I; int Ki = 0;
float D; int Kd = 66.3;
float PID_value;

int leftSpeed;
int rightSpeed;

unsigned long timeObstacle = 0;

// ================== Hàm pattern đọc line 5 mắt ==================
int readLinePattern() {
  int pattern = 0;
  for (int i = 0; i < 5; i++) {
    pattern <<= 1;
    pattern |= digitalRead(sensor[i]);
  }
  return pattern;
}
int patternCount = 0;
// ============ PID ============
int weights[5] = {3.5, 1.25, 0, -1.25, -3.5};
int sensorValue[5];

void errorCalculation() {
  int weightedSum = 0;
  int activeCount = 0;
  int pattern = readLinePattern();
  for (int i = 0; i < 5; i++) {
    sensorValue[i] = digitalRead(sensor[i]);
    // if(pattern == 0b10001){
    //   sensorValue[0] = 0;
    //   sensorValue[4] = 1;
    // }
    if (sensorValue[i] == 1) {
      weightedSum += weights[i];
      activeCount++;
    }
  }
  if (activeCount != 0)
    error = (float)weightedSum / (float)activeCount; // trung bình vị trí line
  else
    error = previous_error; // nếu không thấy line thì giữ hướng cũ
  if (pattern  == 0b10101 || pattern == 0b10100 || pattern ==0b10000){
    error = previous_error;
  }
  if (pattern == 0b11111){
    error = 0;
    patternCount++;
  }
  if(pattern == 0b10001 || pattern == 0b10011){
    error = -3.5;
  }
  Serial.print("Sensors: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(sensorValue[i]);
    Serial.print(" ");
  }
  Serial.print(" | Error: ");
  Serial.println(error);
}

void pid_calculator() {
  P = error;
  I = I + error;
  D = error - previous_error;
  PID_value = Kp * P + Ki * I + Kd * D;
  previous_error = error;
}

void motor_stop(){
  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
}
void speedControl() {
  float speedFactor = 1.0;

  //Giảm tốc khi vào cua
  // if (abs(error) >= 3) speedFactor = 0.25;   // cua gắt
  // else if (abs(error) >= 2) speedFactor = 0.7; // cua vừa // 0.75
  // else if (abs(error) >= 1) speedFactor = 0.72;  // cua nhẹ
  // else speedFactor = 1.0;                     // đi thẳng
  leftSpeed = (baseSpeedL + PID_value) * speedFactor;
  rightSpeed = (baseSpeedR - PID_value) * speedFactor;


  leftSpeed = constrain(leftSpeed, MIN_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, MIN_SPEED, MAX_SPEED);

  control_left();
  control_right();
}

void control_left() {
  if (leftSpeed > 0) {
    digitalWrite(M1_DIR, LOW);
    analogWrite(M1_PWM, leftSpeed);
  } else {
    digitalWrite(M1_DIR, HIGH);
    analogWrite(M1_PWM, abs(leftSpeed));
  }
}

void control_right() {
  if (rightSpeed > 0) {
    digitalWrite(M2_DIR, LOW);
    analogWrite(M2_PWM, rightSpeed);
  } else {
    digitalWrite(M2_DIR, HIGH);
    analogWrite(M2_PWM, abs(rightSpeed));
  }
}

//============ SETUP ============
void setup() {
  //cảm biến dò line
  for (int i = 0; i < 5; i++) {
    pinMode(sensor[i], INPUT);
  }

  //motor
  pinMode(M1_PWM, OUTPUT);
  pinMode(M1_DIR, OUTPUT);
  pinMode(M2_PWM, OUTPUT);
  pinMode(M2_DIR, OUTPUT);

  //cảm biến siêu âm
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  pinMode(Red_Button, INPUT);
  pinMode(Yellow_Button, INPUT);

  Serial.begin(9600);
}
//============ ULTRASONIC ============
bool ultraEnabled = true; 
float ultrasonicM() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  float time = pulseIn(echo, HIGH);
  float distance = 0.034 * time / 2; //đơn vị theo cm
  return distance;
}
unsigned long lastUltrasonicM = 0;
bool detectObstacle() {
  if (!ultraEnabled) return false;   // << TẮT SIÊU ÂM HOÀN TOÀN >>
  static unsigned long lastUltrasonicM = 0;
  static float distanceM = 999;
  if (millis() - lastUltrasonicM > 120) {
    lastUltrasonicM = millis();
    distanceM = ultrasonicM();
  }
  return (distanceM < 18);
}
enum AvoidState { NORMAL, TURN_RIGHT, GO_STRAIGHT, TURN_LEFT, GO_STRAIGHT2 };
AvoidState avoidState = NORMAL;

unsigned long avoidStart = 0;
void avoidObstacle() {
  switch (avoidState) {
    case NORMAL:
      // Không làm gì trong hàm này, xử lý ở loop
      break;
    case TURN_RIGHT:
      // Quẹo phải rời line
      digitalWrite(M1_DIR, HIGH);   // motor trái chạy lùi
      analogWrite(M1_PWM, 80);
      digitalWrite(M2_DIR, LOW);    // motor phải chạy tới
      analogWrite(M2_PWM, 100);
      if (millis() - avoidStart >= 150) {   // thời gian quẹo tùy xe
        avoidState = GO_STRAIGHT;
        avoidStart = millis();
      }
      break;
    case GO_STRAIGHT:
      // Chạy thẳng né vật cản
      digitalWrite(M1_DIR, LOW);
      digitalWrite(M2_DIR, LOW);
      analogWrite(M1_PWM, 80);
      analogWrite(M2_PWM, 80);
      if (millis() - avoidStart >= 700) {   // chạy thẳng 0.5s
        avoidState = TURN_LEFT;
        avoidStart = millis();
      }
      break;
    case TURN_LEFT:
      // Quẹo trái để quay lại lines
      digitalWrite(M1_DIR, LOW);   // trái chạy tới
      analogWrite(M1_PWM, 100);
      digitalWrite(M2_DIR, HIGH);  // phải chạy lùi
      analogWrite(M2_PWM, 80);
      if (millis() - avoidStart >= 250) {   // chạy thẳng 0.5s
        avoidState = GO_STRAIGHT2;
        avoidStart = millis();
      }
      break;
    case GO_STRAIGHT2:
      digitalWrite(M1_DIR, LOW);
      digitalWrite(M2_DIR, LOW);
      analogWrite(M1_PWM, 80);
      analogWrite(M2_PWM, 80);
      // Khi sensor giữa thấy line thì về NORMAL
      if (digitalRead(sensor2) == 1) {
        ultraEnabled = true;
        avoidState = NORMAL;
      }
  }
}
bool lastObstacle = false;
int sonicCount = 0;
void resetAllStates() {
  avoidState = NORMAL;
  sonicCount = 0;
  ultraEnabled = true;
  error = previous_error = 0;
}
// ============ BUTTON ============
void checkStartButton() {
  if (digitalRead(Red_Button) == LOW) {
    if (!isRunning) {
      Serial.println("Bat dau chay!");
      timeObstacle = millis();   // bắt đầu chạy
      resetAllStates();
      baseSpeedL = 255;          // tốc độ ban đầu
      baseSpeedR = 255;
      Kp = 90;
      Kd = 66.3;
      isRunning = true;
      patternCount = 0;
    } else {
      Serial.println("Reset van toc ban dau!");
      timeObstacle = millis();   // reset lại timeObstacle
      baseSpeedL = 255;          // đặt lại tốc độ ban đầu
      baseSpeedR = 255;
      Kp = 90;                   // đặt lại PID ban đầu
      Kd = 66.3;
      patternCount = 0;
    }
    delay(300);
  }
}
void checkStopButton() {
  if (digitalRead(Yellow_Button) == LOW && isRunning){
    Serial.println("Dung xe!");
    isRunning = false;
    motor_stop();
    delay(300);
  }
}
void changeMode() {
  if (digitalRead(Green_Button) == LOW) {
    if (!isRunning) {
      Serial.println("Bat dau chay che do 2!");
      timeObstacle = millis();   // bắt đầu chạy
      resetAllStates();
      baseSpeedL = 80;          // tốc độ ban đầu
      baseSpeedR = 80;
      Kp = 90;
      Kd = 66.3;
      isRunning = false;
      patternCount = 0;
    delay(300);
    }
  }
}
void loop() {
  checkStartButton();
  checkStopButton();
  if (!isRunning) return;
  bool detectObstacleM = detectObstacle();
  // ====== 1) Đếm số lần gặp vật cản theo cách an toàn ======
  if (detectObstacleM && !lastObstacle) {
    // chỉ tăng khi vừa xuất hiện
    sonicCount++;
    lastObstacle = true;
  }
  if (!detectObstacleM) {
    lastObstacle = false;
  }

  // ====== LẦN 2 TRỞ ĐI: dừng vĩnh viễn ======
  if (sonicCount >= 2) {
    analogWrite(M1_PWM, 0);
    analogWrite(M2_PWM, 0);
    return;
  }
  if (avoidState == NORMAL && detectObstacleM && sonicCount == 1) {
    ultraEnabled = false;
    lastObstacle = false;
    sonicCount = 1;
    lastUltrasonicM = millis();
    // dừng + lùi nhẹ
    analogWrite(M1_PWM, 0);
    analogWrite(M2_PWM, 0);
    delay(200);

    digitalWrite(M1_DIR, HIGH);
    digitalWrite(M2_DIR, HIGH);
    analogWrite(M1_PWM, 80);
    analogWrite(M2_PWM, 80);
    delay(350);
    avoidState = TURN_RIGHT;
    avoidStart = millis();
    return;
  }

  // ====== ĐANG NÉ VẬT CẢN  ======
  if (avoidState != NORMAL) {
    avoidObstacle();
    // Khi QUẸO TRÁI mà thấy lại line → bật PID trở lại
    if (avoidState == NORMAL) {
      // KHÔNG tăng sonicCount tại đây
      // Trở lại chạy PID
    }
    return;
  }
  // ====== CHẾ ĐỘ PID BÌNH THƯỜNG ======
  if (millis() - timeObstacle > 7000) {
    baseSpeedL = 80;
    baseSpeedR = 80;
    Kp = 65;
    Kd = 80;
  }
  errorCalculation();
  pid_calculator();
  speedControl();
  delay(30);
}
//============ MAZE SOLVING ============
void mazeCheck(){
  if(patternCount >= 7) return true;
  return false;
}
bool isTurningBack = false;
unsigned long turnTime = 0;
void mazeSolve(){
  baseSpeedL = 80;
  baseSpeedR = 80;
  Kp = 65;
  Kd = 80;
  if(pattern == 0b11111){
    error = -1.33;
  }
  if(pattern == 0b00000){
    //quay đầu
    if(!isTurningBack){
      isTurningBack = true;
      turnTime = millis();
    }
    digitalWrite(M1_DIR, LOW);
    digitalWrite(M2_DIR, HIGH);
    analogWrite(M1_PWM, 80);
    analogWrite(M2_PWM, 80);
    if(millis() - turnTime <= 200) return;
    isTurningBack = false;
    
    return;
  }
  errorCalculation();
  pid_calculator();
  speedControl();
  delay(30);
}
