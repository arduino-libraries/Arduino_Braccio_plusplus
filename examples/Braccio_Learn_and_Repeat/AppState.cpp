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

static int const SAMPLE_BUF_SIZE = 6*100*2; /* 20 seconds. */
static float const HOME_POS[6] = {157.5, 157.5, 157.5, 157.5, 157.5, 90.0};

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

lv_obj_t * counter;
lv_obj_t * btnm;
const char * btnm_map[] = { "RECORD", "\n", "REPLAY", "\n", "ZERO_POSITION", "\n", "\0" };

static float sample_buf[SAMPLE_BUF_SIZE];
static int   sample_cnt;

IdleState   LearnAndRepeatApp::_idle_state;
RecordState LearnAndRepeatApp::_record_state;
ReplayState LearnAndRepeatApp::_replay_state;
ZeroState   LearnAndRepeatApp::_zero_state;

extern LearnAndRepeatApp app;

/**************************************************************************************
 * FUNCTION DEFINITION
 **************************************************************************************/

static void event_handler_menu(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);

  //if (code == LV_EVENT_CLICKED || (code == LV_EVENT_KEY && Braccio.getKey() == BUTTON_ENTER))
  if (code == LV_EVENT_KEY && Braccio.getKey() == BUTTON_ENTER)
  {
    lv_obj_t * obj = lv_event_get_target(e);
    uint32_t const id = lv_btnmatrix_get_selected_btn(obj);

    switch (id)
    {
    case 0: app.update(EventSource::Button_Record);       break;
    case 1: app.update(EventSource::Button_Replay);       break;
    case 2: app.update(EventSource::Button_ZeroPosition); break;  
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

  lv_btnmatrix_set_one_checked(btnm, true);
  lv_btnmatrix_set_selected_btn(btnm, 0);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);

  counter = lv_label_create(btnm);
  lv_label_set_text_fmt(counter, "Counter: %d" , 0);
  lv_obj_align(counter, LV_ALIGN_CENTER, 0, 80);

  lv_obj_add_event_cb(btnm, event_handler_menu, LV_EVENT_ALL, NULL);
  Braccio.lvgl_unlock();

  Braccio.connectJoystickTo(btnm);
}

/**************************************************************************************
 * State
 **************************************************************************************/

State * State::handle_OnZeroPosition()
{
  return &LearnAndRepeatApp::_zero_state;
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
  return &LearnAndRepeatApp::_record_state;
}

State * IdleState::handle_OnReplay()
{
  return &LearnAndRepeatApp::_replay_state;
}

/**************************************************************************************
 * RecordState
 **************************************************************************************/

void RecordState::onEnter()
{
  btnm_map[0] = "STOP";
  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKED);

  Braccio.disengage();
  sample_cnt = 0;
}

void RecordState::onExit()
{
  btnm_map[0] = "RECORD";
  lv_btnmatrix_clear_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_label_set_text_fmt(counter, "Counter: %d" , 0);

  Braccio.engage();
}

State * RecordState::handle_OnRecord()
{ 
  return &LearnAndRepeatApp::_idle_state;
}

State * RecordState::handle_OnTimerTick()
{
  /* The sample buffer is full. */
  if (sample_cnt >= SAMPLE_BUF_SIZE) {
    return &LearnAndRepeatApp::_idle_state;
  }

  /* We still have space, let's sample some data. */
  Braccio.positions(sample_buf + sample_cnt);
  sample_cnt += 6;

  /* Update sample counter. */
  lv_label_set_text_fmt(counter, "Counter: %d" , (sample_cnt / 6));

  return this;
}

/**************************************************************************************
 * ReplayState
 **************************************************************************************/

void ReplayState::onEnter()
{
  _replay_cnt = 0;

  btnm_map[2] = "STOP";
  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKED);
}
 
void ReplayState::onExit()
{
  btnm_map[2] = "REPLAY";
  lv_btnmatrix_clear_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKED);
  lv_label_set_text_fmt(counter, "Counter: %d" , 0);
}

State * ReplayState::handle_OnReplay()
{
  return &LearnAndRepeatApp::_idle_state;
}

State * ReplayState::handle_OnTimerTick()
{
  /* All samples have been replayed. */
  if (_replay_cnt >= sample_cnt) {
    return &LearnAndRepeatApp::_idle_state;
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
 * ReplayState
 **************************************************************************************/

State * ZeroState::handle_OnTimerTick()
{
  return &LearnAndRepeatApp::_idle_state;
}

void ZeroState::onEnter()
{
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);

  Braccio.engage();
  delay(100);
  Braccio.moveTo(HOME_POS[0], HOME_POS[1], HOME_POS[2], HOME_POS[3], HOME_POS[4], HOME_POS[5]);
  delay(500);
}
 
void ZeroState::onExit()
{
  lv_btnmatrix_clear_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
}
