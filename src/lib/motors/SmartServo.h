/*
  smartServoSS485.h - Library for controlling the smart servo.
  Created by Paolo Cavagnolo, may 2021 - paolo.cavagnolo@gmail.com.
  Released into the public domain.
*/

#ifndef SmartServoClass_h
#define SmartServoClass_h

#include "Arduino.h"
#include "ArduinoRS485.h"
#include "rtos.h"

class SmartServoClass
{

  public:
  	SmartServoClass(RS485Class& serial);

    void begin();
    void setID(byte id);
    bool ping(byte id);
    void setLimitAngle(byte id, uint16_t min, uint16_t max);
    void setTorque(byte id, uint16_t torque);
    void hold(byte id, bool state);

    void goTo(byte id, uint16_t angle, uint16_t tempo);

    int regRead(byte id, char name);
    void regWrite(byte id, char name, uint16_t value);

    int _tx(byte id, byte len, byte cmd, byte *prms);
    void _rx(int len);

  private:
    byte _r[400];

    RS485Class* _serial;
    rtos::Mutex mutex;
};

#endif