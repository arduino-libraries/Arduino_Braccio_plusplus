#ifndef APP_STATE_H_
#define APP_STATE_H_

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class EventSource
{
  Button_DownPressed, Button_DownReleased, Button_Up, Button_Left, Button_Right, Button_Enter
};

enum class StateName
{
  Shoulder, Elbow, Wrist, Pinch
};

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class State
{
public:
  virtual ~State() { }
  virtual void onEnter() { }
  virtual void onExit() { }
  virtual StateName name() = 0;
  State * update(EventSource const evt_src)
  {
    State * next_state = this;
    switch (evt_src)
    {
      case EventSource::Button_DownPressed:  next_state = handle_OnButtonDownPressed();  break;
      case EventSource::Button_DownReleased: next_state = handle_OnButtonDownReleased(); break;

      case EventSource::Button_Up:    next_state = handle_OnButtonUp();       break;
      case EventSource::Button_Left:  next_state = handle_OnButtonLeft(); break;
      case EventSource::Button_Right: next_state = handle_OnButtonRight();    break;
      case EventSource::Button_Enter: next_state = handle_OnButtonEnter();    break;
    }
    return next_state;
  }

protected:
  virtual State * handle_OnButtonDownPressed();
  virtual State * handle_OnButtonDownReleased();
  virtual State * handle_OnButtonUp()    { return this; }
  virtual State * handle_OnButtonLeft()  { return this; }
  virtual State * handle_OnButtonRight() { return this; }
  virtual State * handle_OnButtonEnter() { return this; }
};

class ShoulderState : public State
{
public:
  virtual ~ShoulderState() { }
  virtual StateName name() override { return StateName::Shoulder; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnButtonDownPressed() override;
  virtual State * handle_OnButtonDownReleased() override;
  virtual State * handle_OnButtonUp()    override;
  virtual State * handle_OnButtonLeft()  override;
  virtual State * handle_OnButtonRight() override;
  virtual State * handle_OnButtonEnter() override;
};

class ElbowState : public State
{
public:
  virtual ~ElbowState() { }
  virtual StateName name() override { return StateName::Elbow; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnButtonDownPressed() override;
  virtual State * handle_OnButtonDownReleased() override;
  virtual State * handle_OnButtonUp()    override;
  virtual State * handle_OnButtonEnter() override;
};

class WristState : public State
{
public:
  virtual ~WristState() { }
  virtual StateName name() override { return StateName::Wrist; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnButtonDownPressed() override;
  virtual State * handle_OnButtonDownReleased() override;
  virtual State * handle_OnButtonUp()    override;
  virtual State * handle_OnButtonLeft()  override;
  virtual State * handle_OnButtonRight() override;
  virtual State * handle_OnButtonEnter() override;
};

class PinchState : public State
{
public:
  virtual ~PinchState() { }
  virtual StateName name() override { return StateName::Pinch; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnButtonLeft()  override;
  virtual State * handle_OnButtonRight() override;
  virtual State * handle_OnButtonEnter() override;
};

class ControlApp
{
public:
  ControlApp()
  : _state{nullptr}
  { }

  void update(EventSource const evt_src)
  {
    if (!_state)
    {
      _state = new ShoulderState();
      _state->onEnter();
    }
    
    State * next_state = _state->update(evt_src);
      
    if (next_state->name() != _state->name())
    {
      _state->onExit();
      delete _state;
      _state = next_state;
      _state->onEnter();
    }    
  }

private:
  State * _state;
};

#endif /* APP_STATE_H_ */
