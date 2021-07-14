#include <BraccioV2.h>
#include <PDM.h>

#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "lib/motors/SmartServo.h"


//Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "Images.h"

#include <WiFiNINA.h>

class PD_UFP_c PD_UFP;
TCA6424A expander(TCA6424A_ADDRESS_ADDR_HIGH);
Backlight bl;
Adafruit_ST7789 display = Adafruit_ST7789(&SPI, 10, 9, -1);

#define JOYSTICK_LEFT   5
#define JOYSTICK_RIGHT  4
#define JOYSTICK_UP     3
#define JOYSTICK_DOWN   2
#define JOYSTICK_OK     A6
#define BUTTON_ENTER    A7

int joystick_left_status = -1;
int joystick_right_status = -1;
int joystick_up_status = -1;
int joystick_down_status = -1;
int joystick_ok_status = -1;
int button_enter_status = -1;

RS485Class serial485(Serial1, 0, 7, 8); // TX, DE, RE
SmartServoClass servos(serial485);

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Wire.begin();
  PD_UFP.init_PPS(PPS_V(8.0), PPS_A(2.0));

  bl.begin();
  Serial.println(bl.getChipID(), HEX);
  bl.setColor(red);

  int ret = expander.testConnection();

  servos.begin();

  display.init(240, 240);
  display.setRotation(2);
  display.fillScreen(ST77XX_BLACK);
  drawArduino(0x04B3);

  pinMode(JOYSTICK_LEFT, INPUT_PULLUP);
  pinMode(JOYSTICK_RIGHT, INPUT_PULLUP);
  pinMode(JOYSTICK_UP, INPUT_PULLUP);
  pinMode(JOYSTICK_OK, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);

  expander.setPinDirection(5, 0);
  expander.writePin(5, 0);

  expander.setPinDirection(10, 0);
  expander.writePin(10, 0);
}

void drawArduino(uint16_t color) {
  display.drawBitmap(44, 60, ArduinoLogo, 152, 72, color);
  display.drawBitmap(48, 145, ArduinoText, 144, 23, color);
}

void loop() {

  if (joystick_left_status != digitalRead(JOYSTICK_LEFT)) {
    joystick_left_status = digitalRead(JOYSTICK_LEFT);
    Serial.println("joystick_left_status: " + String(joystick_left_status));
  }
  if (joystick_right_status != digitalRead(JOYSTICK_RIGHT)) {
    joystick_right_status = digitalRead(JOYSTICK_RIGHT);
    Serial.println("joystick_right_status: " + String(joystick_right_status));
  }
  if (joystick_up_status != digitalRead(JOYSTICK_UP)) {
    joystick_up_status = digitalRead(JOYSTICK_UP);
    Serial.println("joystick_up_status: " + String(joystick_up_status));
  }
  if (joystick_down_status != digitalRead(JOYSTICK_DOWN)) {
    joystick_down_status = digitalRead(JOYSTICK_DOWN);
    Serial.println("joystick_down_status: " + String(joystick_down_status));
  }
  if (abs(joystick_ok_status - analogRead(JOYSTICK_OK)) > 200) {
    joystick_ok_status = analogRead(JOYSTICK_OK);
    Serial.println("joystick_ok_status: " + String(joystick_ok_status));
  }
  if (abs(button_enter_status - analogRead(BUTTON_ENTER)) > 200) {
    button_enter_status = analogRead(BUTTON_ENTER);
    Serial.println("button_enter_status: " + String(button_enter_status));
  }

  PD_UFP.run();
  if (PD_UFP.is_PPS_ready()) {          // PPS trigger success
    //Serial.println("PPS set");
  } else if (PD_UFP.is_power_ready()) { // Fail to trigger PPS, fall back
    Serial.println("Fallback");
  }
}
