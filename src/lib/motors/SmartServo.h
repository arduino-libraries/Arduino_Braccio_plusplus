#ifndef _SMARTMOTOR_H_
#define _SMARTMOTOR_H_

#include <Arduino.h>
#include <mbed.h>
#include "RS485.h"

enum class PositionMode
{
  IMMEDIATE,
  SYNC
};

static int constexpr MAX_MOTORS = 6;

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

  void setMinAngle(float const angle);

  void setMinAngle(uint8_t const id, float const angle);

  void setMaxAngle(float const angle);

  void setMaxAngle(uint8_t const id, float const angle);

  void setTime(uint8_t const id, uint16_t const time);

  void center(uint8_t const id, uint16_t const position);

  int ping(uint8_t const id);

  //void reset(uint8_t const id = BROADCAST);

  bool isEngaged(uint8_t const id);

  void printTimestamps();

  void getInfo(Stream & stream, uint8_t const id);

  inline void onErrorCb(mbed::Callback<void()> _onError) { onError = _onError; }

  inline int getErrors() const { return errors; }

  static const int BROADCAST = 0xFE;

private:

  static const int MAX_TX_PAYLOAD_LEN = (5*MAX_MOTORS+4);
  static const int MAX_RX_PAYLOAD_LEN = 10;
  static const int OP_PING = 1;
  static const int OP_READ = 2;
  static const int OP_WRITE = 3;
  static const int OP_REG_WRITE = 4;
  static const int OP_ACTION = 5;
  static const int OP_RESET = 6;
  static const int OP_SYNC_WRITE = 0x83;

  static const int MAX_POSITION = 4000;

  int timestamps[6 * 6] = {0};

  int      calcChecksum    ();
  void     sendPacket      ();
  void     writeCmd        (uint8_t const id, uint8_t const instruction);
  void     writeByteCmd    (uint8_t const id, uint8_t const address, uint8_t const data);
  void     writeWordCmd    (uint8_t const id, uint8_t const address, uint16_t const data);
  void     receiveResponse (int const howMany = MAX_RX_PAYLOAD_LEN);
  int      readBuffer      (uint8_t const id, uint8_t const address, uint8_t const len);
  int      readWordCmd     (uint8_t const id, uint8_t const address);
  int      readByteCmd     (uint8_t const id, uint8_t const address);
  void     action          (uint8_t const id);
  void     writeSyncCmd    (uint8_t *id, uint8_t const num, uint8_t const address, uint8_t const len, uint8_t const * data);

  inline uint16_t angleToPosition(float const angle) { return (angle*MAX_POSITION)/360.0; }
  inline float    positionToAngle(uint16_t const position) { return (360.0*position)/MAX_POSITION; }

  mbed::Callback<void()> onError;

  int errors = 0;

  RS485Class& _RS485;

  struct {
    uint8_t header [2];
    uint8_t id;
    uint8_t length;
    uint8_t instruction;
    uint8_t payload[MAX_TX_PAYLOAD_LEN];
  } _txPacket;

  uint8_t _rxBuf[MAX_RX_PAYLOAD_LEN];
  uint8_t _rxLen;
  uint16_t _targetPosition[MAX_MOTORS];
  uint16_t _targetSpeed[MAX_MOTORS];
  PositionMode _positionMode;

  rtos::Mutex mutex;
};

#endif // _SMARTMOTOR_H_