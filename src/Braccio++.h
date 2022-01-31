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

#ifndef __BRACCIO_PLUSPLUS_H__
#define __BRACCIO_PLUSPLUS_H__

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "Arduino.h"
#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "lib/motors/SmartServo.h"
#include "drivers/Ticker.h"

#include "lib/TFT_eSPI/TFT_eSPI.h" // Hardware-specific library
#include <lvgl.h>

#include <chrono>
using namespace std::chrono;

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum speed_grade_t {
  FAST = 10,
  MEDIUM = 100,
  SLOW = 1000,
};

/**************************************************************************************
 * FORWARD DECLARATION
 **************************************************************************************/

class Servo;

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class BraccioClass
{
public:

  BraccioClass();

  inline bool begin() { return begin(nullptr); }
         bool begin(voidFuncPtr custom_menu);


  void pingOn();
  void pingOff();
  bool connected(int const id);


  Servo move(int const id);
  Servo get (int const id);

  void moveTo(float const a1, float const a2, float const a3, float const a4, float const a5, float const a6);
  void positions(float * buffer);
  void positions(float & a1, float & a2, float & a3, float & a4, float & a5, float & a6);

  inline void speed(speed_grade_t const speed_grade) { _servos.setTime(SmartServoClass::BROADCAST, speed_grade); }
  inline void speed(int const id, speed_grade_t const speed_grade) { _servos.setTime(id, speed_grade); }

  inline void disengage(int const id = SmartServoClass::BROADCAST) { _servos.disengage(id); }
  inline void engage   (int const id = SmartServoClass::BROADCAST) { _servos.engage(id); }

  int getKey();
  void connectJoystickTo(lv_obj_t* obj);

  inline bool isJoystickPressed_LEFT()   { return (digitalRead(BTN_LEFT) == LOW); }
  inline bool isJoystickPressed_RIGHT()  { return (digitalRead(BTN_RIGHT) == LOW); }
  inline bool isJoystickPressed_SELECT() { return (digitalRead(BTN_SEL) == LOW); }
  inline bool isJoystickPressed_UP()     { return (digitalRead(BTN_UP) == LOW); }
  inline bool isJoystickPressed_DOWN()   { return (digitalRead(BTN_DOWN) == LOW); }
  inline bool isButtonPressed_ENTER()    { return (digitalRead(BTN_ENTER) == LOW); }

  static BraccioClass& get_default_instance() {
    static BraccioClass dev;
    return dev;
  }

  /* Those functions MUST NOT be used by the user. */
  void lvgl_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  void onPowerIrqEvent();
  void onPowerTimerEvent();

protected:

  inline void setID(int const id) { _servos.setID(id); }

private:

  void button_init();

  rtos::Mutex _i2c_mtx;
  RS485Class _serial485;
  SmartServoClass _servos;
  PD_UFP_log_c _PD_UFP;
  TCA6424A _expander;
  bool expander_init();
  void expander_setGreen(int const i);
  void expander_setRed(int const i);

  bool _is_ping_allowed;
  bool _is_motor_connected[SmartServoClass::NUM_MOTORS];
  rtos::Mutex _motors_connected_mtx;
  rtos::Thread _motors_connected_thd;
  bool isPingAllowed();
  void setMotorConnectionStatus(int const id, bool const is_connected);
  void motorConnectedThreadFunc();


  static int constexpr BTN_LEFT  = 3;
  static int constexpr BTN_RIGHT = 4;
  static int constexpr BTN_UP    = 5;
  static int constexpr BTN_DOWN  = 2;
  static int constexpr BTN_SEL   = A0;
  static int constexpr BTN_ENTER = A1;


  static size_t constexpr LVGL_DRAW_BUFFER_SIZE = 240 * 240 / 10;

  Backlight _bl;
  TFT_eSPI _gfx;
  lv_disp_drv_t _lvgl_disp_drv;
  lv_indev_drv_t _lvgl_indev_drv;
  lv_disp_draw_buf_t _lvgl_disp_buf;
  lv_color_t _lvgl_draw_buf[LVGL_DRAW_BUFFER_SIZE];
  lv_group_t * _lvgl_p_obj_group;
  lv_indev_t * _lvgl_kb_indev;
  lv_style_t _lv_style;
  rtos::Thread _display_thd;
  bool backlight_init();
  void display_init();
  void lvgl_init();
  void display_thread_func();
  void lvgl_splashScreen(unsigned long const duration_ms);
  void lvgl_pleaseConnectPower();
  void lvgl_defaultMenu();


  static uint32_t constexpr PD_IRQ_EVENT_FLAG   = 1;
  static uint32_t constexpr PD_TIMER_EVENT_FLAG = 2;
  rtos::EventFlags _pd_events;
  mbed::Ticker _pd_timer;
  rtos::Thread _pd_thd;
  void pd_thread_func();
};

#define Braccio BraccioClass::get_default_instance()

class Servo
{
public:

  Servo(SmartServoClass & servos, int const id) : _servos(servos), _id(id) { }

  inline void disengage() { _servos.disengage(_id); }
  inline void engage()    { _servos.engage(_id); }
  inline bool engaged()   { return _servos.isEngaged(_id); }

  inline Servo & move()                                    { return *this; }
  inline Servo & to  (float const angle)                   { _servos.setPosition(_id, angle); return *this; }
  inline Servo & in  (std::chrono::milliseconds const len) { _servos.setTime(_id, len.count()); return *this; }

  inline float position()            { return _servos.getPosition(_id); }
  inline bool  connected()           { return Braccio.connected(_id); }
  inline void  info(Stream & stream) { _servos.getInfo(stream, _id); }

  operator bool() { return connected(); }


private:

  SmartServoClass & _servos;
  int const _id;
};

#endif //__BRACCIO_PLUSPLUS_H__
