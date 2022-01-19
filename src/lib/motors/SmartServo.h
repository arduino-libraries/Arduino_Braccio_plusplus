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

  int begin();
  void end();

  inline void setPositionMode(PositionMode const mode) { _positionMode = mode; }

  void setPosition(uint8_t const id, float const angle, uint16_t const speed);

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

  void setTime(uint8_t const id, uint16_t const time);

  void center(uint8_t const id, uint16_t const position);

  int ping(uint8_t const id);

  //void reset(uint8_t const id = BROADCAST);

  bool isEngaged(uint8_t const id);

  void printTimestamps();

  void getInfo(Stream & stream, uint8_t const id);

  inline void onErrorCb(mbed::Callback<void()> onError) { _onError = onError; }

  inline int getErrors() const { return _errors; }

  static const int BROADCAST = 0xFE;
  static float constexpr MAX_ANGLE = 315.0f;

private:

  static int constexpr NUM_MOTORS = 6;
  static int constexpr MAX_TX_PAYLOAD_LEN = (5*NUM_MOTORS+4);
  static int constexpr MAX_RX_PAYLOAD_LEN = 10;
  static int constexpr MAX_POSITION = 4000;

  static int constexpr MIN_MOTOR_ID = 1;
  static int constexpr MAX_MOTOR_ID = 6;

  inline bool isValidAngle(float const angle) { return ((angle >= 0.0f) && (angle <= MAX_ANGLE)); }
  inline bool isValidId(int const id) const { return ((id >= MIN_MOTOR_ID) && (id <= MAX_MOTOR_ID)); }
  inline int  idToArrayIndex(int const id) const { return (id - 1); }

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
  void     writeSyncCmd    (uint8_t *id, uint8_t const num, uint8_t const address, uint8_t const len, uint8_t const * data);

  inline uint16_t angleToPosition(float const angle) { return (angle*MAX_POSITION)/MAX_ANGLE; }
  inline float    positionToAngle(uint16_t const position) { return (MAX_ANGLE*position)/MAX_POSITION; }


  RS485Class& _RS485;
  int _errors;
  mbed::Callback<void()> _onError;


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

  rtos::Mutex mutex;
};

#endif // _SMARTMOTOR_H_
