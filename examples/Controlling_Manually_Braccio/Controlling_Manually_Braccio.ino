#include <Braccio++.h>

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

char * jointsPair[] = {"Shoulder", "Elbow", "Wrist", "Pinch"};
String selectedJoints = "Shoulder";

enum states {
  SHOULDER,
  ELBOW,
  WRIST,
  PINCH
};

int state = SHOULDER;

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

uint32_t pressed_key;
uint32_t last_pressed_key;


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

lv_obj_t * direction_btnm;  // Direction button matrix
static lv_obj_t * label; // Label

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
  
  Braccio.lvgl_lock();
  lv_label_set_text(label, jointsPair[state]);
  Braccio.lvgl_unlock();
}

// Event Handlers

static void eventHandlerDirectional(lv_event_t * e) {
  Braccio.lvgl_lock();
  
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  
  Braccio.lvgl_unlock();

  if (code == LV_EVENT_KEY){
  pressed_key = Braccio.getKey();

    if (pressed_key == ENTER || pressed_key == CLICK){
      state++; // Index the next joints in the states enum array
      
      if (state > PINCH){
        state = SHOULDER; // restart from the shoulder
      }
    }
    updateButtons(pressed_key);
    Braccio.positions(angles);
    delay(5);
    moveJoints(pressed_key);
  }
}

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
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 2, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 3, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 4, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 5, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 6, LV_BTNMATRIX_CTRL_HIDDEN);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 7, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(direction_btnm, 8, LV_BTNMATRIX_CTRL_HIDDEN);

  lv_btnmatrix_set_one_checked(direction_btnm, true);
  lv_btnmatrix_set_selected_btn(direction_btnm, 1);

  lv_obj_add_event_cb(direction_btnm, eventHandlerDirectional, LV_EVENT_ALL, NULL);

 label = lv_label_create(lv_scr_act());
  lv_obj_set_width(label, 240);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(label, jointsPair[state]);
  
  Braccio.lvgl_unlock();

  delay(50);
  Braccio.connectJoystickTo(direction_btnm);
}

void setup()
{
  Braccio.begin(directionScreen);
  delay(500); // Waits for the Braccio initialization

  Braccio.speed(SLOW);

  Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
  delay(500);
}

void loop()
{
  pressed_key= Braccio.getKey();
 if (pressed_key == 0) {
  if(pressed_key != last_pressed_key){
    Braccio.lvgl_lock();
    
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_UP, LV_BTNMATRIX_CTRL_CHECKED);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_DOWN, LV_BTNMATRIX_CTRL_CHECKED);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_RIGHT, LV_BTNMATRIX_CTRL_CHECKED);
    lv_btnmatrix_clear_btn_ctrl(direction_btnm, BTN_LEFT, LV_BTNMATRIX_CTRL_CHECKED);
    
    Braccio.lvgl_unlock();
    
    delay(50);  

    }
  }
    last_pressed_key=pressed_key;
}
