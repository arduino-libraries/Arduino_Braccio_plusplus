/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "AppState.h"

/**************************************************************************************
 * DEFINE
 **************************************************************************************/

#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2
#define COLOR_ORANGE     0xE47128

#define BUTTON_ENTER     6

/**************************************************************************************
 * CONSTANT
 **************************************************************************************/

static int const SAMPLE_BUF_SIZE = 6*200*2; /* 20 seconds. */
static float const HOME_POS[6] = {157.5, 157.5, 157.5, 157.5, 157.5,  90.0};
static float const WAVE_POS[6] = {180.0, 260.0, 157.5, 157.5, 157.5, 180.0};

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

lv_obj_t * counter;
lv_obj_t * btnm;
lv_obj_t * menu_screen = nullptr;
lv_obj_t * stop_screen = nullptr;
const char * btnm_map[] = { "RECORD", "\n", "REPLAY", "\n", "ZERO_POSITION", "\n", "DEMO", "\n", "\0" };

static float sample_buf[SAMPLE_BUF_SIZE];
static int   sample_cnt;

extern RecordAndReplayApp app;

/**************************************************************************************
 * FUNCTION DEFINITION
 **************************************************************************************/

static void event_handler_menu(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED || (code == LV_EVENT_KEY && Braccio.getKey() == BUTTON_ENTER))
  {
    lv_obj_t * obj = lv_event_get_target(e);
    uint32_t const id = lv_btnmatrix_get_selected_btn(obj);

    switch (id)
    {
    case 0: app.update(EventSource::Button_Record);       break;
    case 1: app.update(EventSource::Button_Replay);       break;
    case 2: app.update(EventSource::Button_ZeroPosition); break;  
    case 3: app.update(EventSource::Button_Demo);         break;  
    }
  }
}

void custom_main_menu()
{
  Braccio.lvgl_lock();
  static lv_style_t style_focus;
  lv_style_init(&style_focus);
  lv_style_set_outline_color(&style_focus, lv_color_hex(COLOR_ORANGE));
  lv_style_set_outline_width(&style_focus, 4);

  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(COLOR_LIGHT_TEAL));
  lv_style_set_text_color(&style_btn, lv_color_white());

  btnm = lv_btnmatrix_create(lv_scr_act());
  lv_obj_set_size(btnm, 240, 240);
  lv_btnmatrix_set_map(btnm, btnm_map);
  lv_obj_align(btnm, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_style(btnm, &style_btn, LV_PART_ITEMS);
  lv_obj_add_style(btnm, &style_focus, LV_PART_ITEMS | LV_STATE_FOCUS_KEY);

  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);

  lv_btnmatrix_set_one_checked(btnm, true);
  lv_btnmatrix_set_selected_btn(btnm, 0);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);

  counter = lv_label_create(btnm);
  lv_label_set_text_fmt(counter, "Counter: %d" , 0);
  lv_obj_align(counter, LV_ALIGN_CENTER, 0, 80);

  lv_obj_add_event_cb(btnm, event_handler_menu, LV_EVENT_ALL, NULL);

  /* Store a pointer to the screen in the menu_screen variable. */
  menu_screen = lv_scr_act();

  /* Create a separate screen for the stop sign image. */
  LV_IMG_DECLARE(stop_sign);
  stop_screen = lv_img_create(NULL);
  lv_img_set_src(stop_screen, &stop_sign);
  lv_obj_align(stop_screen, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(stop_screen, 240, 240);

  Braccio.lvgl_unlock();

  Braccio.connectJoystickTo(btnm);
}

/**************************************************************************************
 * IdleState
 **************************************************************************************/

void IdleState::onEnter()
{
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
}

void IdleState::onExit()
{
  lv_btnmatrix_clear_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
}

State * IdleState::handle_OnRecord()
{
  return new RecordState();
}

State * IdleState::handle_OnReplay()
{
  return new ReplayState();
}

State * IdleState::handle_OnZeroPosition()
{
  return new ZeroState();
}

State * IdleState::handle_OnDemo()
{
  return new DemoState();
}

/**************************************************************************************
 * RecordState
 **************************************************************************************/

void RecordState::onEnter()
{
  btnm_map[0] = "STOP";
  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKED);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);

  delay(100);
  Braccio.disengage();
  delay(100);
  sample_cnt = 0;
}

void RecordState::onExit()
{
  btnm_map[0] = "RECORD";
  lv_btnmatrix_clear_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);
  lv_label_set_text_fmt(counter, "Counter: %d" , 0);

  delay(100);
  Braccio.engage();
  delay(100);
}

State * RecordState::handle_OnRecord()
{ 
  return new IdleState();
}

State * RecordState::handle_OnTimerTick()
{
  /* The sample buffer is full. */
  if (sample_cnt >= SAMPLE_BUF_SIZE) {
    return new IdleState();
  }

  /* We still have space, let's sample some data. */
  Braccio.positions(sample_buf + sample_cnt);

  /* Check if any of the servos reports a position of
   * 0.0 degrees. In this case we've entered the dead
   * zone and should display a warning on the screen.
   **/
  int const count = std::count_if(sample_buf + sample_cnt,
                                  sample_buf + sample_cnt + 6,
                                  [](float const v)
                                  {
                                    float const EPSILON = 0.01;
                                    return (fabs(v) < EPSILON);
                                  });
  if (count > 0) {
    if (lv_scr_act() != stop_screen)
     lv_scr_load(stop_screen);
  }
  else {
    sample_cnt += 6;
    /* Reload the menu screen if it has not been currently loaded. */
    if (lv_scr_act() != menu_screen)
      lv_scr_load(menu_screen);
  }

  /* Update sample counter. */
  lv_label_set_text_fmt(counter, "Counter: %d" , (sample_cnt / 6));

  return this;
}

