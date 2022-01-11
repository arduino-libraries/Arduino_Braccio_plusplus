#include "Braccio++.h"

void setup()
{
  Serial.begin(115200);
  while(!Serial) { }

  Braccio.begin();
  Serial.println("Testing motor communication!");
  Serial.println("Choose motor 1 to 6 to test the connection");
  Serial.print(">> ");
}

void loop()
{
  while(Serial.available() <= 0) { }
  int const motorID = Serial.parseInt();
  while(Serial.read() != '\n') { }

  if (motorID < 1 || motorID > 6) {
    Serial.println("Error, wrong motor id, choose value between 1 and 6");
    Serial.print(">> ");
    return;
  }
  
  if (Braccio.connected(motorID))
    Serial.println("Communcation with motor " + String(motorID) + " successful");
  else
    Serial.println("Communcation failure - Please check the motor " + String(motorID) + " connection");

  Serial.print(">> ");
}
