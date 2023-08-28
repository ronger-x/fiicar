#define PRO_BOAT_CONTROL
// #define PRO_SERVO_CONTROL
#include <Arduino.h>
#include <PS2X_lib.h>

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

PS2X ps2x; // khởi tạo class PS2x

void runCar();

void stopCar();

void initCar();

void initPS2();


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
    }
    if (ps2x.Button(PSB_PAD_RIGHT)) {
        Serial.print("Right held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_RIGHT), DEC);
    }
    if (ps2x.Button(PSB_PAD_LEFT)) {
        Serial.print("LEFT held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_LEFT), DEC);
    }
    if (ps2x.Button(PSB_PAD_DOWN)) {
        Serial.print("DOWN held this hard: ");
        Serial.println(ps2x.Analog(PSAB_PAD_DOWN), DEC);
    }

    if (ps2x.NewButtonState()) { // Trả về giá trị TRUE khi nút được thay đổi trạng thái (bật sang tắt, or tắt sang bật)
        if (ps2x.Button(PSB_L3))
            Serial.println("L3 pressed");
        if (ps2x.Button(PSB_R3))
            Serial.println("R3 pressed");
        if (ps2x.Button(PSB_L2))
            Serial.println("L2 pressed");
        if (ps2x.Button(PSB_R2))
            Serial.println("R2 pressed");
        if (ps2x.Button(PSB_TRIANGLE))
            Serial.println("△ pressed");
    }
    //△□○×
    if (ps2x.ButtonPressed(PSB_CIRCLE)) // Trả về giá trị TRUE khi nút được ấn (từ tắt sang bật)
        Serial.println("○ just pressed");
    if (ps2x.NewButtonState(PSB_CROSS)) // Trả về giá trị TRUE khi nút được thay đổi trạng thái
        Serial.println("× just changed");
    if (ps2x.ButtonReleased(PSB_SQUARE)) //  Trả về giá trị TRUE khi nút được ấn (từ tắt sang bật)
        Serial.println("□ just released");

    if (ps2x.Button(PSB_L1) || ps2x.Button(PSB_R1)) // các trả về giá trị TRUE khi nút được giữ
    {                                               // Đọc giá trị 2 joystick khi nút L1 hoặc R1 được giữ
        Serial.print("Stick Values:");
        Serial.print(ps2x.Analog(PSS_LY)); // đọc trục Y của joystick bên trái. Other options: LX, RY, RX
        Serial.print(",");
        Serial.print(ps2x.Analog(PSS_LX), DEC);
        Serial.print(",");
        Serial.print(ps2x.Analog(PSS_RY), DEC);
        Serial.print(",");
        Serial.println(ps2x.Analog(PSS_RX), DEC);
    }
    delay(50);
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
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    pinMode(IN3_PIN, OUTPUT);
    pinMode(IN4_PIN, OUTPUT);
    pinMode(IN5_PIN, OUTPUT);
    pinMode(IN6_PIN, OUTPUT);
    pinMode(IN7_PIN, OUTPUT);
    pinMode(IN8_PIN, OUTPUT);
    stopCar();
}

void runCar() {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, HIGH);
    digitalWrite(IN5_PIN, LOW);
    digitalWrite(IN6_PIN, HIGH);
    digitalWrite(IN7_PIN, LOW);
    digitalWrite(IN8_PIN, HIGH);
}

void stopCar() {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, HIGH);
    digitalWrite(IN3_PIN, HIGH);
    digitalWrite(IN4_PIN, HIGH);
    digitalWrite(IN5_PIN, HIGH);
    digitalWrite(IN6_PIN, HIGH);
    digitalWrite(IN7_PIN, HIGH);
    digitalWrite(IN8_PIN, HIGH);
}