/**************************************************************************************
 * ReplayState
 **************************************************************************************/

void ReplayState::onEnter()
{
  btnm_map[2] = "STOP";
  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKED);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);
}
 
void ReplayState::onExit()
{
  btnm_map[2] = "REPLAY";
  lv_btnmatrix_clear_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);
  lv_label_set_text_fmt(counter, "Counter: %d" , 0);
}

State * ReplayState::handle_OnReplay()
{
  return new IdleState();
}

State * ReplayState::handle_OnTimerTick()
{
  /* All samples have been replayed. */
  if (_replay_cnt >= sample_cnt) {
    return new IdleState();
  }

  /* Replay recorded movements. */
  Braccio.moveTo(sample_buf[_replay_cnt + 0],
                 sample_buf[_replay_cnt + 1],
                 sample_buf[_replay_cnt + 2],
                 sample_buf[_replay_cnt + 3],
                 sample_buf[_replay_cnt + 4],
                 sample_buf[_replay_cnt + 5]);
  _replay_cnt += 6;
 
  lv_label_set_text_fmt(counter, "Counter: %d" , (_replay_cnt / 6));

  return this;
}

/**************************************************************************************
 * ZeroState
 **************************************************************************************/

State * ZeroState::handle_OnTimerTick()
{
  return new IdleState();
}

void ZeroState::onEnter()
{
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);

  delay(100);
  Braccio.engage();
  delay(100);
  Braccio.moveTo(HOME_POS[0], HOME_POS[1], HOME_POS[2], HOME_POS[3], HOME_POS[4], HOME_POS[5]);

  auto isInHomePos = []() -> bool
  {
    float current_angles[SmartServoClass::NUM_MOTORS] = {0};
    Braccio.positions(current_angles);

    float total_angle_err = 0.0;
    for (size_t i = 0; i < SmartServoClass::NUM_MOTORS; i++)
      total_angle_err += fabs(current_angles[i] - HOME_POS[i]);

    static float const TOTAL_EPSILON = 10.0f;
    bool const is_in_home_pos = (total_angle_err < TOTAL_EPSILON);
    return is_in_home_pos;
  };
  auto isTimeout = [](unsigned long const start) -> bool
  {
    /* Timeout of one second. */
    auto const now = millis();
    if ((now - start) > 1000)
      return true;
    else
      return false;
  };

  /* Wait until we've returned to the home position
   * with a timeout (i.e. we leave this function)
   * after one second even if we can't fully reach
   * the home position.
   */
  for(auto start = millis(); !isInHomePos() && !isTimeout(start); delay(100)) { }
}
 
void ZeroState::onExit()
{
  lv_btnmatrix_clear_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
}

/**************************************************************************************
 * DemoState
 **************************************************************************************/

State * DemoState::handle_OnTimerTick()
{
  return new IdleState();
}

void DemoState::onEnter()
{
  delay(100);
  Braccio.engage();
  delay(100);
  Braccio.moveTo(WAVE_POS[0], WAVE_POS[1], WAVE_POS[2], WAVE_POS[3], WAVE_POS[4], WAVE_POS[5]);
  delay(500);

  for (int i = 1; i <= 3; i++) 
  {
    Braccio.moveTo(WAVE_POS[0], WAVE_POS[1], 130.0, WAVE_POS[3], WAVE_POS[4], WAVE_POS[5]);
    delay(300);

    Braccio.moveTo(WAVE_POS[0], WAVE_POS[1], 195.0, WAVE_POS[3], WAVE_POS[4], WAVE_POS[5]);
    delay(600);

    Braccio.moveTo(WAVE_POS[0], WAVE_POS[1], WAVE_POS[2], WAVE_POS[3], WAVE_POS[4], WAVE_POS[5]);
    delay(300);
  }
    
  auto isInDemoPos = []() -> bool
  {
    float current_angles[SmartServoClass::NUM_MOTORS] = {0};
    Braccio.positions(current_angles);

    float total_angle_err = 0.0;
    for (size_t i = 0; i < SmartServoClass::NUM_MOTORS; i++)
      total_angle_err += fabs(current_angles[i] - WAVE_POS[i]);

    static float const TOTAL_EPSILON = 10.0f;
    bool const is_in_demo_pos = (total_angle_err < TOTAL_EPSILON);
    return is_in_demo_pos;
  };
  auto isTimeout = [](unsigned long const start) -> bool
  {
    /* Timeout of one second. */
    auto const now = millis();
    if ((now - start) > 1000)
      return true;
    else
      return false;
  };

  /* Wait until we've returned to the Demo position
   * with a timeout (i.e. we leave this function)
   * after one second even if we can't fully reach
   * the demo position.
   */
  for(auto start = millis(); !isInDemoPos() && !isTimeout(start); delay(100)) { }
}
 
void DemoState::onExit()
{
  lv_btnmatrix_clear_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_CHECKED);
}

/**************************************************************************************
 * RecordAndReplayApp
 **************************************************************************************/

void RecordAndReplayApp::enableButtons()
{
  /* Enable buttons once init is complete. */
  lv_btnmatrix_clear_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);
}