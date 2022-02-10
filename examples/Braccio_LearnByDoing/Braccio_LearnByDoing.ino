/*
 * @brief Learn the arm an movement and replay it.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"

/**************************************************************************************
 * VARIABLES
 **************************************************************************************/

enum states {
  LEARN,
  REPEAT,
  IDLE
};

int state = IDLE;

float values[10000];
float* idx = values;
float* final_idx = 0;

/**************************************************************************************
 * FUNCTIONS
 **************************************************************************************/

static void event_handler(lv_event_t * e) {
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
        Serial.println("LEARN");
        break;
      case 1:
        state = REPEAT;
        Braccio.engage();
        Serial.println("REPEAT");
        break;
      case 2:
        state = IDLE;
        f = fopen("/fs/movements", "wb");
        fwrite(values, 1, (final_idx - values) * sizeof(float), f);
        fflush(f);
        fclose(f);
        Serial.print("SAVED ");
        Serial.print((final_idx - values) * sizeof(float));
        Serial.println(" bytes");
        break;
      case 3:
        state = IDLE;
        f = fopen("/fs/movements", "rb");
        final_idx = values + fread(values, 1, sizeof(values), f) / sizeof(float);
        fclose(f);
        Serial.print("LOADED ");
        Serial.print((final_idx - values) * sizeof(float));
        Serial.println(" bytes");
        break;
      default:
        state = IDLE;
        Serial.println("IDLE");
        break;
    }
  }
}

static lv_obj_t *counter;

static const char * btnm_map[] = { "Learn", "\n", "Replay", "\n", "Save in flash", "\n", "Load from flash", "\n", "Idle", "\n", "\0" };

void customMenu() {
  lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
  lv_btnmatrix_set_map(btnm1, btnm_map);
  lv_btnmatrix_set_btn_ctrl(btnm1, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 2, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 3, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm1, 4, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
  counter = lv_label_create(btnm1);
  lv_label_set_text_fmt(counter, "%d" , 0);
  lv_obj_align(counter, LV_ALIGN_CENTER, 0, 50);
  lv_obj_add_event_cb(btnm1, event_handler, LV_EVENT_ALL, NULL);
  Braccio.connectJoystickTo(btnm1);
}

static FlashIAPBlockDevice bd(XIP_BASE + 0x100000, 0x100000);
static mbed::FATFileSystem fs("fs");

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }

  // Mount file system for load/store movements
  int err = fs.mount(&bd);
  if (err) {
    err = fs.reformat(&bd);
  }

  // Call Braccio.begin() for default menu or pass a function for custom menu
  Braccio.begin(customMenu);
  Serial.println("Replicate a movement");
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
    }
  }
  if (idx - values >= sizeof(values)) {
    Serial.println("IDLE");
    state = IDLE;
  }
  delay(100);
  if (state != IDLE) {
    lv_label_set_text_fmt(counter, "%d" , idx - values);
  }
}
