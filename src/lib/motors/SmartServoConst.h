/*
 * Board support library for the Arduino Braccio++ 6-DOF robot arm.
 * Copyright (C) 2022 Arduino (http://www.arduino.cc/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA.
 */

#ifndef SMART_SERVO_CONST_H_
#define SMART_SERVO_CONST_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <cstdint>

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class SmartServoRegister : uint8_t
{  
  _NI_MFG_YEAR = 0,        //0  (0x00)  Servo factory: year        read/write    --                                           
  _NI_MFG_MONTH,           //1  (0x01)  Servo factory: month       read/write    --                         
  _NI_MFG_DAY,             //2  (0x02)  Servo factory: date        read/write    --                       
  SWVER_H,                 //3  (0x03)  Software version(H)        read          --         Software version              
  SWVER_L,                 //4  (0x04)  Software version(L)        read          --                       
  ID,                      //5  (0x05)  ID                         read/write    0x01                 
  STALL_PROTECTION_TIME,   //6  (0x06)  Stall protection time      read/write    0x03          unit second (default 3s)                             
  _NI_BOOT_TIMES_H,        //7  (0x07)  Boot times (H)             read/write    0x00          Record the boot times                   
  _NI_BOOT_TIMES_L,        //8  (0x08)  Boot times(L)              read/write    0x00                           
  MIN_ANGLE_LIMIT_H,       //9  (0x09)  Minimum angle limit (H)    read/write    0x00 (0)      Default of Minimum angle limit is 0                        
  MIN_ANGLE_LIMIT_L,       //10 (0x0A) Minimum angle limit (L)     read/write    0x00                                 
  MAX_ANGLE_LIMIT_H,       //11 (0x0B) Maximum angle limit (H)     read/write    0x10 (4000)   Default of Maximum angle limit is 4000                        
  MAX_ANGLE_LIMIT_L,       //12 (0x0C) Maximum angle limit (L)     read/write    0x00                                 
  _NI_HIGH_TEMP_LIMIT,     //13 (0x0D) High temperature limit      read/write    0x50(80°)                              
  _NI_HIGH_VOLTAGE_LIMIT,  //14 (0x0E) High Votage limit           read/write    0x08                                 
  _NI_LOW_VOLTAGE_LIMIT,   //15 (0x0F) Low Votage limit            read/write    0x04                                
  MAX_TORQUE_H,            //16 (0x10) Maximum torque (H)          read/write    0x03(800)     The torque control range is 0-1000 (the greater the value, the greater the output torque)                   
  MAX_TORQUE_L,            //17 (0x11) Maximum torque (L)          read/write    0x20                           
  _NI_SPEED_CONTROL,       //18 (0x12) Speed control               read/write    0X1E                            
  _NI_UNLOAD_CONDITION,    //19 (0x13) Unload condition            read/write    0x00                               
  CENTER_POINT_ADJ_H,      //20 (0x14) Center point adjustment(H)  read/write    0x00          Center point of servo motor                         
  CENTER_POINT_ADJ_L,      //21 (0x15) Center point adjustment(L)  read/write    0x00                                 
  _NI_LED0_SWITCH,         //22 (0x16) LED0 Switch                 read/write    0X00          0x01:on   0x00:off                  
  _NI_LED1_SWITCH,         //23 (0x17) LED1 Switch                 read/write    0X00                          
  _NI_LED2_SWITCH,         //24 (0x18) LED2  Switch                read/write    0X00                          
  _NI_LED0_ON_OFF_TIME,    //25 (0x19) LED0  light on and off time read/write    0x32 (50ms)   LED light on and off time interval                       
  _NI_LED1_ON_OFF_TIME,    //26 (0x1A) LED1  light on and off time read/write    0x32 (50ms)                               
  _NI_LED2_ON_OFF_TIME,    //27 (0x1B) LED3 light on and off time  read/write    0x32 (50ms)                               
  SERVO_MOTOR_MODE,        //28 (0x1C) Servo/motor mode switch     read/write    0xFF          0x01:servo mode   0x00:motor mode                       
  MOTOR_DIRECTION,         //29 (0x1D) Motor direction switch      read/write    0xFF          0x01:clockwise    0x00:counter-clockwise                      
  TORQUE_SWITCH = 40,      //40 (0x28) Torque switch               read/write                  0x00:off non-zero:on                 
  _NI_LED,                 //41 (0x29) LED                         read/write    0X00  
  TARGET_POSITION_H,       //42 (0x2A) Target position (H)         read/write                  Set the target position of the servo                
  TARGET_POSITION_L,       //43 (0x2B) Target position (L)         read/write                    
  RUN_TIME_H,              //44 (0x2C) Run time (H)                read/write                  unit: ms         
  RUN_TIME_L,              //45 (0x2D) Run time (L)                read/write             
  _NI_CURRENT_H,           //46 (0x2E) Current (H)                 read            
  _NI_CURRENT_L,           //47 (0x2F) Current (L)                 read            
  _NI_LOCK,                //48 (0x30) Lock                        read/write    00   
  POSITION_H = 56,         //56 (0x38) Position (H)                read                        Current position              
  POSITION_L,              //57 (0x39) Position (L)                read             
  SPEED_H,                 //58 (0x3A) Speed (H)                   read                        Run speed      
  SPEED_L,                 //59 (0x3B) Speed (L)                   read          
  _NI_LOADING_H,           //60 (0x3C) Loading (H)                 read            
  _NI_LOADING_L,           //61 (0x3D) Loading (L)                 read            
  _NI_VOLTAGE,             //62 (0x3E) Votage                      read          
  _NI_TEMPERATURE,         //63 (0x3F) Temperature                 read              
  REG_WRITE_SIGN,          //64 (0x40) REG WRITE sign              read           00             
  SPEED_CONTROL_H,         //65 (0x41) Speed control (H)           read/write                  Low-order 8 bits of data actually used              
  SPEED_CONTROL_L          //66 (0x42) Speed control (L)           read/write                 
};

enum class SmartServoOperation : uint8_t
{
  PING       = 1,
  READ       = 2,
  WRITE      = 3,
  REG_WRITE  = 4,
  ACTION     = 5,
  RESET      = 6,
  SYNC_WRITE = 0x83,
};

enum class PositionMode
{
  IMMEDIATE,
  SYNC
};

/**************************************************************************************
 * CONVERSION FUNCTIONS
 **************************************************************************************/

template <typename Enumeration>
constexpr auto toVal(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

/**************************************************************************************
 * MACROS
 **************************************************************************************/

#define REG(enum_val) toVal(enum_val)
#define CMD(enum_val) toVal(enum_val)

#endif /* SMART_SERVO_CONST_H_ */