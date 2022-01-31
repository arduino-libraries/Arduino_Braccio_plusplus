#include "Braccio++.h"

#define BUTTON_ENTER   6
#define TIME          10

// Braccio ++ joints
auto gripper    = Braccio.get(1);
auto wristRoll  = Braccio.get(2);
auto wristPitch = Braccio.get(3);
auto elbow      = Braccio.get(4);
auto shoulder   = Braccio.get(5);
auto base       = Braccio.get(6);

// Variables
float initialPos[6] = {0.0, 135.0, 130.0, 130.0, 130.0, 135.0};
float limAngles[1000];
int sizeArray = 0;

bool movement = false; // Flag to initialize joints' movements

void calcAngles(float initial, float final, int joint, int gripper = 0){
  sizeArray = 0;
  
  if(gripper){
    for(float angle = initial; angle <= final; angle++){
      limAngles[sizeArray] = angle;
      sizeArray++;
    }
    for(float angle = final - 1; angle >= initial; angle--){
      limAngles[sizeArray] = angle;
      sizeArray++;
    }
  } else {
    for(float angle = initialPos[joint - 1]; angle <= final; angle++){
      limAngles[sizeArray] = angle;
      sizeArray++;
    }
  
    for(float angle = final-1; angle >= initial; angle--){
      limAngles[sizeArray] = angle;
      sizeArray++;
    }
  
    for(float angle = initial+1; angle <= initialPos[joint - 1]; angle++){
      limAngles[sizeArray] = angle;
      sizeArray++;
    }
  }
}

void setup() {
  Braccio.begin();
  delay(500); // Waits for the Braccio initialization

  // You can choose the speed beforehand with
  Braccio.speed(SLOW); // could be FAST or MEDIUM or SLOW

  // Set motors initial angle
  gripper.move().to(initialPos[0]);
  delay(TIME);
  wristRoll.move().to(initialPos[1]);
  delay(TIME);
  wristPitch.move().to(initialPos[2]);
  delay(TIME);
  elbow.move().to(initialPos[3]);
  delay(TIME);
  shoulder.move().to(initialPos[4]);
  delay(TIME);
  base.move().to(initialPos[5]);
  delay(TIME);
}

void loop() {
  int pressedKey = Braccio.getKey();

  if(pressedKey == BUTTON_ENTER)
    movement = true; // Trigger joints' movements

  if(movement) {
    calcAngles(0.0, 270.0, 6); // Calculates base angles
    for(int i = 0; i <= sizeArray - 1; i++){
      base.move().to(limAngles[i]);
      delay(TIME);
    }

    calcAngles(45.0, 215.0, 5); // Calculates shoulder angles
    for(int i = 0; i <= sizeArray - 1; i++){
      shoulder.move().to(limAngles[i]);
      delay(TIME);
    }

    calcAngles(30.0, 240.0, 4); // Calculates elbow angles
    for(int i = 0; i <= sizeArray - 1; i++){
      elbow.move().to(limAngles[i]);
      delay(TIME);
    }

    calcAngles(30.0, 240.0, 3); // Calculates wrist pitch angles
    for(int i = 0; i <= sizeArray - 1; i++){
      wristPitch.move().to(limAngles[i]);
      delay(TIME);
    }

    calcAngles(0.0, 270.0, 2); // Calculates wrist roll angles
    for(int i = 0; i <= sizeArray - 1; i++){
      wristRoll.move().to(limAngles[i]);
      delay(TIME);
    }

    calcAngles(0.0, 70.0, 1, 1); // Calculates gripper angles
    for(int i = 0; i <= sizeArray - 1; i++){
      gripper.move().to(limAngles[i]);
      delay(TIME);
    }
    
    movement = false; // Stop joints' moviments
  }
}


//void loop() {
//  int pressedKey = Braccio.getKey();
//
//  if(pressedKey == BUTTON_ENTER)
//    movement = true; // Trigger joints' movements
//
//  if(movement) {
//    for(float angle = initialAngle; angle <= 270.0; angle++){
//      base.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//      
//    for(float angle = 270.0; angle >= 0.0; angle--){
//      base.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = 0.0; angle <= initialAngle; angle++){
//      base.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = initialAngle; angle <= 215.0; angle++){
//      shoulder.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//      
//    for(float angle = 215.0; angle >= 45.0; angle--){
//      shoulder.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = 45.0; angle <= initialAngle; angle++){
//      shoulder.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = initialAngle; angle <= 250.0; angle++){
//      elbow.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//      
//    for(float angle = 250.0; angle >= 30.0; angle--){
//      elbow.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = 30.0; angle <= initialAngle; angle++){
//      elbow.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = initialAngle; angle <= 250.0; angle++){
//      wristPitch.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//      
//    for(float angle = 250.0; angle >= 30.0; angle--){
//      wristPitch.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = 30.0; angle <= initialAngle; angle++){
//      wristPitch.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = initialAngle; angle <= 270.0; angle++){
//      wristRoll.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//      
//    for(float angle = 270.0; angle >= 0.0; angle--){
//      wristRoll.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = 0.0; angle <= initialAngle; angle++){
//      wristRoll.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//
//    for(float angle = initialGripper; angle <= 70.0; angle++){
//      gripper.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//      
//    for(float angle = 70.0; angle >= initialGripper; angle--){
//      gripper.move().to(angle).in(100ms);
//      delay(TIME);
//    }
//    
//    movement = false; // Stop joints' moviments
//  }
//}
