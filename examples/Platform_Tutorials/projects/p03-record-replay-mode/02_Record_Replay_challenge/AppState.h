#ifndef RECORD_AND_REPLAY_APP_STATE_H_
#define RECORD_AND_REPLAY_APP_STATE_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class EventSource
{
  Button_Record, Button_Replay, Button_ZeroPosition, Button_Demo, TimerTick 
};

enum class StateName
{
  Record, Replay, Idle, Zero, Demo
};

/**************************************************************************************
 * FUNCTION DECLARATION
 **************************************************************************************/

void custom_main_menu();

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
      case EventSource::Button_Demo:         next_state = handle_OnDemo();         break;
      case EventSource::TimerTick:           next_state = handle_OnTimerTick();    break;
    }
    return next_state;
  }

protected:
  virtual State * handle_OnRecord()       { return this; }
  virtual State * handle_OnReplay()       { return this; }
  virtual State * handle_OnZeroPosition() { return this; }
  virtual State * handle_OnDemo()         { return this; }
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
  virtual State * handle_OnRecord      () override;
  virtual State * handle_OnReplay      () override;
  virtual State * handle_OnZeroPosition() override;
  virtual State * handle_OnDemo        () override;
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
           ReplayState() : _replay_cnt{0} { }
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

class DemoState : public State
{
public:
  virtual ~DemoState() { }
  virtual StateName name() override { return StateName::Demo; }
  virtual void onEnter() override;
  virtual void onExit() override;

protected:
  virtual State * handle_OnTimerTick() override;
};

class RecordAndReplayApp
{
public:
  RecordAndReplayApp()
  : _state{nullptr}
  , _mtx{}
  { }

  void enableButtons();

  void update(EventSource const evt_src)
  {
    mbed::ScopedLock<rtos::Mutex> lock(_mtx);
    
    if (!_state)
    {
      _state = new ZeroState();
      _state->onEnter();
      return;
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
  rtos::Mutex _mtx;
};

#endif /* RECORD_AND_REPLAY_APP_STATE_H_ */
