#include "Braccio++.h"

int selected_motor = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial){}
  Serial.println("Testing the motor communication!");
  Braccio.begin();
}

void loop() {
  Serial.println("Choose the motor 1 to 6 to test the connection:");
  while((Serial.available() <= 0)){};
  int motorID = Serial.parseInt();
  
  bool connected = (Braccio.connected(motorID));
  if (connected)
    Serial.println("Communcation with motor " + String(motorID) + " successful");
  else
    Serial.println("Communcation failure - Please check the motor " + String(motorID) + " connection");
  Serial.println();
}
