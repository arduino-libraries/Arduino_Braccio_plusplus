/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>

#include "AppState.h"

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

extern lv_obj_t * btnm;
LearnAndRepeatApp app;

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  Braccio.begin(custom_main_menu);

  /* Enable buttons once init is complete. */
  lv_btnmatrix_clear_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_clear_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_DISABLED);
}

void loop()
{
  /* Only execute every 100 ms. */
  static auto prev = millis();
  auto const now = millis();

  if ((now - prev) > 100)
  {
    prev = now;
    app.update(EventSource::TimerTick);
  }
}
