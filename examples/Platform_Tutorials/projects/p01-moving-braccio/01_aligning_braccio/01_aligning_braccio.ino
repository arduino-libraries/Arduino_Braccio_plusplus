#include <Braccio++.h>

// Variables
// Braccio ++ joints
auto gripper    = Braccio.get(1);
auto wristRoll  = Braccio.get(2);
auto wristPitch = Braccio.get(3);
auto elbow      = Braccio.get(4);
auto shoulder   = Braccio.get(5);
auto base       = Braccio.get(6);

float initialGripper = 160.0;
float initialBase = 90.0;
float initialAngle = 150.0;
float angles[6];

void setup() {
  Braccio.begin();
  delay(500); // Waits for the Braccio initialization
  
  // Send motors initial angle
  gripper.move().to(initialGripper);
  delay(100);
  wristRoll.move().to(initialAngle);
  delay(100);
  wristPitch.move().to(initialAngle);
  delay(100);
  elbow.move().to(initialAngle);
  delay(100);
  shoulder.move().to(initialAngle);
  delay(100);
  base.move().to(initialBase); 
  delay(100);

  Serial.begin(115200);
  while(!Serial){}
}

void loop() {
  // Fetch the joints positions
  Braccio.positions(angles);

  // Print the joint angles
  Serial.println("************* Joints Angles *************");
  Serial.println("|\tMotor ID\t|\tAngle\t|");
  Serial.println("----------------------------------------");
  Serial.print("| 1 - Gripper\t\t|\t" + String(angles[0]) + "\t|\n" + 
               "| 2 - Wrist Rotation\t|\t" + String(angles[1]) + "\t|\n" +
               "| 3 - Wrist Vertical\t|\t" + String(angles[2]) + "\t|\n" + 
               "| 4 - Elbow\t\t|\t" + String(angles[3]) + "\t|\n" + 
               "| 5 - Shoulder\t\t|\t" + String(angles[4]) + "\t|\n" + 
               "| 6 - Base\t\t|\t" + String(angles[5]) + "\t|\n");
  Serial.println("*****************************************\n\n\n\n\n");
  Serial.println("\n\n\n\n");
  
  delay(1000);
}
