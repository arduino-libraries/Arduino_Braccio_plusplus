/*
	smartServoSS485.h - Library for controlling the smart servo.
	Created by Paolo Cavagnolo, may 2021 - paolo.cavagnolo@gmail.com.
	Released into the public domain.
*/

#include "Arduino.h"
#include "SmartServo.h"

#define waitTime 10

char regName[11]  	=	{'i',	'a',	'A',	'T',	'm',	'd',	't',	'P',	'x',	'p',	's'};
byte regAdd[11] 		=	{0x05,0x09,	0x0b,	0x10,	0x1c,	0x1d,	0x28,	0x2a,	0x2c,	0x38,	0x3a};
byte regLen[11] 		=	{1,		2,		2,		2,		1,		1,		1,		2,		2,		2,		2};


SmartServoClass::SmartServoClass(RS485Class& serial) :
	_serial(&serial)
{
}

void SmartServoClass::begin() {
	_serial->begin(115200, 100, 100);
	_serial->receive();
}

void SmartServoClass::setID(byte id)
{
	uint8_t pp[2] = {0x05,id};
	_tx(0xFE,0x04,0x03,pp);
}

int SmartServoClass::_tx(byte id, byte len, byte cmd, byte *prms) {
	
	uint8_t csum; //calc chk sum
	csum = 0xFF - id - len - cmd;
	for (uint8_t i=0; i<len-2 ; i++) {
		csum -= prms[i];
	}

	for (uint8_t i=0; i<38; i++) {
		_r[i] = 0;
	}
	_serial->beginTransmission();
	_serial->write(0xFF);
	_serial->write(0xFF);
	_serial->write(id);
	_serial->write(len);
	_serial->write(cmd);
	for (uint8_t i=0; i<len-2; i++) {
		_serial->write(prms[i]);
	}
	_serial->write(csum);
	_serial->endTransmission();

	return len + 4;
}

void SmartServoClass::_rx(int len){

	uint8_t i = 0;

	while (_serial->available() < 5) {
		yield();
	}

	while (_serial->available()) {
		uint8_t c = _serial->read();
		Serial.print(c, HEX);
		Serial.print(" ");
		delay(1);
		if (i >= len) {
			_r[i - len] = c;
		}
		i++;
	}
	if (i != 0) {
		Serial.println();
	}
}


bool SmartServoClass::ping(uint8_t id)
{
	uint8_t csum; //calc chk sum
	csum = 0xFF - id - 0x02 - 0x01;

	mutex.lock();

	_serial->beginTransmission();
	_serial->	write(0xFF);
	_serial->	write(0xFF);
	_serial->	write(id);
	_serial->	write(0x02);
	_serial->	write(0x01);
	_serial->	write(csum);
	_serial->endTransmission();

	_rx(6);

	bool res = false;

	if (((_r[1] == 0xF5) && (_r[2] == id)) || ((_r[0] == 0xF5) && (_r[1] == id))) {
		res = true;
	}

	memset(_r, 0, sizeof(_r));

	mutex.unlock();

	return res;
}

void SmartServoClass::setLimitAngle(byte id, uint16_t min, uint16_t max)
{

	byte com[2];

	mutex.lock();

	com[0] = 0x09;
	com[1] = highByte(min);
	_tx(id,0x04,0x03,com);
	delay(waitTime);
	com[0] = 0x0a;
	com[1] = lowByte(min);
	_tx(id,0x04,0x03,com);

	delay(waitTime);
	com[0] = 0x0b;
	com[1] = highByte(max);
	_tx(id,0x04,0x03,com);
	delay(waitTime);
	com[0] = 0x0c;
	com[1] = lowByte(max);
	_tx(id,0x04,0x03,com);

	mutex.unlock();
}

void SmartServoClass::setTorque(byte id, uint16_t torque)
{
	byte com[2];

	mutex.lock();

	com[0] = 0x10;
	com[1] = highByte(torque);
	_tx(id,0x04,0x03,com);
	delay(waitTime);
	com[0] = 0x11;
	com[1] = lowByte(torque);
	_tx(id,0x04,0x03,com);

	mutex.unlock();
}

void SmartServoClass::hold(byte id, bool state) {
	byte pp[2] = {0x28,state};
	
	mutex.lock();

	_tx(id,0x04,0x03,pp);
	mutex.unlock();
}

void SmartServoClass::goTo(byte id, uint16_t angle, uint16_t tempo)
{
	byte cmd[5];

	mutex.lock();

	cmd[0] = 0x2a;
	cmd[1] = highByte(angle);
	cmd[2] = lowByte(angle);
	cmd[3] = highByte(tempo);
	cmd[4] = lowByte(tempo);

	_tx(id,0x07,0x03,cmd);

	mutex.unlock();
}

int SmartServoClass::regRead(byte id, char name)
{

	byte com[2];

	mutex.lock();

	for (uint8_t i = 0; i<11; i++) {
		if (name == regName[i]) {
			com[0] = regAdd[i];
			com[1] = regLen[i];
		}
	}

	int len = _tx(id,0x04,0x02,com);
	_rx(len);

	mutex.unlock();

	if ((_r[1] == 0xF5) && (_r[2] == id)) {
		if (com[1] == 2) {
			return word(_r[5],_r[6]);
		}
		else {
			return _r[5];
		}
	}
	else {
		return -1;
	}
}

void SmartServoClass::regWrite(byte id, char name, uint16_t value)
{
	byte com[3];
	uint8_t len;

	mutex.lock();

	for (uint8_t i = 0; i<11; i++) {
		if (name == regName[i]) {
			com[0] = regAdd[i];
			len = regLen[1];
			if (len == 2) {
				com[1] = highByte(value);
				com[2] = lowByte(value);
			}
			else {
				com[1] = value;
			}
		}
	}

	_tx(id,3+len,0x03,com);

	mutex.unlock();
}