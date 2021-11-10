//simple tachometer sending data to serial port
//it only prints rpm's of the last measure

#include <pcint.h>

#define SensorPin 2

volatile unsigned long t=0;
volatile unsigned long ot=0;
unsigned int rpm=0;

void tick() {
   ot=t;
   t=millis();
}

void setup() {
   Serial.begin(9600);
   while(!Serial);
   pinMode(SensorPin,INPUT_PULLUP);
   PCattachInterrupt<SensorPin>(tick,RISING);
}

void loop() {
   if((millis()-t)>300) rpm=0;
   else if (t!=ot) rpm=60000.0/(float)(t-ot);
   Serial.print(rpm);
   Serial.print(" ");
   Serial.println(t-ot);
   delay(100);
}
