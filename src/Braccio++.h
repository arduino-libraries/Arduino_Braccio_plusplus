#ifndef __BRACCIO_PLUSPLUS_H__
#define __BRACCIO_PLUSPLUS_H__

#include "Arduino.h"
#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "lib/motors/SmartServo.h"
#include "drivers/Ticker.h"
#include "lib/PCINT/src/pcint.h"
#include "lib/ArduinoMenu/src/menu.h"
#include "lib/ArduinoMenu/src/menuIO/TFT_eSPIOut.h"
#include "lib/ArduinoMenu/src/menuIO/interruptPins.h"
#include "lib/ArduinoMenu/src/menuIO/keyIn.h"
#include "lib/ArduinoMenu/src/menuIO/chainStream.h"
#include "lib/ArduinoMenu/src/menuIO/serialOut.h"
#include "lib/ArduinoMenu/src/menuIO/serialIn.h"
#include "lib/TFT_eSPI/TFT_eSPI.h" // Hardware-specific library

class MotorsWrapper {
public:
	MotorsWrapper(SmartServoClass<7>* servos, int idx) : _servos(servos), _idx(idx) {}
	MotorsWrapper& to(float angle) {
		_servos->setPosition(_idx, angle, default_speed);
		return *this;
	}
	MotorsWrapper& to(int position) {
		return *this;
	}
private:
	SmartServoClass<7>* _servos;
	int _idx;
	const int default_speed = 1000;
};

class BraccioClass {
public:
	BraccioClass() {}
	bool begin(positionMode _positionMode = pmSYNC);
	// setters
	MotorsWrapper move(int joint_index) {
		MotorsWrapper wrapper(servos, joint_index);
		return wrapper;
	}
	void moveTo(int joint_index, int position);
	void moveTo(int joint_index, float angle);
	void moveTo(float a1, float a2, float a3, float a4, float a5, float a6, float a7);
	// getters
	void positions(int* buffer);
	void positions(float* buffer);
	void positions(float& a1, float& a2, float& a3, float& a4, float& a5, float& a6, float& a7);
	int position(int joint_index);
	float angle(int joint_index);
	bool connected(int joint_index) {
		return _connected[joint_index];
	}

protected:
	// ioexpander APIs
	void digitalWrite(int pin, uint8_t value);

	// default display APIs
	void drawMenu();
	void hideMenu();
	void drawImage(char* image);

private:
	RS485Class serial485 = RS485Class(Serial1, 0, 7, 8); // TX, DE, RE
	SmartServoClass<7>* servos = new SmartServoClass<7>(serial485);

	class PD_UFP_c PD_UFP;
	TCA6424A expander = TCA6424A(TCA6424A_ADDRESS_ADDR_HIGH);
	Backlight bl;

  	bool _connected[8];

#ifdef __MBED__
	rtos::EventFlags pd_events;
	rtos::Mutex pd_mutex;
	mbed::Ticker pd_timer;

	void unlock_pd_semaphore() {
		pd_events.set(1);
	}

	void setGreen(int i) {
		expander.writePin(i * 2 - 1, 0);
		expander.writePin(i * 2 - 2, 1);
	}

	void setRed(int i) {
		expander.writePin(i * 2 - 1, 1);
		expander.writePin(i * 2 - 2, 0);
	}

	void pd_thread();
	void display_thread();
	void motors_connected_thread();
#endif

};

extern BraccioClass Braccio;

struct __callback__container__ {
  mbed::Callback<void()> fn;
};

inline void attachInterrupt(pin_size_t interruptNum, mbed::Callback<void()> func, PinStatus mode) {
  struct __callback__container__* a = new __callback__container__();
  a->fn = func;
  auto callback = [](void* a) -> void {
    ((__callback__container__*)a)->fn();
  };

  attachInterruptParam(interruptNum, callback, mode, (void*)a);
}

#endif //__BRACCIO_PLUSPLUS_H__