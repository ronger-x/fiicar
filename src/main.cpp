#define PRO_BOAT_CONTROL
// #define PRO_SERVO_CONTROL
#include <Arduino.h>
#include <PS2X_lib.h>
#include <ESP32PWMControl.h>

#define SERVICE_UUID "76ad7aaa-3782-11ed-a261-0242ac120002"

#define IN1_PIN 4
#define IN2_PIN 5
#define IN3_PIN 6
#define IN4_PIN 7
#define IN5_PIN 11
#define IN6_PIN 12
#define IN7_PIN 13
#define IN8_PIN 14

#define PS2_DAT 1 // MISO
#define PS2_CMD 2 // MOSI
#define PS2_SEL 42 // SS/CS
#define PS2_CLK 41 // SLK

#define pressures false
#define rumble false

#define fullSpeed 100
#define turnSpeed 50

// 测试出自己手柄空置时的值
#define  rockerPointX 128
#define  rockerPointY 127

ESP32PWMControl leftFrontPWMControl = ESP32PWMControl();
ESP32PWMControl rightFrontPWMControl = ESP32PWMControl();
ESP32PWMControl leftRearPWMControl = ESP32PWMControl();
ESP32PWMControl rightRearPWMControl = ESP32PWMControl();
PS2X ps2x; // khởi tạo class PS2x

void runCar();

void turnLeft();

void turnRight();

void reverseCar();

void stopCar();

void initCar();

void initPS2();

void runCar(uint8_t x, uint8_t y);

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        // wait for serial port to connect.
        // You don't have to do this in your game. This is only for debugging
        // purposes, so that you can see the output in the serial console.
        ;
    }
    initCar();
    initPS2();
}

