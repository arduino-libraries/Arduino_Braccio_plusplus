/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "AppState.h"

#include <Braccio++.h>

extern lv_obj_t * label;
extern lv_obj_t * direction_btnm;

enum BUTTONS {
  BTN_UP = 1,
  BTN_DOWN = 7,
  BTN_LEFT = 3,
  BTN_RIGHT = 5,
};

/**************************************************************************************
 * State
 **************************************************************************************/

State * State::handle_OnButtonDownPressed()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
  return this;
}

State * State::handle_OnButtonDownReleased()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
  return this;
}

/**************************************************************************************
 * ShoulderState
 **************************************************************************************/

void ShoulderState::onEnter()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Shoulder");
  Braccio.lvgl_unlock();
}

void ShoulderState::onExit()
{

}

State * ShoulderState::handle_OnButtonDownPressed()
{
  State::handle_OnButtonDownPressed();
  return this;
}

State * ShoulderState::handle_OnButtonDownReleased()
{
  State::handle_OnButtonDownReleased();
  return this;
}

State * ShoulderState::handle_OnButtonUp()
{
  return this;
}

State * ShoulderState::handle_OnButtonLeft()
{
  return this;
}

State * ShoulderState::handle_OnButtonRight()
{
  return this;
}

State * ShoulderState::handle_OnButtonEnter()
{
  return new ElbowState();
}

/**************************************************************************************
 * ElbowState
 **************************************************************************************/

void ElbowState::onEnter()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Elbow");
  Braccio.lvgl_unlock();
}

void ElbowState::onExit()
{

}

State * ElbowState::handle_OnButtonDownPressed()
{
  State::handle_OnButtonDownPressed();
  return this;
}

State * ElbowState::handle_OnButtonDownReleased()
{
  State::handle_OnButtonDownReleased();
  return this;
}

State * ElbowState::handle_OnButtonUp()
{
  return new WristState();
}

State * ElbowState::handle_OnButtonEnter()
{
  return new WristState();
}

/**************************************************************************************
 * WristState
 **************************************************************************************/

void WristState::onEnter()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Wrist");
  Braccio.lvgl_unlock();
}

void WristState::onExit()
{

}

State * WristState::handle_OnButtonDownPressed()
{
  State::handle_OnButtonDownPressed();
  return this;
}

State * WristState::handle_OnButtonDownReleased()
{
  State::handle_OnButtonDownReleased();
  return this;
}

State * WristState::handle_OnButtonUp()
{
  return this;
}

State * WristState::handle_OnButtonLeft()
{
  return this;
}

State * WristState::handle_OnButtonRight()
{
  return this;
}

State * WristState::handle_OnButtonEnter()
{
  return new PinchState();
}

/**************************************************************************************
 * PinchState
 **************************************************************************************/

void PinchState::onEnter()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Pinch");
  Braccio.lvgl_unlock();
}

void PinchState::onExit()
{

}

State * PinchState::handle_OnButtonLeft()
{
  return this;
}

State * PinchState::handle_OnButtonRight()
{
  return this;
}

State * PinchState::handle_OnButtonEnter()
{
  return new ShoulderState();
}
