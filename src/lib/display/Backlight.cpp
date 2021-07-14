/* 12/23/2017 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer
 *  
 This basic sketch is to operate the IS31FL3194 3-channel led driver
 http://www.issi.com/WW/pdf/IS31FL3194.pdf
 
 The sketch uses default SDA/SCL pins on the Ladybug development board 
 but should work with almost any Arduino-based board.
 Library may be used freely and without limit only with attribution.
 
  */

#include <Wire.h>
#include "Backlight.h"

void Backlight::begin()
{
  reset();
  init();
  powerUp();
}

void Backlight::end()
{
  powerDown();
}

void Backlight::setColor(RGBColors color)
{
  if(color == off) {
  _blue = 0x00;
  _green = 0x00;
  _red = 0x00;
  }

  if(color == green) {
  _blue = 0x00;
  _green = 0xFF;
  _red = 0x00;
  }

  if(color == blue) {
  _blue = 0xFF;
  _green = 0x00;
  _red = 0x00;
  }

  if(color == red) {
  _blue = 0x00;
  _green = 0x00;
  _red = 0xFF;
  }

  if(color == cyan) {
  _blue = 0x20;
  _green = 0x20;
  _red = 0x00;
  }

  if(color == magenta) {
  _blue = 0x20;
  _green = 0x00;
  _red = 0x20;
  }

  if(color == yellow) {
  _blue = 0x00;
  _green = 0x20;
  _red = 0x20;
  }

  setColor(_blue, _green, _red);

}

void Backlight::setColor(uint8_t blue, uint8_t green, uint8_t red)
{
  // set rgb led current
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT1, blue); //maximum current
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT2, green);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT3, red);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_COLOR_UPDATE, 0xC5); // write to color update register for changes to take effect

}

// Read the Chip ID register, this is a good test of communication
uint8_t Backlight::getChipID() 
{
  uint8_t c = readByte(IS31FL3194_ADDRESS, IS31FL3194_PRODUCT_ID); // Read PRODUCT_ID register for IS31FL3194
  return c;
}


void Backlight::reset()
{
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_RESET, 0xC5);
}


void Backlight::powerDown()
{
  uint8_t d = readByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG, d & ~(0x01)); //clear bit 0 to shut down
}


void Backlight::powerUp()
{
  uint8_t d = readByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG, d | 0x01); //set bit 0 to enable
}


void Backlight::init()// configure rgb led function
{
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG, 0x01);     // normal operation in current mode
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT_CONFIG, 0x04);    // enable all three ouputs
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_CURRENT_BAND, 0x03);  // 40 mA max current
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_HOLD_FUNCTION, 0x00); // hold function disable
  writeByte(IS31FL3194_ADDRESS, 0x32, 0xFF); // Max power on led R (OUT 3)
}

void Backlight::ledBlink(RGBColors color, uint32_t duration)
{
  setColor(color);
  delay(duration);
  setColor(off);
}

void Backlight::writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  Wire.beginTransmission(address);
  Wire.write(subAddress);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t Backlight::readByte(uint8_t address, uint8_t subAddress)
{
  char response = 0xFF;
  Wire.beginTransmission(address);
  Wire.write(subAddress);
  Wire.endTransmission(false);
  Wire.requestFrom(address, 1);
  while(!Wire.available()) {}
  return Wire.read();
}
