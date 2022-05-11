/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>

#include "AppState.h"

/**************************************************************************************
 * DEFINES
 **************************************************************************************/

// Colors
#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2
#define COLOR_YELLOW     0xE5AD24

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum states {
  SHOULDER,
  ELBOW,
  WRIST,
  PINCH
};

// Values of clicked input keys returned by Braccio.getKey() function (defined inside the library).
enum KEYS
{
  UP = 4,
  DOWN = 5,
  LEFT = 1,
  RIGHT = 2,
  CLICK = 3, // click of joystick
  ENTER = 6,
};

// IDs of the displayed directional UI buttons
enum BUTTONS {
  BTN_UP = 1,
  BTN_DOWN = 7,
  BTN_LEFT = 3,
  BTN_RIGHT = 5,
};

static const char *direction_btnm_map[] = {" ", LV_SYMBOL_UP, " ", "\n",
                                        LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, "\n",
                                        " ", LV_SYMBOL_DOWN, " ", "\0"};

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

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

char * jointsPair[] = {"Shoulder", "Elbow", "Wrist", "Pinch"};

int state = SHOULDER;

lv_obj_t * direction_btnm;  // Direction button matrix
lv_obj_t * label; // Label

ManualControlApp app;

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

void updateButtons(uint32_t key)
{
  if (key == UP){
    Braccio.lvgl_lock();
    lv_btnmatrix_set_selected_btn(direction_btnm, BTN_UP);
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_UP, LV_BTNMATRIX_CTRL_CHECKED);
    Braccio.lvgl_unlock();
  }
  else if (key == DOWN){
    Braccio.lvgl_lock();
    lv_btnmatrix_set_selected_btn(direction_btnm, BTN_DOWN);
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_CHECKED);
    Braccio.lvgl_unlock();
  }
  else if (key == LEFT) {
    Braccio.lvgl_lock();
    lv_btnmatrix_set_selected_btn(direction_btnm, BTN_LEFT);
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_LEFT, LV_BTNMATRIX_CTRL_CHECKED);
    Braccio.lvgl_unlock();
  }
  else if (key == RIGHT){
    Braccio.lvgl_lock();
    lv_btnmatrix_set_selected_btn(direction_btnm, BTN_RIGHT);
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_RIGHT, LV_BTNMATRIX_CTRL_CHECKED);
    Braccio.lvgl_unlock();
  }
  else {
    Braccio.lvgl_lock();
    lv_btnmatrix_set_selected_btn(direction_btnm, NULL);
    Braccio.lvgl_unlock();
  }
/*
  if (state == ELBOW){
    Braccio.lvgl_lock();
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_LEFT, LV_BTNMATRIX_CTRL_HIDDEN);
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_RIGHT, LV_BTNMATRIX_CTRL_HIDDEN);
    Braccio.lvgl_unlock();
  }
  else if (state == PINCH){
    Braccio.lvgl_lock();
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_UP, LV_BTNMATRIX_CTRL_HIDDEN);
    lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_HIDDEN);
    Braccio.lvgl_unlock();
  }
  else{
    Braccio.lvgl_lock();
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_UP, LV_BTNMATRIX_CTRL_HIDDEN);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_HIDDEN);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_LEFT, LV_BTNMATRIX_CTRL_HIDDEN);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_RIGHT, LV_BTNMATRIX_CTRL_HIDDEN);
    Braccio.lvgl_unlock();
  }
  */
  /*
  Braccio.lvgl_lock();
  lv_label_set_text(label, jointsPair[state]);
  Braccio.lvgl_unlock();
  */
}

// Event Handlers



// Screens functions

void directionScreen(void)
{
  Braccio.lvgl_lock();
  
  static lv_style_t style_bg;
  lv_style_init(&style_bg);
  lv_style_set_bg_color(&style_bg, lv_color_white());

  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(COLOR_LIGHT_TEAL));
  lv_style_set_text_color(&style_btn, lv_color_white());

  direction_btnm = lv_btnmatrix_create(lv_scr_act());
  lv_obj_set_size(direction_btnm, 240, 240);
  lv_btnmatrix_set_map(direction_btnm, direction_btnm_map);
  lv_obj_align(direction_btnm, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_style(direction_btnm, &style_bg, 0);
  lv_obj_add_style(direction_btnm, &style_btn, LV_PART_ITEMS);

  lv_btnmatrix_set_btn_ctrl(direction_btnm, 0, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 2, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 4, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 5, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 6, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 7, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 8, LV_BTNMATRIX_CTRL_HIDDEN);

  lv_btnmatrix_set_one_checked(direction_btnm, true);
  lv_btnmatrix_set_selected_btn(direction_btnm, 1);

  label = lv_label_create(lv_scr_act());
  lv_obj_set_width(label, 240);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(label, jointsPair[state]);
  
  Braccio.lvgl_unlock();
}

