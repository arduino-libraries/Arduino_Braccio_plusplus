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

#include "Braccio++.h"

#if LVGL_VERSION_MAJOR < 8 || (LVGL_VERSION_MAJOR == 8 && LVGL_VERSION_MINOR < 1)
#error Please use lvgl >= 8.1
#endif

#include "mbed.h"

/**************************************************************************************
 * DEFINES
 **************************************************************************************/

/* Uncommenting this line enables debug output of the USB Power Delivery library. */
//#define BRACCIO_POWER_DEBUG_ENABLE

/**************************************************************************************
 * FUNCTION DECLARATION
 **************************************************************************************/

#if LV_USE_LOG
void lvgl_my_print(const char * dsc);
#endif /* #if LV_USE_LOG */

extern "C"
{
  void braccio_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  void braccio_read_keypad(lv_indev_drv_t * indev, lv_indev_data_t * data);
  void braccio_onPowerIrqEvent();
  void braccio_onPowerTimerEvent();
};

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

using namespace std::chrono_literals;

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

BraccioClass::BraccioClass()
: _i2c_mtx{}
, _serial485{Serial1, 0, 7, 8} /* TX, DE, RE */
, _servos{_serial485}
, _PD_UFP{PD_LOG_LEVEL_VERBOSE}
, _expander{TCA6424A_ADDRESS_ADDR_HIGH, _i2c_mtx}
, _is_ping_allowed{true}
, _is_motor_connected{false}
, _motors_connected_mtx{}
, _motors_connected_thd{}
, _bl{_i2c_mtx}
, _gfx{}
, _lvgl_disp_drv{}
, _lvgl_indev_drv{}
, _lvgl_disp_buf{}
, _lvgl_draw_buf{}
, _lvgl_p_obj_group{nullptr}
, _lvgl_kb_indev{nullptr}
, _display_mtx{}
, _display_thd{}
, _pd_events{}
, _pd_timer{}
, _pd_thd{osPriorityHigh}
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

bool BraccioClass::begin(voidFuncPtr custom_menu, bool const wait_for_all_motor_connected)
{
  static int constexpr RS485_RX_PIN = 1;

  SPI.begin();
  Wire.begin();
  Serial.begin(115200);

  pinMode(PIN_FUSB302_INT, INPUT_PULLUP);
  attachInterrupt(PIN_FUSB302_INT, braccio_onPowerIrqEvent, FALLING);
  pinMode(RS485_RX_PIN, INPUT_PULLUP);

  _PD_UFP.init_PPS(_i2c_mtx, PPS_V(7.2), PPS_A(2.0));
  _pd_timer.attach(braccio_onPowerTimerEvent, 10ms);
  braccio_onPowerIrqEvent(); /* Start power burst. */
  _pd_thd.start(mbed::callback(this, &BraccioClass::pd_thread_func));

  button_init();

  if (!expander_init()) return false;

  display_init();
  if (!backlight_init()) return false;
  lvgl_init();
  _display_thd.start(mbed::callback(this, &BraccioClass::display_thread_func));

  lvgl_splashScreen(2000);
  lv_obj_clean(lv_scr_act());

  if (!_PD_UFP.is_PPS_ready())
    lvgl_pleaseConnectPower();

  /* Loop forever, if no power is attached. */
  while(!_PD_UFP.is_PPS_ready()) { delay(10); }
  lv_obj_clean(lv_scr_act());

  if (custom_menu)
    custom_menu();
  else
    lvgl_defaultMenu();

  _servos.begin();
  _servos.setTime(SLOW);
  _servos.setPositionMode(PositionMode::IMMEDIATE);

  _motors_connected_thd.start(mbed::callback(this, &BraccioClass::motorConnectedThreadFunc));

  if (wait_for_all_motor_connected)
  {
    /* Wait for all motors to be actually connected. */
    for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++)
    {
      auto const start = millis();
      auto isTimeout = [start]() -> bool { return ((millis() - start) > 5000); };
      for(; !isTimeout() && !connected(id); delay(100)) { }
      if (isTimeout()) return false;
    }
  }

  return true;
}

void BraccioClass::pingOn()
{
  mbed::ScopedLock<rtos::Mutex> lock(_motors_connected_mtx);
  _is_ping_allowed = true;
}

