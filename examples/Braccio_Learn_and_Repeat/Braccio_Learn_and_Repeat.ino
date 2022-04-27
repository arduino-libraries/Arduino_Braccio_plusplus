#include <Braccio++.h>

// Colors
#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2
#define COLOR_ORANGE     0xE47128

enum states {
  RECORD,
  REPLAY,
  ZERO_POSITION
};

int state = ZERO_POSITION;

float values[10000];
float* idx = values;
float* final_idx = 0;
float homePos[6] = {157.5, 157.5, 157.5, 157.5, 157.5, 90.0};

static lv_obj_t * counter;
static lv_obj_t * btnm;

static const char * btnm_map[] = { "RECORD", "\n", "REPLAY", "\n", "ZERO_POSITION", "\n", "\0" };


static void eventHandlerMenu(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);

  if (code == LV_EVENT_KEY && lv_indev_get_key(lv_indev_get_act()) == LV_KEY_HOME) {
    state = ZERO_POSITION;
    return;
  }

  if (code == LV_EVENT_CLICKED) {
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    const char * txt = lv_btnmatrix_get_btn_text(obj, id);

    if (state == RECORD) {
      final_idx = idx;
    }

    idx = values;

    switch (id) {
      case 0: // if the button pressed is the first one
        if (txt == "RECORD") {
          state = RECORD;
          Braccio.disengage(); // allow the user to freely move the braccio
          lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKED);
          Serial.println("RECORD");
          lv_btnmatrix_clear_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED); // remove disabled state of the replay button
          btnm_map[0] = "STOP"; // change the label of the first button to "STOP"
        }
        else if (txt == "STOP") {
          state = ZERO_POSITION;
          Braccio.engage(); // enable the steppers so that the braccio stands still
          lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
          btnm_map[0] = "RECORD"; // reset the label of the first button back to "RECORD"
        }
        break;
      case 1:
        btnm_map[0] = "RECORD"; // reset the label of the first button back to "RECORD"
        if (txt == "REPLAY"){
        state = REPLAY;
        btnm_map[2] = "STOP"; // change the label of the second button to "STOP"
        Braccio.engage();
        lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKED);
        Serial.println("REPLAY");
        }
        else if (txt=="STOP"){
          state = ZERO_POSITION;
          Braccio.engage(); // enable the steppers so that the braccio stands still
          lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
          btnm_map[2] = "REPLAY"; // reset the label of the first button back to "REPLAY"
        }
        
        break;
      
      default:
        state = ZERO_POSITION;
        btnm_map[0] = "RECORD"; // reset the label of the first button back to "RECORD"
        btnm_map[2] = "REPLAY"; // reset the label of the first button back to "REPLAY"
        Braccio.engage();
        delay(500);
        Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
        lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
        Serial.println("ZERO_POSITION");
        break;
    }
  }
}

void mainMenu() {
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

  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKABLE);

  lv_btnmatrix_set_one_checked(btnm, true);
  lv_btnmatrix_set_selected_btn(btnm, 0);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);

  counter = lv_label_create(btnm);
  lv_label_set_text_fmt(counter, "Counter: %d" , 0);
  lv_obj_align(counter, LV_ALIGN_CENTER, 0, 80);

  lv_obj_add_event_cb(btnm, eventHandlerMenu, LV_EVENT_ALL, NULL);

  Braccio.connectJoystickTo(btnm);
}

void setup() {
  Braccio.begin(mainMenu);
  delay(500);

  Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
  delay(500);
  
  Serial.begin(115200);
  Serial.println("Replicate a movement");
}

void loop() {
  if (state == RECORD) {
    Braccio.positions(idx);
    idx += 6;
  }
  if (state == REPLAY) {
    Braccio.moveTo(idx[0], idx[1], idx[2], idx[3], idx[4], idx[5]);
    idx += 6;
    if (idx >= final_idx) {
      Serial.println("REPLAY done");
      state = ZERO_POSITION;
      btnm_map[2] = "REPLAY"; // reset the label of the first button back to "REPLAY"
      lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
    }
  }
  if (idx - values >= sizeof(values)) {
    Serial.println("ZERO_POSITION");
    state = ZERO_POSITION;
  }
  delay(100);
  if (state != ZERO_POSITION) {
    lv_label_set_text_fmt(counter, "Counter: %d" , idx - values);
  }
}
