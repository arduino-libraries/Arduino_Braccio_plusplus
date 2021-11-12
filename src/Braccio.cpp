#include "Braccio++.h"
#include "menu.impl"

bool BraccioClass::begin(positionMode _positionMode) {

	Serial.begin(115200);

	Wire.begin();
	SPI.begin();

	PD_UFP.init_PPS(PPS_V(7.2), PPS_A(2.0));

#ifdef __MBED__
	static rtos::Thread th;
	th.start(mbed::callback(this, &BraccioClass::pd_thread));
	attachInterrupt(PIN_FUSB302_INT, mbed::callback(this, &BraccioClass::unlock_pd_semaphore), FALLING);
	pd_timer.attach(mbed::callback(this, &BraccioClass::unlock_pd_semaphore), 0.06f);
#endif

	pinMode(1, INPUT_PULLUP);

	bl.begin();
	if (bl.getChipID() != 0xCE) {
		return false;
	}
	bl.setColor(red);

	int ret = expander.testConnection();

	if (ret == false) {
		return ret;
	}

	for (int i = 0; i < 14; i++) {
		expander.setPinDirection(i, 0);
	}

	// Set SLEW to low
	expander.setPinDirection(21, 0); // P25 = 8 * 2 + 5
	expander.writePin(21, 0);

	// Set TERM to HIGH (default)
	expander.setPinDirection(19, 0); // P23 = 8 * 2 + 3
	expander.writePin(19, 1);

	expander.setPinDirection(18, 0); // P22 = 8 * 2 + 2
	expander.writePin(18, 0); // reset LCD
	expander.writePin(18, 1); // LCD out of reset

	braccio::encoder.begin();

	braccio::gfx.init();
	braccio::gfx.setRotation(4);
	braccio::gfx.fillScreen(TFT_BLACK);
	braccio::gfx.setFreeFont(&FreeMono18pt7b);
	braccio::gfx.println("Arduino\nBraccio++");

#ifdef __MBED__
	static rtos::Thread display_th;
	display_th.start(mbed::callback(this, &BraccioClass::display_thread));
#endif

	braccio::gfx.println("Please\nconnect\npower");

	while (!PD_UFP.is_PPS_ready() /* && !encoder.menu_interrupt */) {
		pd_mutex.lock();
		//PD_UFP.print_status(Serial);
		PD_UFP.set_PPS(PPS_V(7.2), PPS_A(2.0));
		pd_mutex.unlock();
	}

	servos->begin();
	servos->setPositionMode(pmSYNC);

	braccio::gfx.println("Get Ready!");

#ifdef __MBED__
	static rtos::Thread connected_th;
	connected_th.start(mbed::callback(this, &BraccioClass::motors_connected_thread));
#endif
}


void BraccioClass::pd_thread() {
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

void BraccioClass::display_thread() {
  while (1) {
    if ((braccio::encoder.menu_running) && (braccio::encoder.menu_interrupt)) {
      braccio::encoder.menu_interrupt = false;
      braccio::nav.doInput();
      braccio::nav.doOutput();
    }
    yield();
  }
}

void BraccioClass::motors_connected_thread() {
  while (1) {
    for (int i = 1; i < 7; i++) {
      _connected[i] = (servos->ping(i) == 0);
      Serial.print(String(i) + ": ");
      Serial.println(_connected[i]);
      pd_mutex.lock();
      if (_connected[i]) {
        setGreen(i);
      } else {
        setRed(i);
      }
      pd_mutex.unlock();
    }
    delay(1000);
  }
}

BraccioClass Braccio;