void BraccioClass::pingOff()
{
  mbed::ScopedLock<rtos::Mutex> lock(_motors_connected_mtx);
  _is_ping_allowed = false;
}

bool BraccioClass::connected(int const id)
{
  mbed::ScopedLock<rtos::Mutex> lock(_motors_connected_mtx);
  return _is_motor_connected[SmartServoClass::idToArrayIndex(id)];
}

Servo BraccioClass::move(int const id)
{
  Servo wrapper(_servos, id);
  return wrapper;
}

Servo BraccioClass::get(int const id)
{
  return move(id);
}

void BraccioClass::moveTo(float const a1, float const a2, float const a3, float const a4, float const a5, float const a6)
{
  _servos.setPositionMode(PositionMode::SYNC);
  _servos.setPosition(1, a1);
  _servos.setPosition(2, a2);
  _servos.setPosition(3, a3);
  _servos.setPosition(4, a4);
  _servos.setPosition(5, a5);
  _servos.setPosition(6, a6);
  _servos.synchronize();
  _servos.setPositionMode(PositionMode::IMMEDIATE);
}

void BraccioClass::positions(float * buffer)
{
  for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++)
    *buffer++ = _servos.getPosition(id);
}

void BraccioClass::positions(float & a1, float & a2, float & a3, float & a4, float & a5, float & a6)
{
  // TODO: add check if motors are actually connected
  a1 = _servos.getPosition(1);
  a2 = _servos.getPosition(2);
  a3 = _servos.getPosition(3);
  a4 = _servos.getPosition(4);
  a5 = _servos.getPosition(5);
  a6 = _servos.getPosition(6);
}

int BraccioClass::getKey() {
  if (isJoystickPressed_LEFT()) {
    return 1;
  }
  if (isJoystickPressed_RIGHT()) {
    return 2;
  }
  if (isJoystickPressed_SELECT()) {
    return 3;
  }
  if (isJoystickPressed_UP()) {
    return 4;
  }
  if (isJoystickPressed_DOWN()) {
    return 5;
  }
  if (isButtonPressed_ENTER()) {
    return 6;
  }
  return 0;
}

void BraccioClass::connectJoystickTo(lv_obj_t* obj)
{
  mbed::ScopedLock<rtos::Mutex> lock(_display_mtx);
  lv_group_add_obj(_lvgl_p_obj_group, obj);
  lv_indev_set_group(_lvgl_kb_indev, _lvgl_p_obj_group);
}

void BraccioClass::lvgl_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  _gfx.startWrite();
  _gfx.setAddrWindow(area->x1, area->y1, w, h);
  _gfx.pushColors(&color_p->full, w * h, true);
  _gfx.endWrite();

  lv_disp_flush_ready(disp);
}

void BraccioClass::onPowerIrqEvent()
{
 _pd_events.set(PD_IRQ_EVENT_FLAG);
}

void BraccioClass::onPowerTimerEvent()
{
  _pd_events.set(PD_TIMER_EVENT_FLAG);
}

/**************************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************************************/

void BraccioClass::button_init()
{
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_SEL,   INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
}

bool BraccioClass::expander_init()
{
  static uint16_t constexpr EXPANDER_RS485_SLEW_PIN   = 21; /* P25 = 8 * 2 + 5 = 21 */
  static uint16_t constexpr EXPANDER_RS485_TERM_PIN   = 19; /* P23 = 8 * 2 + 3 = 19 */
  static uint16_t constexpr EXPANDER_RS485_RST_DP_PIN = 18; /* P22 = 8 * 2 + 2 = 18 */

  if (!_expander.testConnection())
    return false;

  /* Init IO expander outputs of RGB LEDs */
  for (int i = 0; i < 14; i++) {
    _expander.setPinDirection(i, 0);
  }

  /* Set SLEW to low */
  _expander.setPinDirection(EXPANDER_RS485_SLEW_PIN, 0);
  _expander.writePin(EXPANDER_RS485_SLEW_PIN, 0);

  /* Set TERM to HIGH (default) */
  _expander.setPinDirection(EXPANDER_RS485_TERM_PIN, 0);
  _expander.writePin(EXPANDER_RS485_TERM_PIN, 1);

  /* Reset GLCD */
  _expander.setPinDirection(EXPANDER_RS485_RST_DP_PIN, 0); // P22 = 8 * 2 + 2
  _expander.writePin(EXPANDER_RS485_RST_DP_PIN, 0); // reset LCD
  _expander.writePin(EXPANDER_RS485_RST_DP_PIN, 1); // LCD out of reset

  /* Set all motor status LEDs to red. */
  for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++) {
    expander_setRed(id);
  }

  return true;
}