void setup()
{
  Serial.begin(115200);

  if (Braccio.begin(directionScreen))
  {
    /* Configure Braccio. */
    Braccio.speed(MEDIUM);
    /* Move to home position. */
    Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
    delay(500);
    /* Enable buttons. */
    Braccio.lvgl_lock();
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, 1, LV_BTNMATRIX_CTRL_DISABLED);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, 3, LV_BTNMATRIX_CTRL_DISABLED);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, 5, LV_BTNMATRIX_CTRL_DISABLED);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, 7, LV_BTNMATRIX_CTRL_DISABLED);
    Braccio.lvgl_unlock();
  }

  app.update(Button::None);
}

void loop()
{
  /* ENTER */

  static bool prev_joystick_pressed_enter = false;
  bool const curr_joystick_pressed_enter = Braccio.isButtonPressed_ENTER();
  if (!prev_joystick_pressed_enter && curr_joystick_pressed_enter) {
    app.update(Button::Enter);
  }
  prev_joystick_pressed_enter = curr_joystick_pressed_enter;

  /* SELECT */

  static bool prev_joystick_pressed_select = false;
  bool const curr_joystick_pressed_select = Braccio.isJoystickPressed_SELECT();
  if (!prev_joystick_pressed_select && curr_joystick_pressed_select) {
    app.update(Button::Enter);
  }
  prev_joystick_pressed_select = curr_joystick_pressed_select;

  /* DOWN */

  static bool prev_joystick_pressed_down = false;
  bool const curr_joystick_pressed_down = Braccio.isJoystickPressed_DOWN();
  if (!prev_joystick_pressed_down && curr_joystick_pressed_down) {
    handle_OnButtonDownPressed();
  }
  if (prev_joystick_pressed_down && !curr_joystick_pressed_down) {
    handle_OnButtonDownReleased();
  }
  prev_joystick_pressed_down = curr_joystick_pressed_down;

  /* UP */

  static bool prev_joystick_pressed_up = false;
  bool const curr_joystick_pressed_up = Braccio.isJoystickPressed_UP();
  if (!prev_joystick_pressed_up && curr_joystick_pressed_up) {
    handle_OnButtonUpPressed();
  }
  if (prev_joystick_pressed_up && !curr_joystick_pressed_up) {
    handle_OnButtonUpReleased();
  }
  prev_joystick_pressed_up = curr_joystick_pressed_up;

  /* LEFT */

  static bool prev_joystick_pressed_left = false;
  bool const curr_joystick_pressed_left = Braccio.isJoystickPressed_LEFT();
  if (!prev_joystick_pressed_left && curr_joystick_pressed_left) {
    handle_OnButtonLeftPressed();
  }
  if (prev_joystick_pressed_left && !curr_joystick_pressed_left) {
    handle_OnButtonLeftReleased();
  }
  prev_joystick_pressed_left = curr_joystick_pressed_left;

  /* RIGHT */

  static bool prev_joystick_pressed_right = false;
  bool const curr_joystick_pressed_right = Braccio.isJoystickPressed_RIGHT();
  if (!prev_joystick_pressed_right && curr_joystick_pressed_right) {
    handle_OnButtonRightPressed();
  }
  if (prev_joystick_pressed_right && !curr_joystick_pressed_right) {
    handle_OnButtonRightReleased();
  }
  prev_joystick_pressed_right = curr_joystick_pressed_right;


  static auto prev = millis();
  auto const now = millis();
  if ((now - prev) > 100)
  {
    prev = now;

    if (Braccio.isJoystickPressed_UP())
      app.update(Button::Up);
    if (Braccio.isJoystickPressed_DOWN())
      app.update(Button::Down);
    if (Braccio.isJoystickPressed_LEFT())
      app.update(Button::Left);
    if (Braccio.isJoystickPressed_RIGHT())
      app.update(Button::Right);
  }
}

void handle_OnButtonDownPressed()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}

void handle_OnButtonDownReleased()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}

void handle_OnButtonUpPressed()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_UP, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}

void handle_OnButtonUpReleased()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_UP, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}

void handle_OnButtonLeftPressed()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_LEFT, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}

void handle_OnButtonLeftReleased()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_LEFT, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}

void handle_OnButtonRightPressed()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_set_btn_ctrl(direction_btnm, BTN_RIGHT, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}

void handle_OnButtonRightReleased()
{
  Braccio.lvgl_lock();
  lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_RIGHT, LV_BTNMATRIX_CTRL_CHECKED);
  Braccio.lvgl_unlock();
}
