#include "Braccio++.h"

#if LVGL_VERSION_MAJOR < 8 || (LVGL_VERSION_MAJOR == 8 && LVGL_VERSION_MINOR < 1)
#error Please use lvgl >= 8.1
#endif

#include "mbed.h"

#if LV_USE_LOG
void lvgl_my_print(const char * dsc)
{
  Serial.println(dsc);
}
#endif /* #if LV_USE_LOG */

extern "C" {
  void braccio_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  void braccio_read_keypad(lv_indev_drv_t * indev, lv_indev_data_t * data);
};

using namespace std::chrono_literals;

BraccioClass::BraccioClass()
: serial485{Serial1, 0, 7, 8} /* TX, DE, RE */
, servos{serial485}
, PD_UFP{PD_LOG_LEVEL_VERBOSE}
, _expander{TCA6424A_ADDRESS_ADDR_HIGH}
, _is_ping_allowed{true}
, _is_motor_connected{false}
, _motors_connected_mtx{}
, _motors_connected_thd{}
, _bl{}
, _gfx{}
, _lvgl_disp_drv{}
, _lvgl_indev_drv{}
, _lvgl_disp_buf{}
, _lvgl_draw_buf{}
, _lvgl_p_obj_group{nullptr}
, _lvgl_kb_indev{nullptr}
, _display_thd{}
{

}

bool BraccioClass::begin(voidFuncPtr custom_menu)
{
  Wire.begin();
  Serial.begin(115200);

  pinMode(PIN_FUSB302_INT, INPUT_PULLUP);

#ifdef __MBED__
  static rtos::Thread th(osPriorityHigh);
  th.start(mbed::callback(this, &BraccioClass::pd_thread));
  attachInterrupt(PIN_FUSB302_INT, mbed::callback(this, &BraccioClass::unlock_pd_semaphore_irq), FALLING);
  pd_timer.attach(mbed::callback(this, &BraccioClass::unlock_pd_semaphore), 10ms);
#endif

  PD_UFP.init_PPS(PPS_V(7.2), PPS_A(2.0));

/*
  while (millis() < 200) {
    PD_UFP.run();
  }
*/

  pinMode(1, INPUT_PULLUP);

  SPI.begin();

  i2c_mutex.lock();
  _bl.begin();
  if (_bl.getChipID() != 0xCE) {
    return false;
  }
  _bl.setColor(red);

  int ret = _expander.testConnection();

  if (ret == false) {
    return ret;
  }

  for (int i = 0; i < 14; i++) {
    _expander.setPinDirection(i, 0);
  }

  // Set SLEW to low
  _expander.setPinDirection(21, 0); // P25 = 8 * 2 + 5
  _expander.writePin(21, 0);

  // Set TERM to HIGH (default)
  _expander.setPinDirection(19, 0); // P23 = 8 * 2 + 3
  _expander.writePin(19, 1);

  _expander.setPinDirection(18, 0); // P22 = 8 * 2 + 2
  _expander.writePin(18, 0); // reset LCD
  _expander.writePin(18, 1); // LCD out of reset

  /* Set all motor status LEDs to red. */
  for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++) {
    setRed(id);
  }
  i2c_mutex.unlock();

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);

#if LV_USE_LOG
  lv_log_register_print_cb(lvgl_my_print);
#endif

  lv_init();

  lv_disp_draw_buf_init(&_lvgl_disp_buf, _lvgl_draw_buf, NULL, LVGL_DRAW_BUFFER_SIZE);

  /*Initialize the display*/
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

  _gfx.init();
  _gfx.setRotation(4);
  _gfx.fillScreen(TFT_WHITE);
  _gfx.setAddrWindow(0, 0, 240, 240);

  _lvgl_p_obj_group = lv_group_create();
  lv_group_set_default(_lvgl_p_obj_group);

  _display_thd.start(mbed::callback(this, &BraccioClass::display_thread_func));

  auto check_power_func = [this]()
  {
    if (!PD_UFP.is_PPS_ready())
    {
      i2c_mutex.lock();
      PD_UFP.print_status(Serial);
      PD_UFP.set_PPS(PPS_V(7.2), PPS_A(2.0));
      delay(10);
      i2c_mutex.unlock();
    }
  };

  lvgl_splashScreen(2000, check_power_func);
  lv_obj_clean(lv_scr_act());

  if (!PD_UFP.is_PPS_ready())
    lvgl_pleaseConnectPower();

  /* Loop forever, if no power is attached. */
  while(!PD_UFP.is_PPS_ready())
    check_power_func();
  lv_obj_clean(lv_scr_act());

  if (custom_menu)
    custom_menu();
  else
    lvgl_defaultMenu();

  servos.begin();
  servos.setTime(SmartServoClass::BROADCAST, SLOW);
  servos.setPositionMode(PositionMode::IMMEDIATE);

  _motors_connected_thd.start(mbed::callback(this, &BraccioClass::motorConnectedThreadFunc));

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
  Servo wrapper(servos, id);
  return wrapper;
}