void BraccioClass::expander_setGreen(int const i)
{
  _expander.writePin(i * 2 - 1, 0);
  _expander.writePin(i * 2 - 2, 1);
}

void BraccioClass::expander_setRed(int const i)
{
  _expander.writePin(i * 2 - 1, 1);
  _expander.writePin(i * 2 - 2, 0);
}

bool BraccioClass::isPingAllowed()
{
  mbed::ScopedLock<rtos::Mutex> lock(_motors_connected_mtx);
  return _is_ping_allowed;
}

void BraccioClass::setMotorConnectionStatus(int const id, bool const is_connected)
{
  mbed::ScopedLock<rtos::Mutex> lock(_motors_connected_mtx);
  _is_motor_connected[SmartServoClass::idToArrayIndex(id)] = is_connected;
}

void BraccioClass::motorConnectedThreadFunc()
{
  for (;;)
  {
    if (isPingAllowed())
    {
      for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++)
      {
        bool const is_connected = (_servos.ping(id) == 0);
        setMotorConnectionStatus(id, is_connected);
      }

      for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++) {
        if (connected(id))
          expander_setGreen(id);
        else
          expander_setRed(id);
      }
    }
    delay(1000);
  }
}

bool BraccioClass::backlight_init()
{
  _bl.begin();
  if (_bl.getChipID() != 0xCE)
    return false;

  _bl.turnOn();
  return true;
}

void BraccioClass::display_init()
{
  _gfx.init();
  _gfx.setRotation(4);
  _gfx.fillScreen(TFT_WHITE);
  _gfx.setAddrWindow(0, 0, 240, 240);
}

void BraccioClass::lvgl_init()
{
  lv_init();

#if LV_USE_LOG
  lv_log_register_print_cb(lvgl_my_print);
#endif

  lv_disp_draw_buf_init(&_lvgl_disp_buf, _lvgl_draw_buf, NULL, LVGL_DRAW_BUFFER_SIZE);

  lv_disp_drv_init(&_lvgl_disp_drv);
  _lvgl_disp_drv.hor_res = 240;
  _lvgl_disp_drv.ver_res = 240;
  _lvgl_disp_drv.flush_cb = braccio_disp_flush;
  _lvgl_disp_drv.draw_buf = &_lvgl_disp_buf;
  lv_disp_drv_register(&_lvgl_disp_drv);

  lv_indev_drv_init(&_lvgl_indev_drv);
  _lvgl_indev_drv.type = LV_INDEV_TYPE_KEYPAD;
  _lvgl_indev_drv.read_cb = braccio_read_keypad;
  _lvgl_kb_indev = lv_indev_drv_register(&_lvgl_indev_drv);

  lv_style_init(&_lv_style);

  _lvgl_p_obj_group = lv_group_create();
  lv_group_set_default(_lvgl_p_obj_group);
}

void BraccioClass::display_thread_func()
{
  for(;; delay(LV_DISP_DEF_REFR_PERIOD))
  {
    mbed::ScopedLock<rtos::Mutex> lock(_display_mtx);
    lv_task_handler();
    lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
  }
}

void BraccioClass::lvgl_splashScreen(unsigned long const duration_ms)
{
  extern const lv_img_dsc_t img_bulb_gif;
  lv_obj_t * img = nullptr;

  {
    mbed::ScopedLock<rtos::Mutex> lock(_display_mtx);
    LV_IMG_DECLARE(img_bulb_gif);
    img = lv_gif_create(lv_scr_act());
    lv_gif_set_src(img, &img_bulb_gif);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
  }

  /* Wait until the splash screen duration is over. */
  for (unsigned long const start = millis(); millis() - start < duration_ms; delay(10)) { }

  {
    mbed::ScopedLock<rtos::Mutex> lock(_display_mtx);
    lv_obj_del(img);
  }
}

