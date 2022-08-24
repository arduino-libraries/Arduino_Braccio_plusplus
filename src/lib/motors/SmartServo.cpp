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
    delayMicroseconds(10);
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

  if (!isValidId(id))
    return;

  float const MAX_ANGULAR_VELOCITY_deg_per_sec = 10.0f;

  float const target_position_deg = angleToPosition(angle);
  float const actual_position_deg = getPosition(id);
  if (actual_position_deg < 0.0f)
  {
    char msg[64] = {0};
    snprintf(msg, sizeof(msg), "error obtaining position for servo %d", (int)id);
    Serial.println(msg);
    return;
  }

  float const abs_position_diff_deg = fabs(target_position_deg - actual_position_deg);
  float const runtime_sec = static_cast<float>(getTime(id)) / 1000.0f;
  float const angular_velocity_deg_per_sec = abs_position_diff_deg / runtime_sec;

  /* Check whether or not the maximum allowed angular velocity is exceeded,
   * if it is indeed exceeded increase the runtime accordingly.
   */
  if (angular_velocity_deg_per_sec > MAX_ANGULAR_VELOCITY_deg_per_sec)
  {
    float const limited_runtime_sec = abs_position_diff_deg / MAX_ANGULAR_VELOCITY_deg_per_sec;
    setTime(id, static_cast<uint16_t>(limited_runtime_sec));

    char msg[64] = {0};
    snprintf(msg, sizeof(msg), "targed = %0.2f, actual = %0.2f, diff = %0.2f, omega_vel = %0.2f",
      target_position_deg, actual_position_deg, abs_position_diff_deg, angular_velocity_deg_per_sec);
    Serial.println(msg);
  }

  _targetPosition[idToArrayIndex(id)] = target_position_deg;

  if (_positionMode == PositionMode::IMMEDIATE)
  {
    mbed::ScopedLock<rtos::Mutex> lock(_mtx);
    writeWordCmd(id, REG(SmartServoRegister::TARGET_POSITION_H), target_position_deg);
  }
}

float SmartServoClass::getPosition(uint8_t const id)
{
  if (!isValidId(id))
    return -1.0f;

  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
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

uint16_t SmartServoClass::getTime(uint8_t const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);
  return readWordCmd(id, REG(SmartServoRegister::RUN_TIME_H));
}

void SmartServoClass::setMaxTorque(uint8_t const id, uint16_t const max_torque)
{
  mbed::ScopedLock<rtos::Mutex> lock(_mtx);

  if (max_torque > TORQUE_MAX)
    writeWordCmd(id, REG(SmartServoRegister::MAX_TORQUE_H), TORQUE_MAX);
  else
    writeWordCmd(id, REG(SmartServoRegister::MAX_TORQUE_H), max_torque);
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
