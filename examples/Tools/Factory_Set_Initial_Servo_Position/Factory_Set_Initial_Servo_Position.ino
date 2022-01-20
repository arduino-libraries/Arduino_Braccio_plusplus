/* This example is used for setting all servos to the
 * desired initial servo position prior to mounting the
 * servo horns.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

class InitialServoPosition
{
public:
  InitialServoPosition(int const motor_id, float const motor_angle_deg)
  : _motor_id{motor_id}
  , _motor_angle_deg{motor_angle_deg}
  { }

  inline int id() const { return _motor_id; }
  inline float angle() const { return _motor_angle_deg; }

private:
  int const _motor_id;
  float const _motor_angle_deg;
};

/**************************************************************************************
 * CONSTANT
 **************************************************************************************/

static std::array<InitialServoPosition, 6> const INITIAL_SERVO_POSITION =
{
  InitialServoPosition{1, SmartServoClass::MAX_ANGLE / 2.0f},
  InitialServoPosition{2, SmartServoClass::MAX_ANGLE / 2.0f},
  InitialServoPosition{3, SmartServoClass::MAX_ANGLE / 2.0f},
  InitialServoPosition{4, SmartServoClass::MAX_ANGLE / 2.0f},
  InitialServoPosition{5, SmartServoClass::MAX_ANGLE / 2.0f},
  InitialServoPosition{6, 90.0f},
};

/**************************************************************************************
 * FUNCTIONS
 **************************************************************************************/

bool set_initial_servo_position(int const id, float const target_angle)
{
  Serial.print("Connecting .... ");
  for (; !Braccio.get(id).connected(); delay(10)) { }
  Serial.println("OK.");
  delay(500);

  Serial.print("Disengaging ... ");
  Braccio.get(id).disengage();
  Serial.println("OK.");
  delay(500);

  Serial.print("Engaging ...... ");
  Braccio.get(id).engage();
  Serial.println("OK.");
  delay(500);

  /* Drive to the position for assembling the servo horn. */
  auto const start = millis();
  auto isTimeout = [start]() -> bool { return ((millis() - start) > 5000); };
  auto isTargetAngleReached = [target_angle, id](float const epsilon) -> bool { return (fabs(Braccio.get(id).position() - target_angle) <= epsilon); };

  static float constexpr EPSILON = 2.0f;

  for ( float current_angle = Braccio.get(id).position();
       !isTargetAngleReached(EPSILON) && !isTimeout();)
  {
    Braccio.get(id).move().to(target_angle).in(200ms);
    delay(250);
  
    char msg[64] = {0};
    snprintf(msg, sizeof(msg), "Angle (Target | Current) = %.2f | %.2f", target_angle, Braccio.get(id).position());
    Serial.println(msg);
  }

  if (!isTargetAngleReached(EPSILON)) {
    Serial.println("Servo did not reach target angle.");
    return false;
  }

  if (isTimeout()) {
    Serial.println("Timeout error.");
    return false;
  }

  return true;
}

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  Serial.begin(115200);
  while(!Serial) { }

  Braccio.begin();
  Braccio.disengage();

  for (auto & servo : INITIAL_SERVO_POSITION)
  {
    Serial.print("Servo ");
    Serial.print(servo.id());
    Serial.print(": Target Angle = ");
    Serial.print(servo.angle());
    Serial.print(" °");
    Serial.println();

    if (!set_initial_servo_position(servo.id(), servo.angle())) {
      Serial.println("ERROR.");
      return;
    }
  }

  Serial.println("SUCCESS : all servos are set to their initial position.");
}

void loop()
{

}
