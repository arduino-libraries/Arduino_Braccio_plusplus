#include <Braccio++.h>

String toMessage(int const input)
{
  static String const message[] =
  { "",
    "LEFT   (Joystick)",
    "RIGHT  (Joystick)",
    "SELECT (Joystick)",
    "UP     (Joystick)",
    "DOWN   (Joystick)",
    "ENTER  (Button)"
  };

  if (input < 7)
    return message[input];
  else
    return String("Error, invalid input value");
}

void setup()
{
  Serial.begin(115200);
  while(!Serial){}

  Braccio.begin();
  Serial.println("Press any button or move the joystick.");
}

void loop()
{
  String const message = toMessage(Braccio.getKey());
  if(message != "")
    Serial.println(message);
  delay(100);
}
