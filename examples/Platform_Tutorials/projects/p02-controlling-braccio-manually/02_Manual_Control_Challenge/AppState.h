#ifndef APP_STATE_H_
#define APP_STATE_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <mbed.h>

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class Button
{
  None, Enter, Home, Up, Down, Left, Right
};

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class State
{
public:
  virtual ~State() { }
  State * update(Button const button)
  {
    State * next_state = this;
    switch (button)
    {
      case Button::Home:
      case Button::Enter: next_state = handle_OnEnter(); break;
      case Button::Up:    next_state = handle_OnUp();    break;
      case Button::Down:  next_state = handle_OnDown();  break;
      case Button::Left:  next_state = handle_OnLeft();  break;
      case Button::Right: next_state = handle_OnRight(); break;
      default:                                           break;
    }
    return next_state;
  }

protected:
  virtual State * handle_OnEnter() = 0;
  virtual State * handle_OnUp   () { return this; }
  virtual State * handle_OnDown () { return this; }
  virtual State * handle_OnLeft () { return this; }
  virtual State * handle_OnRight() { return this; }
};

class ShoulderState : public State
{
public:
           ShoulderState();
  virtual ~ShoulderState() { }
protected:
  virtual State * handle_OnEnter() override;
  virtual State * handle_OnUp   () override;
  virtual State * handle_OnDown () override;
  virtual State * handle_OnLeft () override;
  virtual State * handle_OnRight() override;
};


class ElbowState : public State
{
public:
           ElbowState();
  virtual ~ElbowState();
protected:
  virtual State * handle_OnEnter() override;
  virtual State * handle_OnUp   () override;
  virtual State * handle_OnDown () override;
};

class WristState : public State
{
public:
           WristState();
  virtual ~WristState() { }
protected:
  virtual State * handle_OnEnter() override;
  virtual State * handle_OnUp   () override;
  virtual State * handle_OnDown () override;
  virtual State * handle_OnLeft () override;
  virtual State * handle_OnRight() override;
};


class PinchState : public State
{
public:
           PinchState();
  virtual ~PinchState();
protected:
  virtual State * handle_OnEnter() override;
  virtual State * handle_OnLeft () override;
  virtual State * handle_OnRight() override;
};

class ManualControlApp
{
public:
  ManualControlApp()
  : _state{nullptr}
  , _mtx{}
  { }

  void update(Button const button)
  {
    mbed::ScopedLock<rtos::Mutex> lock(_mtx);
    
    if (!_state)
    {
      _state = new PinchState();
      return;
    }
    
    State * next_state = _state->update(button);
      
    if (next_state != _state) {
      delete _state;
      _state = next_state;
    }    
  }

private:
  State * _state;
  rtos::Mutex _mtx;
};

#endif /* APP_STATE_H_ */
