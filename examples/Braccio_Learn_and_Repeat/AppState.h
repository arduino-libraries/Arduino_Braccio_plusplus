#ifndef LEARN_AND_REPEAT_APP_STATE_H_
#define LEARN_AND_REPEAT_APP_STATE_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class EventSource
{
  Button_Record, Button_Replay, Button_ZeroPosition, TimerTick
};

enum class StateName
{
  Record, Replay, Idle, Zero
};

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class State
{
public:
  virtual ~State() { }
  virtual void onEnter() { Serial.println("State::onEnter"); }
  virtual void onExit() { Serial.println("State::onExit"); }
  virtual StateName name() = 0;
  State * update(EventSource const evt_src)
  {
    State * next_state = this;
    switch (evt_src)
    {
      case EventSource::Button_Record:       next_state = handle_OnRecord();       break;
      case EventSource::Button_Replay:       next_state = handle_OnReplay();       break;
      case EventSource::Button_ZeroPosition: next_state = handle_OnZeroPosition(); break;
      case EventSource::TimerTick:           next_state = handle_OnTimerTick();    break;
    }
    return next_state;
  }

protected:
  virtual State * handle_OnRecord()       { return this; }
  virtual State * handle_OnReplay()       { return this; }
  virtual State * handle_OnZeroPosition();
  virtual State * handle_OnTimerTick()    { return this; }
};

class IdleState : public State
{
public:
  virtual ~IdleState() { }
  virtual StateName name() override { return StateName::Idle; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnRecord() override;
  virtual State * handle_OnReplay() override;
};

class RecordState : public State
{
public:
  virtual ~RecordState() { }
  virtual StateName name() override { return StateName::Record; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnRecord() override;
  virtual State * handle_OnTimerTick() override;
};

class ReplayState : public State
{
public:
  virtual ~ReplayState() { }
  virtual StateName name() override { return StateName::Replay; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnReplay() override;
  virtual State * handle_OnTimerTick() override;

private:
  int _replay_cnt;
};

class ZeroState : public State
{
public:
  virtual ~ZeroState() { }
  virtual StateName name() override { return StateName::Zero; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnTimerTick() override;
};

class LearnAndRepeatApp
{
public:
  LearnAndRepeatApp()
  : _state{nullptr}
  , _mtx{}
  { }

  static IdleState   _idle_state;
  static RecordState _record_state;
  static ReplayState _replay_state;
  static ZeroState   _zero_state;

  void update(EventSource const evt_src)
  {
    mbed::ScopedLock<rtos::Mutex> lock(_mtx);
    
    if (!_state)
    {
      _state = &_zero_state;
      _state->onEnter();
      return;
    }
    
    State * next_state = _state->update(evt_src);
      
    if (next_state->name() != _state->name())
    {
      _state->onExit();
      _state = next_state;
      _state->onEnter();
    }    
  }

private:
  State * _state;
  rtos::Mutex _mtx;
};

#endif /* LEARN_AND_REPEAT_APP_STATE_H_ */
