/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "AppState.h"

#include <Braccio++.h>

/**************************************************************************************
 * EXTERN
 **************************************************************************************/

extern lv_obj_t * label;

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

static auto gripper    = Braccio.get(1);
static auto wristRoll  = Braccio.get(2);
static auto wristPitch = Braccio.get(3);
static auto elbow      = Braccio.get(4);
static auto shoulder   = Braccio.get(5);
static auto base       = Braccio.get(6);

/**************************************************************************************
 * ShoulderState
 **************************************************************************************/

ShoulderState::ShoulderState()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Shoulder");
  Braccio.lvgl_unlock();
}

State * ShoulderState::handle_OnEnter()
{
  return new ElbowState();
}

State * ShoulderState::handle_OnUp()
{
  return this;
}

State * ShoulderState::handle_OnDown()
{
  return this;
}

State * ShoulderState::handle_OnLeft()
{
  return this;
}

State * ShoulderState::handle_OnRight()
{
  return this;
}

/**************************************************************************************
 * ElbowState
 **************************************************************************************/

ElbowState::ElbowState()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Elbow");
  Braccio.lvgl_unlock();
}

State * ElbowState::handle_OnEnter()
{
  return new WristState();
}

State * ElbowState::handle_OnUp()
{
  return this;
}

State * ElbowState::handle_OnDown()
{
  return this;
}

/**************************************************************************************
 * WristState
 **************************************************************************************/

WristState::WristState()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Wrist");
  Braccio.lvgl_unlock();
}

State * WristState::handle_OnEnter()
{
  return new PinchState();
}

State * WristState::handle_OnUp()
{
  return this;
}

State * WristState::handle_OnDown()
{
  return this;
}

State * WristState::handle_OnLeft()
{
  return this;
}

State * WristState::handle_OnRight()
{
  return this;
}

/**************************************************************************************
 * PinchState
 **************************************************************************************/

PinchState::PinchState()
{
  Braccio.lvgl_lock();
  lv_label_set_text(label, "Pinch");
  Braccio.lvgl_unlock();
}

State * PinchState::handle_OnEnter()
{
  return new ShoulderState();
}

State * PinchState::handle_OnLeft()
{
  gripper.move().to(gripper.position() + 10.0f).in(10ms);
  return this;
}

State * PinchState::handle_OnRight()
{
  gripper.move().to(gripper.position() - 10.0f).in(10ms);
  return this;
}
