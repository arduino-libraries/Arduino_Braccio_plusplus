#include <Braccio++.h>

auto elbow = Braccio.get(1);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Braccio.begin();
}

void loop() {

  if (elbow) {
    elbow.move().to(122.7).in(1s);
    delay(1000);
    elbow.move().to(11.9).in(100ms);
  }
  Serial.println(elbow.position());

  // Should move all the motors at once
  Braccio.moveTo(10.0, 20.0, 30.0, 40.0, 50.0, 60.0);

  // You can choose the speed beforehand with
  Braccio.speed(FAST); // could be MEDIUM or SLOW too

  float a1, a2, a3, a4, a5, a6;
  Braccio.positions(a1, a2, a3, a4, a5, a6);
  Serial.println("Motor positions are: " + String(a1) + " " + String(a2) + " " + String(a3) + " " + String(a4) + " " + String(a5) + " " + String(a6));

  // Can also use the more compact notation
  float values[6];
  Braccio.positions(values);
}
