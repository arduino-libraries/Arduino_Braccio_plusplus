#include "Braccio++.h"

String message = "";

String checkInputs(int input){
  String check[] = { "",
    "Joystick left was moved!",
    "Joystick right was moved!",
    "Joystick select button was pressed!",
    "Joystick up was moved!",
    "Joystick down was moved!",
    "Enter button was pressed!"};
   return check[input];
}

void setup() {
  Serial.begin(115200);
  while(!Serial){}
  Braccio.begin();
  Serial.println("Press any button or move the joystick.");
}

void loop() {
  message = checkInputs(Braccio.getKey());
  if(message != ""){
    Serial.println(message);
    message = "";
  }
  delay(500);
}
