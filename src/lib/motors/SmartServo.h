/*
 * Board support library for the Arduino Braccio++ 6-DOF robot arm.
 * Copyright (C) 2022 Arduino (http://www.arduino.cc/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA.
 */

#ifndef _SMARTMOTOR_H_
#define _SMARTMOTOR_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino.h>
#include <mbed.h>

#include "RS485.h"
#include "SmartServoConst.h"

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class SmartServoClass
{
public:
  
  SmartServoClass(RS485Class & RS485);

  void begin();
  void end();

  inline void setPositionMode(PositionMode const mode) { _positionMode = mode; }

  void setPosition(uint8_t const id, float const angle);

  float getPosition(uint8_t const id);

  void synchronize();

  void setTorque(bool const torque);

  void setTorque(uint8_t const id, bool const torque);

  void setMaxTorque(uint16_t const torque);

  void setMaxTorque(uint8_t const id, uint16_t const torque);

  void setID(uint8_t const id);

  void engage(uint8_t const id);

  void disengage(uint8_t const id);

  void setStallProtectionTime(uint8_t const time);

  void setStallProtectionTime(uint8_t const id, uint8_t const time);

  void setMinAngle(uint16_t const min_angle);
  void setMinAngle(uint8_t const id, uint16_t const min_angle);
  void setMaxAngle(uint16_t const max_angle);
  void setMaxAngle(uint8_t const id, uint16_t const max_angle);

  void setTime(uint16_t const time);
  void setTime(uint8_t const id, uint16_t const time);

  void center(uint8_t const id, uint16_t const position);

  int ping(uint8_t const id);

  //void reset(uint8_t const id = BROADCAST);

  bool isEngaged(uint8_t const id);

  void getInfo(Stream & stream, uint8_t const id);

  inline void onErrorCb(mbed::Callback<void()> onError) { _onError = onError; }

  inline int getErrors() const { return _errors; }

  static int   constexpr BROADCAST = 0xFE;
  static int   constexpr MIN_MOTOR_ID = 1;
  static int   constexpr MAX_MOTOR_ID = 6;
  static int   constexpr NUM_MOTORS = 6;
  static float constexpr MAX_ANGLE = 315.0f;

  static size_t idToArrayIndex(size_t const id) { return (id - 1); }

private:

  static int constexpr MAX_TX_PAYLOAD_LEN = (5*NUM_MOTORS+4);
  static int constexpr MAX_RX_PAYLOAD_LEN = 10;
  static int constexpr MAX_POSITION = 4000;

  inline bool isValidAngle(float const angle) { return ((angle >= 0.0f) && (angle <= MAX_ANGLE)); }
  inline bool isValidId(int const id) const { return ((id >= MIN_MOTOR_ID) && (id <= MAX_MOTOR_ID)) || (id == BROADCAST); }

  int      calcChecksum    ();
  void     sendPacket      ();
  void     writeCmd        (uint8_t const id, SmartServoOperation const instruction);
  void     writeByteCmd    (uint8_t const id, uint8_t const address, uint8_t const data);
  void     writeWordCmd    (uint8_t const id, uint8_t const address, uint16_t const data);
  void     receiveResponse (int const howMany = MAX_RX_PAYLOAD_LEN);
  int      readBuffer      (uint8_t const id, uint8_t const address, uint8_t const len);
  int      readWordCmd     (uint8_t const id, uint8_t const address);
  int      readByteCmd     (uint8_t const id, uint8_t const address);
  void     action          (uint8_t const id);

  inline uint16_t angleToPosition(float const angle) { return (angle*MAX_POSITION)/MAX_ANGLE; }
  inline float    positionToAngle(uint16_t const position) { return (MAX_ANGLE*position)/MAX_POSITION; }


  RS485Class& _RS485;
  int _errors;
  mbed::Callback<void()> _onError;
  rtos::Mutex _mtx;


  struct __attribute__((packed)) {
    uint8_t header [2];
    uint8_t id;
    uint8_t length;
    uint8_t instruction;
    uint8_t payload[MAX_TX_PAYLOAD_LEN];
  } _txPacket;

  uint8_t _rxBuf[MAX_RX_PAYLOAD_LEN];
  uint8_t _rxLen;
  uint16_t _targetPosition[NUM_MOTORS];
  uint16_t _targetSpeed[NUM_MOTORS];

  PositionMode _positionMode;
};

#endif // _SMARTMOTOR_H_