Servo BraccioClass::get(int const id)
{
  return move(id);
}

void BraccioClass::moveTo(float const a1, float const a2, float const a3, float const a4, float const a5, float const a6)
{
  servos.setPositionMode(PositionMode::SYNC);
  servos.setPosition(1, a1);
  servos.setPosition(2, a2);
  servos.setPosition(3, a3);
  servos.setPosition(4, a4);
  servos.setPosition(5, a5);
  servos.setPosition(6, a6);
  servos.synchronize();
  servos.setPositionMode(PositionMode::IMMEDIATE);
}

void BraccioClass::positions(float * buffer)
{
  for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++)
    *buffer++ = servos.getPosition(id);
}

void BraccioClass::positions(float & a1, float & a2, float & a3, float & a4, float & a5, float & a6)
{
  // TODO: add check if motors are actually connected
  a1 = servos.getPosition(1);
  a2 = servos.getPosition(2);
  a3 = servos.getPosition(3);
  a4 = servos.getPosition(4);
  a5 = servos.getPosition(5);
  a6 = servos.getPosition(6);
}

void BraccioClass::connectJoystickTo(lv_obj_t* obj)
{
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

void BraccioClass::pd_thread() {
  start_pd_burst = millis();
  size_t last_time_ask_pps = 0;
  while (1) {
    auto ret = pd_events.wait_any(0xFF);
    if ((ret & 1) && (millis() - start_pd_burst > 1000)) {
      pd_timer.detach();
      pd_timer.attach(mbed::callback(this, &BraccioClass::unlock_pd_semaphore), 5s);
    }
    if (ret & 2) {
      pd_timer.detach();
      pd_timer.attach(mbed::callback(this, &BraccioClass::unlock_pd_semaphore), 50ms);
    }
    i2c_mutex.lock();
    if (millis() - last_time_ask_pps > 5000) {
      PD_UFP.set_PPS(PPS_V(7.2), PPS_A(2.0));
      last_time_ask_pps = millis();
    }
    PD_UFP.run();
    i2c_mutex.unlock();
    if (PD_UFP.is_power_ready() && PD_UFP.is_PPS_ready()) {

    }
  }
}

void BraccioClass::display_thread_func()
{
  for(;;)
  {
    lv_task_handler();
    lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
    delay(LV_DISP_DEF_REFR_PERIOD);
  }
}

#include <extra/libs/gif/lv_gif.h>

void BraccioClass::lvgl_splashScreen(unsigned long const duration_ms, std::function<void()> check_power_func)
{
  LV_IMG_DECLARE(img_bulb_gif);
  lv_obj_t* img = lv_gif_create(lv_scr_act());
  lv_gif_set_src(img, &img_bulb_gif);
  lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

  /* Wait until the splash screen duration is over.
   * Meanwhile use the wait time for checking the
   * power.
   */
  for (unsigned long const start = millis(); millis() - start < duration_ms;)
  {
    check_power_func();
  }

  lv_obj_del(img);
}

void BraccioClass::lvgl_pleaseConnectPower()
{
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
  // TODO: create a meaningful default menu
  lv_style_set_text_font(&_lv_style, &lv_font_montserrat_32);
  lv_obj_t * label1 = lv_label_create(lv_scr_act());
  lv_obj_add_style(label1, &_lv_style, 0);
  lv_label_set_text(label1, "Braccio++");
  lv_label_set_long_mode(label1, LV_LABEL_LONG_SCROLL);
  lv_obj_set_align(label1, LV_ALIGN_CENTER);
  lv_obj_set_pos(label1, 0, 0);
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
        bool const is_connected = (servos.ping(id) == 0);
        setMotorConnectionStatus(id, is_connected);
      }

      i2c_mutex.lock();
      for (int id = SmartServoClass::MIN_MOTOR_ID; id <= SmartServoClass::MAX_MOTOR_ID; id++) {
        if (connected(id))
          setGreen(id);
        else
          setRed(id);
      }
      i2c_mutex.unlock();
    }
    delay(1000);
  }
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
