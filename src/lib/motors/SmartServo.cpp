/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "SmartServo.h"

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

SmartServoClass::SmartServoClass(RS485Class & RS485)
: _RS485{RS485}
, _errors{0}
, _onError{}
{

}

/**************************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************************************/

int SmartServoClass::calcChecksum() {
  char csum =0xff-_txPacket.id-_txPacket.length-_txPacket.instruction;
  int i=0;
  for (i=0;(i<(_txPacket.length-2))&&(i<(MAX_TX_PAYLOAD_LEN-1));i++) {
    csum -= _txPacket.payload[i];
  }
  _txPacket.payload[i]=csum;
  return i+6;
}

void SmartServoClass::sendPacket()
{
  char *buffer = (char *) &_txPacket;
  int len = calcChecksum();
  _RS485.beginTransmission();
  for (int i=0;i<len;i++) {
    _RS485.write(*buffer++);
  }
  _RS485.endTransmission();
  // should now receive an echo of what we just sent
  while (_RS485.available() < len) {
    delay(100);
  }
  // discard the echo
  int i = len;
  while (i-- > 0) {
    _RS485.read();
  }
}

void SmartServoClass::writeCmd(uint8_t const id, SmartServoOperation const instruction) {
  _txPacket.id = id;
  _txPacket.length = 2;
  _txPacket.instruction = toVal(instruction);
  sendPacket();
}

void SmartServoClass::writeByteCmd(uint8_t const id, uint8_t const address, uint8_t const data) {
  _txPacket.id = id;
  _txPacket.length = 2+2;
  _txPacket.instruction = toVal(SmartServoOperation::WRITE);
  _txPacket.payload[0]=address;
  _txPacket.payload[1]=data;
  sendPacket();
}

void SmartServoClass::writeWordCmd(uint8_t const id, uint8_t const address, uint16_t const data) {
  _txPacket.id = id;
  _txPacket.length = 2+3;
  _txPacket.instruction = toVal(SmartServoOperation::WRITE);
  _txPacket.payload[0]=address;
  _txPacket.payload[1]=data>>8;
  _txPacket.payload[2]=data;
  sendPacket();
}

void SmartServoClass::receiveResponse(int const howMany) {
  _rxLen=0;
  memset(_rxBuf, 0, sizeof(_rxBuf));
  long start = millis();
  while ((millis() - start < howMany) && (_rxLen < howMany)) {  // timeout_min = 70uS * howMany
    if (_RS485.available()) {
      uint8_t c = _RS485.read();
      //Serial.print(c, HEX);
      //Serial.print(" ");
      _rxBuf[_rxLen++]=c;
    }
  }
}

int SmartServoClass::readBuffer(uint8_t const id, uint8_t const address,uint8_t const len) {
  _txPacket.id = id;
  _txPacket.length = 2+2;
  _txPacket.instruction = toVal(SmartServoOperation::READ);
  _txPacket.payload[0]=address;
  _txPacket.payload[1]=len;
  sendPacket();
  receiveResponse(6+len);
  if (_rxLen==(6+len) && 
    _rxBuf[0]==0xff &&
    _rxBuf[1]==0xf5 &&
    _rxBuf[2]==id &&
    _rxBuf[3]==len+2) {
    return 0;
  }
  _errors++;
  if (_onError) _onError();
  return -1;
}


int SmartServoClass::readWordCmd(uint8_t const id, uint8_t const address) {
  if (readBuffer(id,address,2) == 0) {
    return (_rxBuf[5]<<8)|_rxBuf[6];
  }
  return -1;
}

int SmartServoClass::readByteCmd(uint8_t const id, uint8_t const address) {
  if (readBuffer(id,address,1) == 0) {
    return _rxBuf[5];
  }
  return -1;
}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

int SmartServoClass::ping(uint8_t const id) {
  mutex.lock();
  writeCmd(id, SmartServoOperation::PING);
  // TODO: check return
  receiveResponse(6);
  if (_rxLen==6 && 
    _rxBuf[0]==0xff &&
    _rxBuf[1]==0xf5 &&
    _rxBuf[2]==id &&
    _rxBuf[3]==2) {

  	mutex.unlock();
    return _rxBuf[4];
  }
  mutex.unlock();
  _errors++;
  if (_onError) _onError();
  return -1;
}

/*
// ATTENTION: RESET also changes the ID of the motor

void SmartServoClass::reset(uint8_t const id) {
  mutex.lock();
  writeCmd(id, SmartServoOperation::RESET);
  mutex.unlock();
}
*/

void SmartServoClass::action(uint8_t const id) {
  mutex.lock();
  writeCmd(id, SmartServoOperation::ACTION);
  mutex.unlock();
}

int SmartServoClass::begin() {
  if (_RS485) {
    _txPacket.header[0] = 0xff;
    _txPacket.header[1] = 0xff;
    _RS485.begin(115200, 0, 90);
    _RS485.receive();
    writeByteCmd(BROADCAST, toVal(SmartServoRegister::SERVO_MOTOR_MODE), 1);
    writeByteCmd(BROADCAST, toVal(SmartServoRegister::TORQUE_SWITCH) ,1);
    _positionMode = PositionMode::IMMEDIATE;
    return 0;
  } else {
    return -1;
  }
}

void SmartServoClass::setPosition(uint8_t const id, float const angle, uint16_t const speed) {
  mutex.lock();
  if (id<MAX_MOTORS) {
    _targetPosition[id] = angleToPosition(angle);
    _targetSpeed[id] = speed;
    if (_positionMode==PositionMode::IMMEDIATE) {
      writeWordCmd(id, toVal(SmartServoRegister::TARGET_POSITION_H), angleToPosition(angle));
    }
  }  
  mutex.unlock();
}

