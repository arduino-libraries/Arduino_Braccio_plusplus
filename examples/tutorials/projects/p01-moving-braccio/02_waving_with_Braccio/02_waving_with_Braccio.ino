#include "Braccio++.h"

#define BUTTON_ENTER   6

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
float wavePos[6] = {180.0, 250.0, 145.0, 150.0, 150.0, 90.0};

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

// Waving whit Wrist pitch
void loop() {
  int pressedKey = Braccio.getKey();

  if (pressedKey == BUTTON_ENTER)
    movement = true; // Trigger joints' movements

  if (movement) {
    Braccio.moveTo(wavePos[0], wavePos[1], wavePos[2], wavePos[3], wavePos[4], wavePos[5]);
    delay(1000);

    for (int i = 1; i <= 10; i++) {
      wristPitch.move().to(100.0f);
      delay(300);
      wristPitch.move().to(190.0f);
      delay(600);
      wristPitch.move().to(145.0f);
      delay(300);
    }

    movement = false; // Stop joints' movements
  }
}
