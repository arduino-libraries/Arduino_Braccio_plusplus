#include <BraccioV2.h>

#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "lib/motors/SmartServo.h"


class PD_UFP_log_c PD_UFP;
TCA6424A expander(TCA6424A_ADDRESS_ADDR_HIGH);

RS485Class serial485(Serial1, 0, 7, 8); // TX, DE, RE
SmartServoClass<7> servos(serial485);

rtos::Mutex pd_mutex;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);

  pinMode(6, OUTPUT);

  /*
    PD_UFP.init_PPS(PPS_V(6.5), PPS_A(2.0));

    #ifdef __MBED__
    static rtos::Thread th;
    th.start(pd_thread);
    #endif

    while (!PD_UFP.is_PPS_ready()) {
      pd_mutex.lock();
      PD_UFP.print_status(Serial);
      PD_UFP.set_PPS(PPS_V(6.5), PPS_A(2.0));
      pd_mutex.unlock();
    }
  */

  for (int i = 0; i < 14; i++) {
    expander.setPinDirection(i, 0);
  }

  servos.begin();

  static rtos::Thread th;
  th.start(connected_thd);
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

void connected_thd() {
  while (1) {
    for (int i = 1; i < 8; i++) {
      bool connected = (servos.ping(i) == 0);
      if (connected == true) {
        setGreen(i);
      } else {
        setRed(i);
      }
    }
    delay(1000);
  }
}

void setGreen(int i) {
  expander.writePin(i * 2 - 1, 0);
  expander.writePin(i * 2 - 2, 1);
}

void setRed(int i) {
  expander.writePin(i * 2 - 1, 1);
  expander.writePin(i * 2 - 2, 0);
}

void loop() {
  Serial.println("Write the ID you want to use for this motor:");
  String ret = "";
  do {
    ret = Serial.readStringUntil('\n');
  } while (ret == "");

  int selected = ret.toInt();

  Serial.println("Writing ID " + String(selected));
  servos.setID(selected);
}
