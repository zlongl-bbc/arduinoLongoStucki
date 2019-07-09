#include <TimerOne.h>
#include <Wire.h>
#include <MultiFuncShield.h>

 int counter1 = 0;
 int counter2 = 0;
 
void setup() {
 Serial.begin(9600);
 Timer1.initialize();
 MFS.initialize(&Timer1);
}

void loop() {
 byte btn = MFS.getButton();
 
 if (btn)
 {
   byte buttonNumber = btn & B00111111;
   byte buttonAction = btn & B11000000;
  
   Serial.print("BUTTON_");
   Serial.write(buttonNumber + '0');
   Serial.print("_");
  
   if (btn == BUTTON_1_PRESSED)
   {
    counter1 = counter1 + 1;
    MFS.write(counter1);
   }
   
   if (btn == BUTTON_2_PRESSED)
   {
    counter2 = counter1;
    counter1 = 0;
   }
   
   if (btn == BUTTON_3_PRESSED)
   {
    MFS.write(counter1 + counter2);
    MFS.beep();
   }
   
   for(int i; i < counter1+counter2; i++)
   {
    MFS.beep();
    delay(500);
   }
 }
}
