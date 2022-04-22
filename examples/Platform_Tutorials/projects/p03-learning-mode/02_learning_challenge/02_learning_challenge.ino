#include <Braccio++.h>

// Colors
#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2

auto wristPitch  = Braccio.get(3);

enum states {
  LEARN,
  REPEAT,
  IDLE,
  DEMO
};

int state = IDLE;

float values[1000];
float* idx = values;
float* final_idx = 0;
float homePos[6] = {160.0, 150.0, 220.0, 220.0, 100.0, 180.0};
float wavePos[6] = {180.0, 250.0, 145.0, 150.0, 150.0, 90.0};

static lv_obj_t * counter;
static lv_obj_t * btnm;

static const char * btnm_map[] = { "Learn", "\n", "Replay", "\n", "Idle", "\n", "Demo", "\n", "\0" };


void ciaoMovement() {
  for (int i = 1; i <= 10; i++) {
    wristPitch.move().to(100.0f);
    delay(300);
    wristPitch.move().to(190.0f);
    delay(600);
    wristPitch.move().to(145.0f);
    delay(300);
  }

  Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
}


static void eventHandlerMenu(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);

  if (code == LV_EVENT_KEY && lv_indev_get_key(lv_indev_get_act()) == LV_KEY_HOME) {
    state = IDLE;
    return;
  }

  if (code == LV_EVENT_CLICKED) {
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    const char * txt = lv_btnmatrix_get_btn_text(obj, id);

    if (state == LEARN) {
      final_idx = idx;
    }

    idx = values;

    FILE* f;
    switch (id) {
      case 0:
        state = LEARN;
        Braccio.disengage();
        lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKED);
        Serial.println("LEARN");
        break;
      case 1:
        state = REPEAT;
        Braccio.engage();
        lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKED);
        Serial.println("REPEAT");
        break;
      case 3:
        state = DEMO;
        Braccio.moveTo(wavePos[0], wavePos[1], wavePos[2], wavePos[3], wavePos[4], wavePos[5]);
        lv_btnmatrix_set_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_CHECKED);
        Serial.println("DEMO");
        break;
      default:
        state = IDLE;
        Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
        lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
        Serial.println("IDLE");
        break;
    }
  }
}

void mainMenu() {
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(COLOR_LIGHT_TEAL));
  lv_style_set_text_color(&style_btn, lv_color_white());

  btnm = lv_btnmatrix_create(lv_scr_act());
  lv_obj_set_size(btnm, 240, 240);
  lv_btnmatrix_set_map(btnm, btnm_map);
  lv_obj_align(btnm, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_style(btnm, &style_btn, LV_PART_ITEMS);

  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKABLE);

  lv_btnmatrix_set_one_checked(btnm, true);
  lv_btnmatrix_set_selected_btn(btnm, 2);
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

  Serial.begin(115200);
  Serial.println("Replicate a movement");

  Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
}

void loop() {
  if (state == LEARN) {
    Braccio.positions(idx);
    idx += 6;
  }
  if (state == REPEAT) {
    Braccio.moveTo(idx[0], idx[1], idx[2], idx[3], idx[4], idx[5]);
    idx += 6;
    if (idx >= final_idx) {
      Serial.println("Repeat done");
      state = IDLE;
      lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKED);
    }
  }

  if (state == DEMO) {
    ciaoMovement();
    state = IDLE;
  }

  if (idx - values >= sizeof(values)) {
    Serial.println("IDLE");
    state = IDLE;
  }
  delay(100);
  if (state != IDLE) {
    lv_label_set_text_fmt(counter, "Counter: %d" , idx - values);
  }
}
