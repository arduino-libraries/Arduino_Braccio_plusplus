#include "Braccio++.h"

#define BUTTON_ENTER    6

// Braccio ++ joints
auto gripper    = Braccio.get(1);
auto wristRoll  = Braccio.get(2);
auto wristPitch = Braccio.get(3);
auto elbow      = Braccio.get(4);
auto shoulder   = Braccio.get(5);
auto base       = Braccio.get(6);

/* Variables */
// initialAngles[6] = {gripper, wristRoll, wristPitch, elbow, shoulder, base}
float homePos[6] = {160.0, 150.0, 220.0, 220.0, 100.0, 180.0};

bool movement = false; // Flag to initialize joints' movements

void setup() {
  Braccio.begin();
  delay(500); // Waits for the Braccio initialization

  // You can choose the speed beforehand with
  Braccio.speed(SLOW); // could be FAST or MEDIUM or SLOW

  // Set motors initial angle
  // Should move all the motors at once
  Braccio.moveTo(homePos[0], homePos[1], homePos[2], homePos[3], homePos[4], homePos[5]);
  delay(500);
}

void loop() {
  int pressedKey = Braccio.getKey();

  if (pressedKey == BUTTON_ENTER)
    movement = true; // Trigger joints' movements

  if (movement) {
    Braccio.moveTo(160.0, 150.0, 220.0, 220.0, 100.0, 270.0);
    delay(1000);

    for (int i = 1; i <= 10; i++) {
      wristPitch.move().to(190.0f);
      delay(200);
      wristPitch.move().to(250.0f);
      delay(400);
      wristPitch.move().to(220.0f);
      delay(200);
    }

    movement = false; // Stop joints' moviments
  }
}
