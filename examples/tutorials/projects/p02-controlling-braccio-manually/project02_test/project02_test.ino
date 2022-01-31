#include "Braccio++.h"

#define BUTTON_ENTER         6

// Colors
#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2
#define COLOR_ORANGE     0xE47128
#define COLOR_YELLOW     0xE5AD24
#define COLOR_WHITE      0xFFFFFF
#define COLOR_BLACK      0x000000

// Variables
String selectedJoints;
float homePos[6] = {0.0, 135.0, 200.0, 200.0, 80.0, 80.0};
float angles[6]; // All motors current angles

// Braccio ++ joints
auto gripper    = Braccio.get(1);
auto wristRoll  = Braccio.get(2);
auto wristPitch = Braccio.get(3);
auto elbow      = Braccio.get(4);
auto shoulder   = Braccio.get(5);
auto base       = Braccio.get(6);

String jointsPair[] = {"Shoulder", "Elbow", "Wrist", "Gripper"};

static const char * btnm_map[] = {"Shoulder", "\n",
                                  "Elbow", "\n",
                                  "Wrist", "\n",
                                  "Gripper", "\0"
                                 };

static const char * directional_map[] = {" ", LV_SYMBOL_UP, " ", "\n",
                                         LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, "\n",
                                         " ", LV_SYMBOL_DOWN, " ", "\0"
                                        };

lv_obj_t * btnm; // Joints button matrix
lv_obj_t * directional; // Direction button matrix


// Event Handlers
static void eventHandlerMenu(lv_event_t * e) {
  lv_obj_t * obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);

  selectedJoints = jointsPair[id];
  
  directionScreen();
  lv_obj_del(btnm);
}

static void eventHandlerDirectional(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  
  if (code == LV_EVENT_DRAW_PART_BEGIN) {
    lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);

    /*Change the draw descriptor the 1nd button*/
    if ((dsc->id) % 2 == 1) {
      dsc->rect_dsc->bg_color = lv_color_hex(COLOR_LIGHT_TEAL);
      dsc->label_dsc->color = lv_color_white();
    } else {
      dsc->rect_dsc->bg_color = lv_color_white();
      dsc->rect_dsc->shadow_width = 0;
    }
  }
  if (code == LV_EVENT_KEY) {
    uint32_t pressed_key = Braccio.getKey();

    if (pressed_key == BUTTON_ENTER) {
      motorMenu(); // Load motor menu screen
      lv_obj_del(directional); // Delete the object
    }
  }
  if (code == LV_EVENT_PRESSING) {
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    Braccio.positions(angles);
    delay(50);
    
    if(selectedJoints == "Shoulder"){
      switch(id){
        case 1:
          shoulder.move().to(angles[4]-5.0);
          break;
        case 3:
          base.move().to(angles[5]-10.0);
          break;
        case 5:
          base.move().to(angles[5]+10.0);
          break;
        case 7:
          shoulder.move().to(angles[4]+5.0);
          break;
        default:
          break;
      }
    }
  }
}

// Screens functions
void directionScreen(void)
{
  directional = lv_btnmatrix_create(lv_scr_act());
  lv_btnmatrix_set_map(directional, directional_map);
  lv_obj_align(directional, LV_ALIGN_CENTER, 0, 0);

  lv_btnmatrix_set_btn_ctrl(directional, 0, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(directional, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 2, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(directional, 3, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 4, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(directional, 5, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 6, LV_BTNMATRIX_CTRL_DISABLED);
  lv_btnmatrix_set_btn_ctrl(directional, 7, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(directional, 8, LV_BTNMATRIX_CTRL_DISABLED);

  lv_btnmatrix_set_one_checked(directional, true);

  lv_obj_add_event_cb(directional, eventHandlerDirectional, LV_EVENT_ALL, NULL);

  Braccio.connectJoystickTo(btnm);
}

void motorMenu() {
  static lv_style_t style_bg;
  lv_style_init(&style_bg);
  lv_style_set_bg_color(&style_bg, lv_color_hex(COLOR_WHITE));

  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(COLOR_YELLOW));
  lv_style_set_border_color(&style_btn, lv_color_hex(COLOR_LIGHT_TEAL));
  lv_style_set_border_width(&style_btn, 2);
  lv_style_set_text_color(&style_btn, lv_color_hex(COLOR_TEAL));
  lv_style_set_text_letter_space(&style_btn, 8);
//  lv_style_set_text_font(&style_btn, &lv_font_montserrat_14);

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
  lv_btnmatrix_set_btn_ctrl(btnm, 4, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_btnmatrix_set_btn_ctrl(btnm, 5, LV_BTNMATRIX_CTRL_CHECKABLE);

  lv_btnmatrix_set_one_checked(btnm, true);

  lv_obj_add_event_cb(btnm, eventHandlerMenu, LV_EVENT_PRESSED, NULL);

  Braccio.connectJoystickTo(btnm);
}

void setup() {
  Braccio.begin(motorMenu);
  delay(500); // Waits for the Braccio initialization

  Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
}

void loop() {
  // Fetch the joints positions
  Braccio.positions(angles);
}