// Arduino loop function. Runs in CPU 1
void loop() {
// check if a peripheral has been discovered
    ps2x.read_gamepad(false, false); // gọi hàm để đọc tay điều khiển
    bool run = false;
    // các trả về giá trị TRUE (1) khi nút được giữ
    if (ps2x.Button(PSB_START)) { // nếu nút Start được giữ, in ra Serial monitor
        Serial.println("Start is being held");
        stopCar();
    }
    if (ps2x.Button(PSB_PAD_UP)) // tương tự như trên kiểm tra nút Lên (PAD UP)
    {
        Serial.print("Up held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_UP),
                       DEC); // đọc giá trị analog ở nút này, xem nút này được bấm mạnh hay nhẹ
        runCar();
        run = true;
    }
    if (ps2x.Button(PSB_PAD_RIGHT)) {
        Serial.print("Right held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_RIGHT), DEC);
        turnRight();
        run = true;
    }
    if (ps2x.Button(PSB_PAD_LEFT)) {
        Serial.print("LEFT held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_LEFT), DEC);
        turnLeft();
        run = true;
    }
    if (ps2x.Button(PSB_PAD_DOWN)) {
        Serial.print("DOWN held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_DOWN), DEC);
        reverseCar();
        run = true;
    }
    if (!run) {

        Serial.println(ps2x.Analog(PSS_LX), DEC);
        Serial.println(ps2x.Analog(PSS_LY), DEC);
        uint8_t x = ps2x.Analog(PSS_LX);
        uint8_t y = ps2x.Analog(PSS_LY);
        // 防抖动
        if (abs(x - rockerPointX) < 5 && abs(y - rockerPointY) < 5) {
            stopCar();
        } else {
            runCar(x, y);
        }
    }
    delay(50);
}

void runCar(uint8_t x, uint8_t y) {
    // 0-255
    uint8_t absX = abs(x - rockerPointX) * 200 / 255;
    uint8_t absY = abs(y - rockerPointY) * 200 / 255;
    bool isReverse = false;
    if (y > rockerPointY) {
        isReverse = true;
    }
    if (x < rockerPointX) {
        // 左转
        if (absY < turnSpeed) {
            absY = turnSpeed;
        }
        if (isReverse) {
            leftFrontPWMControl.motorReverse(0, 0, turnSpeed);
            rightFrontPWMControl.motorReverse(0, 1, absY);
        } else {
            leftFrontPWMControl.motorReverse(0, 0, turnSpeed);
            rightFrontPWMControl.motorForward(0, 1, absY);
        }
    } else if (x > rockerPointX) {
        // 右转
        if (absY < turnSpeed) {
            absY = turnSpeed;
        }
        if (isReverse) {
            leftFrontPWMControl.motorReverse(0, 0, absY);
            rightFrontPWMControl.motorReverse(0, 1, turnSpeed);
        } else {
            leftFrontPWMControl.motorForward(0,0, absY);
            rightFrontPWMControl.motorReverse(0, 1, turnSpeed);
        }
    } else {
        // 直行
        if (isReverse) {
            leftFrontPWMControl.motorReverse(0, 0, absY);
            rightFrontPWMControl.motorReverse(0, 1, absY);
        } else {
            leftFrontPWMControl.motorForward(0, 0, absY);
            rightFrontPWMControl.motorForward(0, 1, absY);
        }
    }

    if (y < rockerPointY) {
        // 前进
        leftRearPWMControl.motorForward(1, 0, absY);
        rightRearPWMControl.motorForward(1, 1, absY);
    } else if (y > rockerPointY) {
        // 后退
        leftRearPWMControl.motorReverse(1,0, absY);
        rightRearPWMControl.motorReverse(1,1, absY);
    } else {
        // 前进
        if (absX < turnSpeed) {
            absX = turnSpeed;
        }
        leftRearPWMControl.motorForward(1,0, absX);
        rightRearPWMControl.motorForward(1,1, absX);
    }
}

void initPS2() {
    Serial.print("Ket noi voi tay cam PS2:");

    int error = -1;
    for (int i = 0; i < 10; i++) // thử kết nối với tay cầm ps2 trong 10 lần
    {
        delay(1000); // đợi 1 giây
        // cài đặt chân và các chế độ: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
        error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
        Serial.print(".");
    }

    switch (error) // kiểm tra lỗi nếu sau 10 lần không kết nối được
    {
        case 0:
            Serial.println(" Ket noi tay cam PS2 thanh cong");
            break;
        case 1:
            Serial.println(" LOI: Khong tim thay tay cam, hay kiem tra day ket noi vơi tay cam ");
            break;
        case 2:
            Serial.println(" LOI: khong gui duoc lenh");
            break;
        case 3:
            Serial.println(" LOI: Khong vao duoc Pressures mode ");
            break;
    }
}

void initCar() {
    // write your initialization code here
    leftFrontPWMControl.attachMotorInit(IN7_PIN, IN8_PIN, 0 , 0);
    rightFrontPWMControl.attachMotorInit(IN5_PIN, IN6_PIN, 0, 1);
    leftRearPWMControl.attachMotorInit(IN3_PIN, IN4_PIN, 1, 0);
    rightRearPWMControl.attachMotorInit(IN1_PIN, IN2_PIN, 1, 1);
    stopCar();
}

void runCar() {
    leftFrontPWMControl.motorForward(0, 0, fullSpeed);
    rightFrontPWMControl.motorForward(0, 1, fullSpeed);
    leftRearPWMControl.motorForward(1, 0, fullSpeed);
    rightRearPWMControl.motorForward(1, 1, fullSpeed);
}

void turnLeft() {
    leftFrontPWMControl.motorForward(0, 0, turnSpeed);
    rightFrontPWMControl.motorForward(0, 1, fullSpeed);
    leftRearPWMControl.motorForward(1, 0, fullSpeed);
    rightRearPWMControl.motorForward(1, 1, fullSpeed);
}

void turnRight() {
    leftFrontPWMControl.motorForward(0, 0, fullSpeed);
    rightFrontPWMControl.motorForward(0, 1, turnSpeed);
    leftRearPWMControl.motorForward(1, 0, fullSpeed);
    rightRearPWMControl.motorForward(1, 1, fullSpeed);
}

void reverseCar() {
    leftFrontPWMControl.motorReverse(0, 0, fullSpeed);
    rightFrontPWMControl.motorReverse(0, 1, fullSpeed);
    leftRearPWMControl.motorReverse(1, 0, fullSpeed);
    rightRearPWMControl.motorReverse(1, 1, fullSpeed);
}

void stopCar() {
    leftFrontPWMControl.motorStop(0, 0);
    rightFrontPWMControl.motorStop(0, 1);
    leftRearPWMControl.motorStop(1, 0);
    rightRearPWMControl.motorStop(1, 1);
}