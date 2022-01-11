#include "Braccio++.h"

int selected_motor = 0;

void setup() {
  // Call Braccio.begin() for default menu or pass a function for custom menu
  Braccio.begin();
  Serial.begin(115200);
  while(!Serial){}
  Serial.println("Testing the motor angular movement!");
}

void loop() {
  Serial.println("Choose the motor 1 to 6 to test the connection:");
  while((Serial.available() <= 0)){};
  int selected_motor = Serial.parseInt();
  for (float i = 0.0; i <= 180.0; i+=10.0){
    Braccio.move(selected_motor).to(i);
    Serial.println("Current angle: " + String(i));
    delay(100);
  }
}
