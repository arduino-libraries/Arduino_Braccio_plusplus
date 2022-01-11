/* This example demonstrates how to use the motor
 * API of the Braccio++ to directly control the
 * angle for each smart servo motor.
 */

#include <Braccio++.h>

void setup()
{
  Serial.begin(115200);
  while(!Serial){}

  /* Call Braccio.begin() for default menu
   * or pass a function for custom menu.
   */
  Braccio.begin();

  Serial.println("Testing motor angular movement!");
}

void loop()
{
  Serial.println("Choose motor to test (1 - 6):");
  Serial.println(">> ");

  while((Serial.available() <= 0)) { }
  int const selected_motor = Serial.parseInt();
  while(Serial.read() != '\n') { }

  if (selected_motor < 1 || selected_motor > 6) {
    Serial.println("Error, wrong motor id, choose motor id between 1 and 6");
    return;
  }

  for (float i = 0.0; i <= 180.0; i+=10.0) {
    Braccio.move(selected_motor).to(i);
    Serial.println("Current angle: " + String(i));
    delay(100);
  }
}
