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
, _mtx{}
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
  _txPacket.instruction = CMD(SmartServoOperation::WRITE);
  _txPacket.payload[0]=address;
  _txPacket.payload[1]=data;
  sendPacket();
}

void SmartServoClass::writeWordCmd(uint8_t const id, uint8_t const address, uint16_t const data) {
  _txPacket.id = id;
  _txPacket.length = 2+3;
  _txPacket.instruction = CMD(SmartServoOperation::WRITE);
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
  _txPacket.instruction = CMD(SmartServoOperation::READ);
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

int SmartServoClass::ping(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeCmd(id, SmartServoOperation::PING);
  // TODO: check return
  receiveResponse(6);
  if (_rxLen==6 && 
    _rxBuf[0]==0xff &&
    _rxBuf[1]==0xf5 &&
    _rxBuf[2]==id &&
    _rxBuf[3]==2) {
    return _rxBuf[4];
  }
  _errors++;
  if (_onError) _onError();
  return -1;
}

/*
// ATTENTION: RESET also changes the ID of the motor

void SmartServoClass::reset(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeCmd(id, SmartServoOperation::RESET);
}
*/

void SmartServoClass::action(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeCmd(id, SmartServoOperation::ACTION);
}

void SmartServoClass::begin()
{
  _txPacket.header[0] = 0xff;
  _txPacket.header[1] = 0xff;
  _RS485.begin(115200, 0, 90);
  _RS485.receive();
  writeByteCmd(BROADCAST, REG(SmartServoRegister::SERVO_MOTOR_MODE), 1);
  writeByteCmd(BROADCAST, REG(SmartServoRegister::TORQUE_SWITCH) ,1);
  _positionMode = PositionMode::IMMEDIATE;

  for (int i = MIN_MOTOR_ID; i <= MAX_MOTOR_ID; i++)
    _targetSpeed[idToArrayIndex(i)] = 1000;
}

void SmartServoClass::setPosition(uint8_t const id, float const angle)
{
  if (!isValidAngle(angle))
    return;

  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  if (isValidId(id))
  {
    _targetPosition[idToArrayIndex(id)] = angleToPosition(angle);
    if (_positionMode==PositionMode::IMMEDIATE) {
      writeWordCmd(id, REG(SmartServoRegister::TARGET_POSITION_H), angleToPosition(angle));
    }
  }  
}

float SmartServoClass::getPosition(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  float ret = -1;
  if (isValidId(id))
    return positionToAngle(readWordCmd(id, REG(SmartServoRegister::POSITION_H)));
}

void SmartServoClass::center(uint8_t const id, uint16_t const position)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeWordCmd(id, REG(SmartServoRegister::CENTER_POINT_ADJ_H), position);
}

void SmartServoClass::synchronize()
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  _txPacket.id = 0xFE;
  _txPacket.length = MAX_TX_PAYLOAD_LEN;
  _txPacket.instruction = CMD(SmartServoOperation::SYNC_WRITE);
  _txPacket.payload[0] = REG(SmartServoRegister::TARGET_POSITION_H);
  _txPacket.payload[1] = 4;
  int index = 2;
  
  for (int i = MIN_MOTOR_ID; i <= MAX_MOTOR_ID; i++) {
    _txPacket.payload[index++] = i;
    _txPacket.payload[index++] = _targetPosition[idToArrayIndex(i)]>>8;
    _txPacket.payload[index++] = _targetPosition[idToArrayIndex(i)];
    _txPacket.payload[index++] = _targetSpeed[idToArrayIndex(i)]>>8;
    _txPacket.payload[index++] = _targetSpeed[idToArrayIndex(i)];
  }
  sendPacket();
}

void SmartServoClass::setTorque(bool const torque)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeByteCmd(BROADCAST, REG(SmartServoRegister::TORQUE_SWITCH), torque ? 1 : 0);
}

void SmartServoClass::setTorque(uint8_t const id, bool const torque)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeByteCmd(id, REG(SmartServoRegister::TORQUE_SWITCH), torque ? 1 : 0);
}

void SmartServoClass::setTime(uint16_t const time)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  for (int i = MIN_MOTOR_ID; i <= MAX_MOTOR_ID; i++)
    _targetSpeed[idToArrayIndex(i)] = time;
  writeWordCmd(BROADCAST, REG(SmartServoRegister::RUN_TIME_H), time);
}

void SmartServoClass::setTime(uint8_t const id, uint16_t const time)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  if ((id >= MIN_MOTOR_ID) && (id <= MAX_MOTOR_ID)) _targetSpeed[idToArrayIndex(id)] = time;
  writeWordCmd(id, REG(SmartServoRegister::RUN_TIME_H), time);
}

void SmartServoClass::setMaxTorque(uint16_t const torque)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeWordCmd(BROADCAST, REG(SmartServoRegister::MAX_TORQUE_H), torque);
}

void SmartServoClass::setMaxTorque(uint8_t const id, uint16_t const torque)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeWordCmd(id+1, REG(SmartServoRegister::MAX_TORQUE_H), torque);
}

void SmartServoClass::setID(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeByteCmd(BROADCAST, REG(SmartServoRegister::ID), id);
}

void SmartServoClass::engage(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeByteCmd(id, REG(SmartServoRegister::TORQUE_SWITCH), 0x1);
}

void SmartServoClass::disengage(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeByteCmd(id, REG(SmartServoRegister::TORQUE_SWITCH), 0);
}

bool SmartServoClass::isEngaged(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  int ret = readByteCmd(id, REG(SmartServoRegister::TORQUE_SWITCH));
  return ret != 0;
}

void SmartServoClass::setStallProtectionTime(uint8_t const time)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeByteCmd(BROADCAST, REG(SmartServoRegister::STALL_PROTECTION_TIME), time);
}

void SmartServoClass::setStallProtectionTime(uint8_t const id, uint8_t const time)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeByteCmd(id, REG(SmartServoRegister::STALL_PROTECTION_TIME), time);
}

void SmartServoClass::setMinAngle(uint16_t const min_angle)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeWordCmd(BROADCAST, REG(SmartServoRegister::MIN_ANGLE_LIMIT_H), min_angle);
}

void SmartServoClass::setMinAngle(uint8_t const id, uint16_t const min_angle)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeWordCmd(id, REG(SmartServoRegister::MIN_ANGLE_LIMIT_H), min_angle);
}

void SmartServoClass::setMaxAngle(uint16_t const max_angle)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeWordCmd(BROADCAST, REG(SmartServoRegister::MAX_ANGLE_LIMIT_H), max_angle);
}

void SmartServoClass::setMaxAngle(uint8_t const id, uint16_t const max_angle)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  writeWordCmd(id, REG(SmartServoRegister::MAX_ANGLE_LIMIT_H), max_angle);
}

void  SmartServoClass::getInfo(Stream & stream, uint8_t const id)
{
  uint8_t regs[65];
  memset(regs, 0x55, sizeof(regs));
  int i = 0;

  {
    mbed::ScopedLock<rtos::Mutex> lock(_mtx);
    while (i < sizeof(regs)) {
      if ((i > 29 && i < 40) || (i > 48 && i < 56)) {
        i++;
        continue;
      }
      regs[i++] = readByteCmd(id, i);
    }
  }

  stream.println("regs map:");
  for (i = 0; i < sizeof(regs); i++) {
    stream.println(String(i, HEX) + " : " + String(regs[i], HEX));
  }
}
