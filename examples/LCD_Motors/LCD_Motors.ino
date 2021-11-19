#include "Braccio++.h"

int selected_motor = 0;

static void event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    const char * txt = lv_btnmatrix_get_btn_text(obj, id);

    LV_LOG_USER("%s was pressed\n", txt);
    Serial.println(txt);
    if (strcmp(txt, "Motor 1") == 0) {
      selected_motor = 1;
      Braccio.ping_allowed = false;
    } else if (strcmp(txt, "Motor 2") == 0) {
      selected_motor = 2;
      Braccio.ping_allowed = false;
    } else if (strcmp(txt, "Motor 3") == 0) {
      selected_motor = 3;
      Braccio.ping_allowed = false;
    } else if (strcmp(txt, "Motor 4") == 0) {
      selected_motor = 4;
      Braccio.ping_allowed = false;
    } else if (strcmp(txt, "Motor 5") == 0) {
      selected_motor = 5;
      Braccio.ping_allowed = false;
    } else if (strcmp(txt, "Motor 6") == 0) {
      selected_motor = 6;
      Braccio.ping_allowed = false;
    } else {
      Braccio.ping_allowed = true;
      selected_motor = 0;
    }
  }
}


static const char * btnm_map[] = {"Motor 1", "Motor 2", "\n",
                                  "Motor 3", "Motor 4", "\n",
                                  "Motor 5", "Motor 6", "\n", "\0"
                                 };

void customMenu() {
  lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
  lv_btnmatrix_set_map(btnm1, btnm_map);
  lv_btnmatrix_set_btn_ctrl(btnm1, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 2, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 3, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 4, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 5, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(btnm1, event_handler, LV_EVENT_ALL, NULL);
  Braccio.connectJoystickTo(btnm1);
}

void setup() {
  // Call Braccio.begin() for default menu or pass a function for custom menu
  Braccio.begin(customMenu);
  Serial.begin(115200);
}

void loop() {
  for (float angle = 0.0; angle <= 180.0; angle+=10.0){
    Braccio.move(selected_motor).to(angle);
    delay(500);
  }
}
