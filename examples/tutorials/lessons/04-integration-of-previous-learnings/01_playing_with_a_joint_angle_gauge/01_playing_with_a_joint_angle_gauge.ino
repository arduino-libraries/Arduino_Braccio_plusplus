#include <Braccio++.h>

#define JOYSTICK_LEFT        1
#define JOYSTICK_RIGHT       2
#define JOYSTICK_SEL_BUTTON  3
#define JOYSTICK_UP          4
#define JOYSTICK_DOWN        5
#define BUTTON_ENTER         6

// Colors
#define COLOR_TEAL       0x00878F
#define COLOR_LIGHT_TEAL 0x62AEB2
#define COLOR_ORANGE     0xE47128

int motorID = 1;
float initialAngle = 0.0;
float currentAngle = initialAngle;
float angles[6];

static lv_obj_t * meter;
lv_meter_indicator_t * indic;


static void eventHandlerMeter(lv_event_t * e) {
  /* Set the meter value */
  Braccio.lvgl_lock();
  lv_meter_set_indicator_end_value(meter, indic, (int32_t)angles[motorID - 1]);
  Braccio.lvgl_unlock();
}


void meterScreen(void)
{
  Braccio.lvgl_lock();
  meter = lv_meter_create(lv_scr_act());

  lv_obj_center(meter);
  lv_obj_set_size(meter, 200, 200);

  /*Remove the circle from the middle*/
  lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);

  static lv_style_t style_meter;
  lv_style_init(&style_meter);
  lv_style_set_text_color(&style_meter, lv_color_hex(COLOR_TEAL));
  lv_obj_add_style(meter, &style_meter, 0);

  /*Add a scale first*/
  lv_meter_scale_t * scale = lv_meter_add_scale(meter);
  lv_meter_set_scale_ticks(meter, scale, 8, 2, 10, lv_color_hex(COLOR_ORANGE));
  lv_meter_set_scale_major_ticks(meter, scale, 1, 2, 20, lv_color_hex(COLOR_ORANGE), 20);
  lv_meter_set_scale_range(meter, scale, 0, 315, 315, 90);

  /*Add a arc indicator*/
  indic = lv_meter_add_arc(meter, scale, 10, lv_color_hex(COLOR_LIGHT_TEAL), 0);

  lv_obj_add_event_cb(meter, eventHandlerMeter, LV_EVENT_KEY, NULL);
  Braccio.lvgl_unlock();

  Braccio.connectJoystickTo(meter);
}

void setup() {
  Braccio.begin(meterScreen);

  delay(500); // Waits for the Braccio initialization

  // Sets the initial angle for the motors
  for (int i = 1; i <= 6; i++) {
    Braccio.move(i).to(initialAngle);
    delay(1000); // Necessary to set the motor ID correctly
  }
}

void loop() {
  int pressedKey = Braccio.getKey();

  Braccio.positions(angles);
  currentAngle = angles[motorID - 1];

  // Check if the Joystick Left is pressed
  if (pressedKey == JOYSTICK_LEFT) {
    if (currentAngle <= 315 ) {
      currentAngle += 15.0;
    } else {
      currentAngle = 315.0;
    }
  }

  // Check if the Joystick Right is pressed
  if (pressedKey == JOYSTICK_RIGHT) {
    if (currentAngle > 15 ) {
      currentAngle -= 15.0;
    } else {
      currentAngle = 0.0;
    }
  }

  Braccio.move(motorID).to(currentAngle); // Moves the motor to the new angle
  delay(100);
}
