#include <BraccioV2.h>
#include <PDM.h>

#include "lib/powerdelivery/PD_UFP.h"
#include "lib/ioexpander/TCA6424A.h"
#include "lib/display/Backlight.h"
#include "Images.h"

//Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

class PD_UFP_c PD_UFP;
TCA6424A expander(TCA6424A_ADDRESS_ADDR_HIGH);
Backlight bl;

Adafruit_ST7789 display = Adafruit_ST7789(&SPI, 10, 9, -1);

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Wire.begin();
  PD_UFP.init_PPS(PPS_V(8.0), PPS_A(2.0));

  bl.begin();
  Serial.println(bl.getChipID(), HEX);
  bl.setColor(red);

  int ret = expander.testConnection();

  display.init(240, 240);
  display.setRotation(2);
  display.fillScreen(ST77XX_BLACK);
  drawArduino(0x04B3);

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
  PD_UFP.run();
  if (PD_UFP.is_PPS_ready()) {          // PPS trigger success
    //Serial.println("PPS set");
  } else if (PD_UFP.is_power_ready()) { // Fail to trigger PPS, fall back
    Serial.println("Fallback");
  }
}