float SmartServoClass::getPosition(uint8_t const id) {
  mutex.lock();
  float ret = -1;
  if (id<MAX_MOTORS) {
    ret = positionToAngle(readWordCmd(id, toVal(SmartServoRegister::POSITION_H)));
  }
  mutex.unlock();
  return ret;
}

void SmartServoClass::center(uint8_t const id, uint16_t const position) {
  mutex.lock();
  writeWordCmd(id, toVal(SmartServoRegister::CENTER_POINT_ADJ_H), position);
  mutex.unlock();
}

void SmartServoClass::synchronize() {
  mutex.lock();
  _txPacket.id = 0xFE;
  _txPacket.length = (4+1)*MAX_MOTORS +4;
  _txPacket.instruction = toVal(SmartServoOperation::SYNC_WRITE);
  _txPacket.payload[0] = toVal(SmartServoRegister::TARGET_POSITION_H);
  _txPacket.payload[1] = 4;
  int index = 2;
  
  for (int i=1;i < MAX_MOTORS;i++) {
    _txPacket.payload[index++] = i;
    _txPacket.payload[index++] = _targetPosition[i] >>8;
    _txPacket.payload[index++] = _targetPosition[i];
    _txPacket.payload[index++] = _targetSpeed[i]>>8;
    _txPacket.payload[index++] = _targetSpeed[i];
  }
  sendPacket();
  mutex.unlock();
}

void  SmartServoClass::setTorque(bool const torque) {
  mutex.lock();
  writeByteCmd(BROADCAST, toVal(SmartServoRegister::TORQUE_SWITCH), torque ? 1 : 0);
  mutex.unlock();
}

void  SmartServoClass::setTorque(uint8_t const id, bool const torque) {
  mutex.lock();
  writeByteCmd(id, toVal(SmartServoRegister::TORQUE_SWITCH), torque ? 1 : 0);
  mutex.unlock();
}

void  SmartServoClass::setTime(uint8_t const id, uint16_t const time) {
  mutex.lock();
  writeWordCmd(id, toVal(SmartServoRegister::RUN_TIME_H), time);
  mutex.unlock();
}

void  SmartServoClass::setMaxTorque(uint16_t const torque) {
  mutex.lock();
  writeWordCmd(BROADCAST, toVal(SmartServoRegister::MAX_TORQUE_H), torque);
  mutex.unlock();
}

void  SmartServoClass::setMaxTorque(uint8_t const id, uint16_t const torque) {
  mutex.lock();
  writeWordCmd(id+1, toVal(SmartServoRegister::MAX_TORQUE_H), torque);
  mutex.unlock();
}

void  SmartServoClass::setID(uint8_t const id) {
  mutex.lock();
  writeByteCmd(BROADCAST, toVal(SmartServoRegister::ID), id);
  mutex.unlock();
}

void  SmartServoClass::engage(uint8_t const id) {
  mutex.lock();
  writeByteCmd(id, toVal(SmartServoRegister::TORQUE_SWITCH), 0x1);
  mutex.unlock();
}

void  SmartServoClass::disengage(uint8_t const id) {
  mutex.lock();
  writeByteCmd(id, toVal(SmartServoRegister::TORQUE_SWITCH), 0);
  mutex.unlock();
}

bool SmartServoClass::isEngaged(uint8_t const id) {
  mutex.lock();
  int ret = readByteCmd(id, toVal(SmartServoRegister::TORQUE_SWITCH));
  mutex.unlock();
  return ret != 0;
}

void  SmartServoClass::setStallProtectionTime(uint8_t const time) {
  mutex.lock();
  writeByteCmd(BROADCAST, toVal(SmartServoRegister::STALL_PROTECTION_TIME), time);
  mutex.unlock();
}

void  SmartServoClass::setStallProtectionTime(uint8_t const id, uint8_t const time) {
  mutex.lock();
  writeByteCmd(id, toVal(SmartServoRegister::STALL_PROTECTION_TIME), time);
  mutex.unlock();
}

void  SmartServoClass::setMinAngle(float const angle) {
  mutex.lock();
  writeByteCmd(BROADCAST, toVal(SmartServoRegister::MIN_ANGLE_LIMIT_H), angleToPosition(angle));
  mutex.unlock();
}

void  SmartServoClass::setMinAngle(uint8_t const id, float const angle) {
  mutex.lock();
  writeByteCmd(id, toVal(SmartServoRegister::MIN_ANGLE_LIMIT_H), angleToPosition(angle));
  mutex.unlock();
}

void  SmartServoClass::setMaxAngle(float const angle) {
  mutex.lock();
  writeByteCmd(BROADCAST, toVal(SmartServoRegister::MAX_ANGLE_LIMIT_H), angleToPosition(angle));
  mutex.unlock();
}

void  SmartServoClass::setMaxAngle(uint8_t const id, float const angle) {
  mutex.lock();
  writeByteCmd(id, toVal(SmartServoRegister::MAX_ANGLE_LIMIT_H), angleToPosition(angle));
  mutex.unlock();
}

void  SmartServoClass::getInfo(Stream & stream, uint8_t const id) {
  uint8_t regs[65];
  memset(regs, 0x55, sizeof(regs));
  int i = 0;
  mutex.lock();
  while (i < sizeof(regs)) {
    if ((i > 29 && i < 40) || (i > 48 && i < 56)) {
      i++;
      continue;
    }
    regs[i++] = readByteCmd(id, i);
  }
  mutex.unlock();
  stream.println("regs map:");
  for (i = 0; i < sizeof(regs); i++) {
    stream.println(String(i, HEX) + " : " + String(regs[i], HEX));
  }
}