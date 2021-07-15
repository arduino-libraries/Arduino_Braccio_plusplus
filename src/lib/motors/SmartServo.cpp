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
	_serial->begin(115200, 0, 200);
	_serial->noReceive();
}

void SmartServoClass::setID(byte id)
{
	uint8_t pp[2] = {0x05,id};
	_tx(0xFE,0x04,0x03,pp);
}

void SmartServoClass::_tx(byte id, byte len, byte cmd, byte *prms) {
	
	uint8_t csum; //calc chk sum
	csum = 0xFF - id - len - cmd;
	for (uint8_t i=0; i<len-2 ; i++) {
		csum -= prms[i];
	}

	_serial->noReceive();
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
	_serial->receive();
}

void SmartServoClass::_rx(){

	for (uint8_t i=0; i<38; i++) {
		_r[i] = 0;
	}

	uint8_t i = 0;

	if (_serial->available() == 1) {
		return;
	}

	while (_serial->available()) {
		uint8_t c = _serial->read();
		if (i == 0 && c == 0) {
			continue;
		}
		_r[i] = c;
		i++;
	}
}


int8_t SmartServoClass::ping(uint8_t id)
{
	uint8_t csum; //calc chk sum
	csum = 0xFF - id - 0x02 - 0x01;

	_serial->noReceive();
	for (uint8_t i=0; i<38; i++) {
		_r[i] = 0;
	}
	_serial->beginTransmission();
	_serial->	write(0xFF);
	_serial->	write(0xFF);
	_serial->	write(id);
	_serial->	write(0x02);
	_serial->	write(0x01);
	_serial->	write(csum);
	_serial->endTransmission();
	_serial->receive();

	delayMicroseconds(400);
		
	_rx();

	if ((_r[1] == 0xF5) && (_r[2] == id)) {
		return _r[4];
		//implementare diversi stati di risposta? (caldo, troppa coppia...)
	}
	else {
		return -1;
	}
}

void SmartServoClass::setLimitAngle(byte id, uint16_t min, uint16_t max)
{

	byte com[2];

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

}

void SmartServoClass::setTorque(byte id, uint16_t torque)
{
	byte com[2];

	com[0] = 0x10;
	com[1] = highByte(torque);
	_tx(id,0x04,0x03,com);
	delay(waitTime);
	com[0] = 0x11;
	com[1] = lowByte(torque);
	_tx(id,0x04,0x03,com);
}

void SmartServoClass::hold(byte id, bool state) {
	byte pp[2] = {0x28,state};
	
	_tx(id,0x04,0x03,pp);
}

void SmartServoClass::goTo(byte id, uint16_t angle, uint16_t tempo)
{
	byte cmd[5];

	cmd[0] = 0x2a;
	cmd[1] = highByte(angle);
	cmd[2] = lowByte(angle);
	cmd[3] = highByte(tempo);
	cmd[4] = lowByte(tempo);

	_tx(id,0x07,0x03,cmd);
}

int SmartServoClass::regRead(byte id, char name)
{

	byte com[2];

	for (uint8_t i = 0; i<11; i++) {
		if (name == regName[i]) {
			com[0] = regAdd[i];
			com[1] = regLen[i];
		}
	}

	_tx(id,0x04,0x02,com);
	delayMicroseconds(500);
	_rx();

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
}