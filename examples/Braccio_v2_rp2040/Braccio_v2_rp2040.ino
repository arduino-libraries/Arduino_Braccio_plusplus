#include <BraccioV2.h>

#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "lib/motors/SmartServo.h"
#include "drivers/Ticker.h"

//Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "Images.h"

#if defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiNINA.h>
#endif

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
SmartServoClass<7> servos(serial485);


bool connected[8];

rtos::EventFlags pd_events;
rtos::Mutex pd_mutex;
mbed::Ticker pd_timer;

void unlock_pd_semaphore() {
  pd_events.set(1);
}

void onErrors() {
  Serial.println("got error");
}

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Wire.begin();

  Serial.println("start");

  pinMode(6, OUTPUT);

  PD_UFP.init_PPS(PPS_V(7.2), PPS_A(2.0));

#ifdef __MBED__
  static rtos::Thread th;
  th.start(pd_thread);
  attachInterrupt(PIN_FUSB302_INT, unlock_pd_semaphore, FALLING);
  pd_timer.attach(unlock_pd_semaphore, 0.06f);
#endif

  while (!PD_UFP.is_PPS_ready()) {
    pd_mutex.lock();
    //PD_UFP.print_status(Serial);
    PD_UFP.set_PPS(PPS_V(7.2), PPS_A(2.0));
    pd_mutex.unlock();
  }

  pinMode(1, INPUT_PULLUP);

  bl.begin();
  Serial.println(bl.getChipID(), HEX);
  bl.setColor(red);

  int ret = expander.testConnection();

  servos.begin();
  servos.setPositionMode(pmSYNC);

  display.init(240, 240);
  display.setRotation(2);
  display.fillScreen(ST77XX_BLACK);
  drawArduino(0x04B3);

  pinMode(JOYSTICK_LEFT, INPUT_PULLUP);
  pinMode(JOYSTICK_RIGHT, INPUT_PULLUP);
  pinMode(JOYSTICK_UP, INPUT_PULLUP);
  pinMode(JOYSTICK_OK, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);

  servos.onErrorCb(onErrors);

  for (int i = 0; i < 14; i++) {
    expander.setPinDirection(i, 0);
  }

  for (int i = 1; i < 7; i++) {
    //servos.hold(i, false);
    //delay(100);
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
    pd_events.wait_any(0xFF);
    pd_mutex.lock();
    PD_UFP.run();
    pd_mutex.unlock();
    if (PD_UFP.is_power_ready() && PD_UFP.is_PPS_ready()) {
      Serial.println("error");
      while (1) {}
    }
  }
}

uint32_t uart_error = 0;

enum learn_situation {
  DISENGAGE,
  ENGAGE,
  LEARN,
  REPLAY,
  WAIT
};

int learn_index = 0;
int learn_index_max = 0;
int learn_mode = WAIT;

int count_total = 0;
int lost = 0;

float movement[1000][6];

void motors_connected_thread() {
  while (1) {
    if (learn_mode == WAIT) {
      for (int i = 1; i < 7; i++) {
        connected[i] = (servos.ping(i) == 0);
        Serial.print(String(i) + ": ");
        Serial.println(connected[i]);
        pd_mutex.lock();
        if (connected[i]) {
          setGreen(i);
        } else {
          setRed(i);
        }
        pd_mutex.unlock();
      }
    }
    delay(1000);
  }
}

void loop() {

#ifndef __MBED__
  PD_UFP.run();
  if (PD_UFP.is_power_ready() && PD_UFP.is_PPS_ready()) {
    Serial.println("error");
    while (1) {}
  }
#endif

  if (Serial.available()) {
    int action = Serial.read();
    if (action == 'W') {
        learn_mode = WAIT;
    }
    if (action == 'E') {
        learn_mode = ENGAGE;
    }
    if (action == 'D') {
        learn_mode = DISENGAGE;
    }
  }

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

  if (learn_mode == DISENGAGE) {
    Serial.println("Learn mode enabled");
    servos.setTorque(false);
  } 

  if (learn_mode == ENGAGE) {
    Serial.println("Replay mode enabled");
    servos.setTorque(true);
  }

  for (int i = 1; i < 7; i++) {
    /*
        if (connected[i] == false || (learn_index > sizeof(movement) / sizeof(float))) {
          continue;
        }
    */
    switch (learn_mode) {
      /*
                  case DISENGAGE:
                    servos.disengage(i);
                    break;
      */
      case ENGAGE:
        servos.setTime(i, 100);
        break;
      case REPLAY:
        if (learn_index < learn_index_max) {
          servos.setPosition(i, movement[learn_index][i - 1], 0);
        }
        break;
      case LEARN:
        if (learn_index < 1000) {
          movement[learn_index][i - 1] = servos.getPosition(i);
        }
        break;
    }
  }

  if (learn_mode == DISENGAGE) {
    learn_mode = LEARN;
    learn_index = 0;
  }
  if (learn_mode == ENGAGE) {
    learn_mode = REPLAY;
    learn_index_max = learn_index;
    learn_index = 0;
  }
  if (learn_mode == LEARN || learn_mode == REPLAY) {
    learn_index++;
  }
  if (learn_mode == REPLAY) {
    servos.synchronize();
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