//
// Created by ronger on 2023/8/29.
//

#ifndef ESP32PWMCONTROL_H
#define ESP32PWMCONTROL_H

#include "Arduino.h"

#define PWM_FREQ 1000    // PWM Frequency

class ESP32PWMControl {
public:
    uint16_t mMotorSpeed[6] = {0, 0, 0, 0, 0, 0};
    boolean mMotorForward[6] = {true, true, true, true, true, true};

    void attachMotor(uint8_t gpioIn1, uint8_t gpioIn2);
    void attachMotorInit(uint8_t gpioIn1, uint8_t gpioIn2, uint8_t pwm, uint8_t ioSignal);

    void motorFullForward(uint8_t pwm);
    void motorForward(uint8_t pwm, uint8_t timer, uint8_t speed);
    void motorFullReverse(uint8_t pwm);
    void motorReverse(uint8_t pwm, uint8_t timer, uint8_t speed);
    void motorStop(uint8_t pwm, uint8_t timer);
    void motorsStop();
private:

    // Fields:

    boolean mMotorAttached[6] = {false, false, false, false, false, false};

    // Methods

    boolean isMotorValid(uint8_t pwm, uint8_t timer);
};


#endif //FIICAR_ESP32PWMCONTROL_H
