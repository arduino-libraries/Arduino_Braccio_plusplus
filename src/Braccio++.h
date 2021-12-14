#ifndef __BRACCIO_PLUSPLUS_H__
#define __BRACCIO_PLUSPLUS_H__

#include "Arduino.h"
#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "lib/motors/SmartServo.h"
#include "drivers/Ticker.h"
/*
#include "lib/PCINT/src/pcint.h"
#include "lib/ArduinoMenu/src/menu.h"
#include "lib/ArduinoMenu/src/menuIO/TFT_eSPIOut.h"
#include "lib/ArduinoMenu/src/menuIO/interruptPins.h"
#include "lib/ArduinoMenu/src/menuIO/keyIn.h"
#include "lib/ArduinoMenu/src/menuIO/chainStream.h"
#include "lib/ArduinoMenu/src/menuIO/serialOut.h"
#include "lib/ArduinoMenu/src/menuIO/serialIn.h"
*/
#include "lib/TFT_eSPI/TFT_eSPI.h" // Hardware-specific library
#include <lvgl.h>

extern const lv_img_dsc_t img_bulb_gif;

extern "C" {
	void braccio_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
	void read_keypad(lv_indev_drv_t * indev, lv_indev_data_t * data);
};

enum speed_grade_t {
	FAST = 10,
	MEDIUM = 100,
	SLOW = 1000,
};

#include <chrono>
using namespace std::chrono;

class MotorsWrapper {
public:
	MotorsWrapper(SmartServoClass<7>* servos, int idx) : _servos(servos), _idx(idx) {}
	MotorsWrapper& to(float angle) {
		_servos->setPosition(_idx, angle, _speed);
		return *this;
	}
	MotorsWrapper& in(std::chrono::milliseconds len) {
		_servos->setTime(_idx, len.count());
		return *this;
	}
	MotorsWrapper& move() {
		return *this;
	}
	float position() {
		return _servos->getPosition(_idx);
	}
	bool connected() {
		return _servos->ping(_idx) == 0;
	}
	operator bool() {
		return connected();
	}
	void info(Stream& stream) {
		_servos->getInfo(stream, _idx);
	}

private:
	SmartServoClass<7>* _servos;
	int _idx;
	int _speed = 100;
};

class BraccioClass {
public:
	BraccioClass() {}
	bool begin(voidFuncPtr customMenu = nullptr);

	// setters
	MotorsWrapper move(int joint_index) {
		MotorsWrapper wrapper(servos, joint_index);
		return wrapper;
	}
	MotorsWrapper get(int joint_index) {
		return move(joint_index);
	}
	void moveTo(int joint_index, int position) {
		//servos->setPosition(joint_index, position, 100);
	}
	void moveTo(int joint_index, float angle) {
		servos->setPosition(joint_index, angle, 100);
	}
	void moveTo(float a1, float a2, float a3, float a4, float a5, float a6) {
		servos->setPositionMode(pmSYNC);
		servos->setPosition(1, a1, runTime);
		servos->setPosition(2, a2, runTime);
		servos->setPosition(3, a3, runTime);
		servos->setPosition(4, a4, runTime);
		servos->setPosition(5, a5, runTime);
		servos->setPosition(6, a6, runTime);
		servos->setPositionMode(pmIMMEDIATE);
	}
	// getters
	void positions(float* buffer) {
		for (int i = 1; i < 7; i++)	{
			*buffer++ = servos->getPosition(i);
		}
	}
	void positions(float& a1, float& a2, float& a3, float& a4, float& a5, float& a6) {
		// TODO: add check if motors are actually connected
		a1 = servos->getPosition(1);
		a2 = servos->getPosition(2);
		a3 = servos->getPosition(3);
		a4 = servos->getPosition(4);
		a5 = servos->getPosition(5);
		a6 = servos->getPosition(6);
	}
	float position(int joint_index);
	bool connected(int joint_index) {
		return _connected[joint_index];
	}

	void speed(speed_grade_t speed_grade) {
		runTime  = speed_grade;
	}

	int getKey();
	void connectJoystickTo(lv_obj_t* obj);

	TFT_eSPI gfx = TFT_eSPI();

	bool ping_allowed = true;

	static BraccioClass& get_default_instance();

protected:
	// ioexpander APIs
	void digitalWrite(int pin, uint8_t value);

	// default display APIs
	void drawMenu();
	void hideMenu();
	void drawImage(char* image);
	void defaultMenu();

	void setID(int id) {
		servos->setID(id);
	}

private:

  RS485Class serial485 = RS485Class(Serial1, 0, 7, 8); // TX, DE, RE
  SmartServoClass<7>* servos = new SmartServoClass<7>(serial485);

	PD_UFP_log_c PD_UFP = PD_UFP_log_c(PD_LOG_LEVEL_VERBOSE);
	TCA6424A expander = TCA6424A(TCA6424A_ADDRESS_ADDR_HIGH);
	Backlight bl;

	speed_grade_t runTime = MEDIUM; //ms

	voidFuncPtr _customMenu = nullptr;

	const int BTN_LEFT = 3;
	const int BTN_RIGHT = 4;
	const int BTN_UP = 5;
	const int BTN_DOWN = 2;
	const int BTN_SEL = A0;
	const int BTN_ENTER = A1;

  lv_disp_drv_t disp_drv;
  lv_indev_drv_t indev_drv;
	lv_disp_draw_buf_t disp_buf;
	lv_color_t buf[240 * 240 / 10];
	lv_group_t* p_objGroup;
	lv_indev_t *kb_indev;

  bool _connected[8];

#ifdef __MBED__
	rtos::EventFlags pd_events;
	rtos::Mutex i2c_mutex;
	mbed::Ticker pd_timer;

	unsigned int start_pd_burst = 0xFFFFFFFF;

	void unlock_pd_semaphore_irq() {
		start_pd_burst = millis();
		pd_events.set(2);
	}

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

#define Braccio BraccioClass::get_default_instance()

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