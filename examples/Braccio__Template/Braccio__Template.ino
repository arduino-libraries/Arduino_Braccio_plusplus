#include "Braccio++.h"

int demo_mode = 0;

static void event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    const char * txt = lv_btnmatrix_get_btn_text(obj, id);

    LV_LOG_USER("%s was pressed\n", txt);
    Serial.println(txt);
    if (strcmp(txt, "Demo") == 0) {
      demo_mode = 1;
    } else {
      demo_mode = 0;
    }
  }
}


static const char * btnm_map[] = {"Demo", "Learn", "\n",
                                  "Repeat", "Unlock", "\n",
                                 };

void customMenu() {
  lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
  lv_btnmatrix_set_map(btnm1, btnm_map);
  lv_btnmatrix_set_btn_ctrl(btnm1, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 2, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 3, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(btnm1, event_handler, LV_EVENT_ALL, NULL);
  Braccio.connectJoystickTo(btnm1);
}

void setup() {
  // Call Braccio.begin() for default menu or pass a function for custom menu
  Braccio.begin(customMenu);
}

void loop() {
  if (demo_mode) {
    Braccio.move(4).to(20.0f);
    delay(1000);
    Braccio.move(4).to(10.0f);
    delay(1000);
  }
}