void BraccioClass::lvgl_pleaseConnectPower()
{
  mbed::ScopedLock<rtos::Mutex> lock(_display_mtx);

  lv_style_set_text_font(&_lv_style, &lv_font_montserrat_32);
  lv_obj_t * label1 = lv_label_create(lv_scr_act());
  lv_obj_add_style(label1, &_lv_style, 0);
  lv_label_set_text(label1, "Please\nconnect\npower.");
  lv_label_set_long_mode(label1, LV_LABEL_LONG_SCROLL);
  lv_obj_set_align(label1, LV_ALIGN_CENTER);
  lv_obj_set_pos(label1, 0, 0);
}

void BraccioClass::lvgl_defaultMenu()
{
  mbed::ScopedLock<rtos::Mutex> lock(_display_mtx);

  lv_style_set_text_font(&_lv_style, &lv_font_montserrat_32);
  lv_obj_t * label1 = lv_label_create(lv_scr_act());
  lv_obj_add_style(label1, &_lv_style, 0);
  lv_label_set_text(label1, "Braccio++");
  lv_label_set_long_mode(label1, LV_LABEL_LONG_SCROLL);
  lv_obj_set_align(label1, LV_ALIGN_CENTER);
  lv_obj_set_pos(label1, 0, 0);
}

void BraccioClass::pd_thread_func()
{
  size_t last_time_ask_pps = 0;
  unsigned long start_pd_burst = 0;
  static unsigned long const START_PD_BURST_TIMEOUT_ms = 1000;

  for(;;)
  {
    /* Wait for either a timer or a IRQ event. */
    uint32_t const flags = _pd_events.wait_any(0xFF);

    /* The actual calls to the PD library. */
    if ((millis() - last_time_ask_pps) > 5000)
    {
      start_pd_burst = millis();
      _PD_UFP.set_PPS(PPS_V(7.2), PPS_A(2.0));
      last_time_ask_pps = millis();
    }
    _PD_UFP.run();
#ifdef BRACCIO_POWER_DEBUG_ENABLE
    _PD_UFP.print_status(Serial);
#endif

    /* Set up the next time this loop is called. */
    if (flags & PD_IRQ_EVENT_FLAG)
    {
      start_pd_burst = millis();
      _pd_timer.detach();
      _pd_timer.attach(braccio_onPowerTimerEvent, 10ms);
    }

    if (flags & PD_TIMER_EVENT_FLAG)
    {
      _pd_timer.detach();
      if ((millis() - start_pd_burst) < START_PD_BURST_TIMEOUT_ms)
        _pd_timer.attach(braccio_onPowerTimerEvent, 10ms);
      else
        _pd_timer.attach(braccio_onPowerTimerEvent, 1000ms);
    }
  }
}

/**************************************************************************************
 * FUNCTION DEFINITION
 **************************************************************************************/

#if LV_USE_LOG
void lvgl_my_print(const char * dsc)
{
  Serial.println(dsc);
}
#endif /* #if LV_USE_LOG */

/* Display flushing */
extern "C" void braccio_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  Braccio.lvgl_disp_flush(disp, area, color_p);
}

/* Reading input device (simulated encoder here) */
extern "C" void braccio_read_keypad(lv_indev_drv_t * drv, lv_indev_data_t* data)
{
    static uint32_t last_key = 0;

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = Braccio.getKey();
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
        case 4:
            act_key = LV_KEY_UP;
            break;
        case 5:
            act_key = LV_KEY_DOWN;
            break;
        case 6:
            act_key = LV_KEY_HOME;
            break;
        case 1:
            act_key = LV_KEY_LEFT;
            break;
        case 2:
            act_key = LV_KEY_RIGHT;
            break;
        case 3:
            act_key = LV_KEY_ENTER;
            break;
        }

        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;
}

void braccio_onPowerIrqEvent()
{
  Braccio.onPowerIrqEvent();
}

void braccio_onPowerTimerEvent()
{
  Braccio.onPowerTimerEvent();
}
