#include <Braccio++.h>

#define BUTTON_ENTER         6

// Colors
#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2
#define COLOR_YELLOW     0xE5AD24

// Variables
float homePos[6] = {157.5, 157.5, 157.5, 157.5, 157.5, 190.0};
float angles[6]; // All motors current angles

// Braccio ++ joints
auto gripper    = Braccio.get(1);
auto wristRoll  = Braccio.get(2);
auto wristPitch = Braccio.get(3);
auto elbow      = Braccio.get(4);
auto shoulder   = Braccio.get(5);
auto base       = Braccio.get(6);

String jointsPair[] = {"Shoulder", "Elbow", "Wrist"};
String selectedJoints = "Shoulder";

enum states {
  SHOULDER,
  ELBOW,
  WRIST,
  PINCH
};

int state = SHOULDER;


static const char * btnm_map[] = {"Shoulder", "\n",
                                  "Elbow", "\n",
                                  "Wrist", "\0"
                                 };

static const char * directional_map[] = {" ", LV_SYMBOL_UP, " ", "\n",
                                         LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, "\n",
                                         " ", LV_SYMBOL_DOWN, " ", "\0"
                                        };

lv_obj_t * btnm; // Joints button matrix
lv_obj_t * directional; // Direction button matrix


// Function
void moveJoints(uint32_t btnID) {
  switch (state) {
    case SHOULDER:
      switch (btnID) {
        case 4: shoulder.move().to(angles[4] - 10.0); break;
        case 1: base.move().to(angles[5] - 10.0);    break;
        case 2: base.move().to(angles[5] + 10.0);    break;
        case 5: shoulder.move().to(angles[4] + 10.0); break;
        default: break;
      }
      break;
    case ELBOW:
      switch (btnID) {
        case 4: elbow.move().to(angles[3] - 10.0); break;
        case 5: elbow.move().to(angles[3] + 10.0); break;
        default: break;
      }
      break;
    case WRIST:
      switch (btnID) {
        case 4: wristPitch.move().to(angles[2] - 10.0); break;
        case 1: wristRoll.move().to(angles[1] - 10.0);  break;
        case 2: wristRoll.move().to(angles[1] + 10.0);  break;
        case 5: wristPitch.move().to(angles[2] + 10.0); break;
        default: break;
      }
      break;
    case PINCH:
      switch (btnID) {
        case 1: gripper.move().to(angles[0] + 10.0); break;
        case 2: gripper.move().to(angles[0] - 10.0); break;      
        default: break;
      }
    default:
      break;

  }
}

// Event Handlers

static void eventHandlerDirectional(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);

  if (code == LV_EVENT_KEY) {
  uint32_t pressed_key = Braccio.getKey();
  Braccio.positions(angles);
  delay(5);
  moveJoints(pressed_key);

    if (pressed_key == BUTTON_ENTER) {
      //mainMenu(); // Load motor menu screen
      //lv_obj_del(directional); // Delete the object
      state++;
      if (state > PINCH) {
        state = SHOULDER; // restart from the shoulder
      }
    }
  }
}

// Screens functions
void mainMenu() {
  static lv_style_t style_bg;
  lv_style_init(&style_bg);
  lv_style_set_bg_color(&style_bg, lv_color_white());

  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(COLOR_YELLOW));
  lv_style_set_border_color(&style_btn, lv_color_hex(COLOR_LIGHT_TEAL));
  lv_style_set_border_width(&style_btn, 2);
  lv_style_set_text_color(&style_btn, lv_color_hex(COLOR_TEAL));
  lv_style_set_text_letter_space(&style_btn, 8);

  btnm = lv_btnmatrix_create(lv_scr_act());
  lv_obj_set_size(btnm, 240, 240);
  lv_btnmatrix_set_map(btnm, btnm_map);
  lv_obj_align(btnm, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_style(btnm, &style_bg, 0);
  lv_obj_add_style(btnm, &style_btn, LV_PART_ITEMS);

  lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm, 2, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm, 3, LV_BTNMATRIX_CTRL_CHECKABLE);

  lv_btnmatrix_set_one_checked(btnm, true);
  lv_btnmatrix_set_selected_btn(btnm, 0);

  Braccio.connectJoystickTo(btnm);
}

void directionScreen(void)
{
  static lv_style_t style_bg;
  lv_style_init(&style_bg);
  lv_style_set_bg_color(&style_bg, lv_color_white());

  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(COLOR_LIGHT_TEAL));
  lv_style_set_text_color(&style_btn, lv_color_white());

  directional = lv_btnmatrix_create(lv_scr_act());
  lv_obj_set_size(directional, 240, 240);
  lv_btnmatrix_set_map(directional, directional_map);
  lv_obj_align(directional, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_style(directional, &style_bg, 0);
  lv_obj_add_style(directional, &style_btn, LV_PART_ITEMS);

  lv_btnmatrix_set_btn_ctrl(directional, 0, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(directional, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 2, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(directional, 3, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 4, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(directional, 5, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 6, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(directional, 7, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 8, LV_BTNMATRIX_CTRL_HIDDEN);

  if (state == ELBOW) {
    lv_btnmatrix_set_btn_ctrl(directional, 3, LV_BTNMATRIX_CTRL_HIDDEN);
    lv_btnmatrix_set_btn_ctrl(directional, 5, LV_BTNMATRIX_CTRL_HIDDEN);
  }

  lv_btnmatrix_set_one_checked(directional, true);
  lv_btnmatrix_set_selected_btn(directional, 1);

  lv_obj_add_event_cb(directional, eventHandlerDirectional, LV_EVENT_ALL, NULL);

  delay(50);
  Braccio.connectJoystickTo(directional);
}

void setup() {
  Braccio.begin(directionScreen);
  delay(500); // Waits for the Braccio initialization

  Braccio.speed(SLOW);

  Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
  delay(500);
}

void loop() {
}
