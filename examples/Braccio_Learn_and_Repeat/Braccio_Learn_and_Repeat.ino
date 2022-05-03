/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>

#include "AppState.h"

/**************************************************************************************
 * DEFINE
 **************************************************************************************/

#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2
#define COLOR_ORANGE     0xE47128

#define BUTTON_ENTER     6

/**************************************************************************************
 * GLOBAL CONST
 **************************************************************************************/

static float const homePos[6] = {157.5, 157.5, 157.5, 157.5, 157.5, 90.0};

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

lv_obj_t * counter;
lv_obj_t * btnm;

const char * btnm_map[] = { "RECORD", "\n", "REPLAY", "\n", "ZERO_POSITION", "\n", "\0" };

static LearnAndRepeatApp app;

/**************************************************************************************
 * FUNCTIONS
 **************************************************************************************/

static void eventHandlerMenu(lv_event_t * e)
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

  lv_obj_add_event_cb(btnm, eventHandlerMenu, LV_EVENT_ALL, NULL);
  Braccio.lvgl_unlock();

  Braccio.connectJoystickTo(btnm);
}

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
