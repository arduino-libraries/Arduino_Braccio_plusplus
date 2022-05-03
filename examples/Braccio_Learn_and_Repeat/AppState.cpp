/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "AppState.h"

/**************************************************************************************
 * CONSTANT
 **************************************************************************************/

static int const SAMPLE_BUF_SIZE = 6*100*2; /* 20 seconds. */
static float const HOME_POS[6] = {157.5, 157.5, 157.5, 157.5, 157.5, 90.0};


/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

extern const char * btnm_map[];
extern lv_obj_t * counter;
extern lv_obj_t * btnm;

static float sample_buf[SAMPLE_BUF_SIZE];
static int   sample_cnt;

IdleState   LearnAndRepeatApp::_idle_state;
RecordState LearnAndRepeatApp::_record_state;
ReplayState LearnAndRepeatApp::_replay_state;
ZeroState   LearnAndRepeatApp::_zero_state;

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
