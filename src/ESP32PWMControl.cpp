//
// Created by ronger on 2023/8/29.
//
#include "Arduino.h"

#include "ESP32PWMControl.h"

#include <stdio.h>

#include "esp_system.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

//#define debug(fmt, ...)
#define debug(fmt, ...) Serial.printf("%s: " fmt "\r\n", __func__, ##__VA_ARGS__)

// Attach one motor

void ESP32PWMControl::attachMotor(uint8_t gpioIn1, uint8_t gpioIn2) {
    attachMotorInit(gpioIn1, gpioIn2, 0, 0);
}

void ESP32PWMControl::attachMotorInit(uint8_t gpioIn1, uint8_t gpioIn2, uint8_t pwm, uint8_t ioSignal) {
    debug("init MCPWM_UNIT_", pwm, " IO_SIGNAL", ioSignal);
    mcpwm_unit_t pwmNum = mcpwm_unit_t(pwm);
    mcpwm_io_signals_t ioSignalA = mcpwm_io_signals_t(0);
    mcpwm_io_signals_t ioSignalB = mcpwm_io_signals_t(1);
    if (ioSignal == 1) {
        ioSignalA = mcpwm_io_signals_t(2);
        ioSignalB = mcpwm_io_signals_t(3);
    } else if (ioSignal == 2) {
        ioSignalA = mcpwm_io_signals_t(4);
        ioSignalB = mcpwm_io_signals_t(5);
    }
    mcpwm_gpio_init(pwmNum, ioSignalA, gpioIn1);
    mcpwm_gpio_init(pwmNum, ioSignalB, gpioIn2);
    if (pwm == 0) {
        this->mMotorAttached[ioSignal] = true;
    } else {
        this->mMotorAttached[ioSignal + 3] = true;
    }

    // Initial MCPWM configuration

    debug ("Configuring Initial Parameters of MCPWM...");

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency,
    pwm_config.cmpr_a = 0;            //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;            //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config);    //Configure PWM0A & PWM0B with above settings

    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_2, &pwm_config);    //Configure PWM0A & PWM0B with above settings


    debug ("MCPWM initialized");
}

void ESP32PWMControl::motorFullForward(uint8_t pwm) {
    mcpwm_unit_t pwmNum = mcpwm_unit_t(pwm);
    for (int i = 0; i < 3; ++i) {
        if (!isMotorValid(pwm, i)) {
            continue;
        }
        mcpwm_timer_t timerNum = mcpwm_timer_t(i);
        mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_B);
        mcpwm_set_signal_high(pwmNum, timerNum, MCPWM_OPR_A);
    }
}

void ESP32PWMControl::motorForward(uint8_t pwm, uint8_t timer, uint8_t speed) {
    if (!isMotorValid(pwm, timer)) {
        return;
    }
    mcpwm_unit_t pwmNum = mcpwm_unit_t(pwm);
    mcpwm_timer_t timerNum = mcpwm_timer_t(timer);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_B);
    mcpwm_set_duty(pwmNum, timerNum, MCPWM_OPR_A, speed);
    // call this each time, if operator was previously in low/high state
    mcpwm_set_duty_type(pwmNum, timerNum, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    uint8_t motor = timer;
    if (pwm == 1) {
        motor += 3;
    }
    mMotorSpeed[motor] = speed; // Save it
    mMotorForward[motor] = true;

    debug("Motor %u forward speed %u", motor, speed);
}

void ESP32PWMControl::motorFullReverse(uint8_t pwm) {
    mcpwm_unit_t pwmNum = mcpwm_unit_t(pwm);
    for (int i = 0; i < 3; ++i) {
        if (!isMotorValid(pwm, i)) {
            continue;
        }
        mcpwm_timer_t timerNum = mcpwm_timer_t(i);
        mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_A);
        mcpwm_set_signal_high(pwmNum, timerNum, MCPWM_OPR_B);
    }
}

void ESP32PWMControl::motorReverse(uint8_t pwm, uint8_t timer, uint8_t speed) {
    if (!isMotorValid(pwm, timer)) {
        return;
    }
    mcpwm_unit_t pwmNum = mcpwm_unit_t(pwm);
    mcpwm_timer_t timerNum = mcpwm_timer_t(timer);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_A);
    mcpwm_set_duty(pwmNum, timerNum, MCPWM_OPR_B, speed);
    // call this each time, if operator was previously in low/high state
    mcpwm_set_duty_type(pwmNum, timerNum, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
    uint8_t motor = timer;
    if (pwm == 1) {
        motor += 3;
    }
    mMotorSpeed[motor] = speed; // Save it
    mMotorForward[motor] = false;

    debug("Motor %u forward speed %u", motor, speed);
}

void ESP32PWMControl::motorStop(uint8_t pwm, uint8_t timer) {
    if (!isMotorValid(pwm, timer)) {
        return;
    }
    mcpwm_unit_t pwmNum = mcpwm_unit_t(pwm);
    mcpwm_timer_t timerNum = mcpwm_timer_t(timer);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_A);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_B);
}

void ESP32PWMControl::motorsStop() {
    for (int i = 0; i < 3; ++i) {
        motorStop(0, i);
        motorStop(1, i);
    }
    debug("Motors stop");
}

//// Privates

// Is motor valid ?

boolean ESP32PWMControl::isMotorValid(uint8_t pwm, uint8_t timer) {
    if (pwm > 1) {
        return false;
    }
    uint8_t motor = timer;
    if (pwm == 1) {
        motor += 3;
    }
    return mMotorAttached[motor];
}


///// End