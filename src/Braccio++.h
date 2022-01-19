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

class MotorsWrapper;

class BraccioClass
{
public:

  BraccioClass();

  inline bool begin() { return begin(nullptr); }
         bool begin(voidFuncPtr customMenu);


  void pingOn();
  void pingOff();
  bool connected(int const id);


  MotorsWrapper move(int const id);
  MotorsWrapper get (int const id);

  void moveTo(float const a1, float const a2, float const a3, float const a4, float const a5, float const a6);
  void positions(float * buffer);
  void positions(float & a1, float & a2, float & a3, float & a4, float & a5, float & a6);


  void speed(speed_grade_t speed_grade) {
    runTime  = speed_grade;
  }

  void disengage(int id = SmartServoClass::BROADCAST) {
    servos.disengage(id);
  }

  void engage(int id = SmartServoClass::BROADCAST) {
    servos.engage(id);
  }

  int getKey();
  void connectJoystickTo(lv_obj_t* obj);

  inline bool isJoystickPressed_LEFT()   { return (digitalRead(BTN_LEFT) == LOW); }
  inline bool isJoystickPressed_RIGHT()  { return (digitalRead(BTN_RIGHT) == LOW); }
  inline bool isJoystickPressed_SELECT() { return (digitalRead(BTN_SEL) == LOW); }
  inline bool isJoystickPressed_UP()     { return (digitalRead(BTN_UP) == LOW); }
  inline bool isJoystickPressed_DOWN()   { return (digitalRead(BTN_DOWN) == LOW); }
  inline bool isButtonPressed_ENTER()    { return (digitalRead(BTN_ENTER) == LOW); }

  TFT_eSPI gfx = TFT_eSPI();

  static BraccioClass& get_default_instance() {
    static BraccioClass dev;
    return dev;
  }

protected:
  // ioexpander APIs
  void digitalWrite(int pin, uint8_t value);

  // default display APIs
  void lvgl_splashScreen(unsigned long const duration_ms, std::function<void()> check_power_func);
  void lvgl_pleaseConnectPower();
  void defaultMenu();

  void setID(int id) {
    servos.setID(id);
  }

private:

  RS485Class serial485;
  SmartServoClass servos;
  PD_UFP_log_c PD_UFP;
  TCA6424A expander;
  Backlight bl;
  rtos::Thread _display_thd;
  void display_thread_func();

  bool _is_ping_allowed;
  bool _is_motor_connected[8];
  rtos::Mutex _motors_connected_mtx;
  rtos::Thread _motors_connected_thd;
  bool isPingAllowed();
  void setMotorConnectionStatus(int const id, bool const is_connected);
  void motorConnectedThreadFunc();

  speed_grade_t runTime; //ms

  voidFuncPtr _customMenu;

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
  lv_style_t _lv_style;

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
#endif

};

#define Braccio BraccioClass::get_default_instance()

class MotorsWrapper {
public:
  MotorsWrapper(SmartServoClass & servos, int idx) : _servos(servos), _idx(idx) {}
  MotorsWrapper& to(float angle) {
    _servos.setPosition(_idx, angle, _speed);
    return *this;
  }
  MotorsWrapper& in(std::chrono::milliseconds len) {
    _servos.setTime(_idx, len.count());
    return *this;
  }
  MotorsWrapper& move() {
    return *this;
  }
  float position() {
    return _servos.getPosition(_idx);
  }

  inline bool connected() { return Braccio.connected(_idx); }

  operator bool() {
    return connected();
  }
  void info(Stream& stream) {
    _servos.getInfo(stream, _idx);
  }
  void disengage() {
    _servos.disengage(_idx);
  }
  void engage() {
    _servos.engage(_idx);
  }
  bool engaged() {
    return _servos.isEngaged(_idx);
  }

private:
  SmartServoClass & _servos;
  int _idx;
  int _speed = 100;
};

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
