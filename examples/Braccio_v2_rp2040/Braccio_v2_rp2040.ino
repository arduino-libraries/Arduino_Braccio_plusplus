#include <BraccioV2.h>

#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "lib/motors/SmartServo.h"


//Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "Images.h"

#if defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiNINA.h>
#endif

class PD_UFP_log_c PD_UFP;
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

bool connected[8];

rtos::Mutex pd_mutex;

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Wire.begin();

  pinMode(6, OUTPUT);

  PD_UFP.init_PPS(PPS_V(6.0), PPS_A(2.0));

#ifdef __MBED__
  static rtos::Thread th;
  th.start(pd_thread);
#endif

  while (!PD_UFP.is_PPS_ready()) {
    pd_mutex.lock();
    PD_UFP.print_status(Serial);
    PD_UFP.set_PPS(PPS_V(6.0), PPS_A(2.0));
    pd_mutex.unlock();
  }

  pinMode(1, INPUT_PULLUP);

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

  for (int i = 0; i < 14; i++) {
    expander.setPinDirection(i, 0);
  }

  for (int i = 1; i < 7; i++) {
    servos.hold(i, false);
    delay(100);
  }

  // Set SLEW to low
  expander.setPinDirection(21, 0); // P25 = 8 * 2 + 5
  expander.writePin(21, 0);

  // Set TERM to HIGH (default)
  expander.setPinDirection(19, 0); // P23 = 8 * 2 + 3
  expander.writePin(19, 1);

#ifdef __MBED__
  static rtos::Thread connected_th;
  connected_th.start(motors_connected_thread);
#endif
}

void drawArduino(uint16_t color) {
  display.drawBitmap(44, 60, ArduinoLogo, 152, 72, color);
  display.drawBitmap(48, 145, ArduinoText, 144, 23, color);
}

void setGreen(int i) {
  expander.writePin(i * 2 - 1, 0);
  expander.writePin(i * 2 - 2, 1);
}

void setRed(int i) {
  expander.writePin(i * 2 - 1, 1);
  expander.writePin(i * 2 - 2, 0);
}

void pd_thread() {
  while (1) {
    pd_mutex.lock();
    PD_UFP.run();
    pd_mutex.unlock();
    delay(10);
    if (PD_UFP.is_power_ready() && PD_UFP.is_PPS_ready()) {
      Serial.println("error");
      while (1) {}
    }
  }
}

void motors_connected_thread() {
  while (1) {
    for (int i = 1; i < 8; i++) {
      connected[i] = servos.ping(i);
      delay(100);
      pd_mutex.lock();
      if (connected[i] == true) {
        setGreen(i);
      } else {
        setRed(i);
      }
      pd_mutex.unlock();
    }
    delay(1000);
  }
}

int count_total = 0;
int lost = 0;

uint32_t uart_error = 0;

void loop() {

#ifndef __MBED__
  PD_UFP.run();
  if (PD_UFP.is_power_ready() && PD_UFP.is_PPS_ready()) {
    Serial.println("error");
    while (1) {}
  }
#endif

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

  if (uart_error) {
    Serial.println(uart_error, HEX);
    uart_error = 0;
  }

  for (int i = 1; i < 7; i++) {
    if (connected[i] == false) {
      continue;
    }
    int position = servos.regRead(i, 'p');
    delay(10);
    Serial.print("Servo ");
    Serial.print(i);
    Serial.print(" at ");
    Serial.println(position);
  }
  delay(100);
}

extern "C" void error_cb_uart(uint32_t ret) {
  uart_error = ret;
}

extern "C" void do_something_irq() {
  digitalWrite(6, HIGH);
  digitalWrite(6, LOW);
}
