/*
 * Priklad pouziti knihovny Funduino
 * Na displej zobrazuje napeti na A0 - potenciometr na desce
 * První tlačítko přepíná zobrazení mezi napětím A0 a úhlem serva
 * Další tlačítka pomocí autorepeat - časovačů nastavují servo
 */

#include <Funduino.h>
#include <Servo.h>

Funduino F = Funduino();
Servo FS1 = Servo();
void setup() {
  F.begin();
  FS1.attach(FUNDUI_SERVO1);
  F.clearDisplay();
  F.dispDigit(0x0F,0);
  //   _a_
  // f|   |b
  //   -g-
  // e|   |c
  //   -d- .
  //
  //   .gfedcba = .-',_,'^
  // 0b11111111 0b11111111
  F.dispDigit(0b10111111,1,false); // - 
  F.dispDigit(0b11000111,1,false); // L
  F.dispDigit(0b00001010,1,false); // P.
}

void loop() {
  static byte angle=90;
  static bool mode=false;
  F.step();
  if (F.Mili>1000) { // Sekundu ceka na zacatku
    byte v=F.Mili & 0xFF;
    if (F.buttPressed(1)) {
      F.led(1);
      mode=!mode;
    } else if (F.buttReleased(1)) 
      F.led(1,false);
    if ((v & 0xFF) == 0) { // kazdych 256 milisekund cca 4 x za sekundu
      if (mode)
        F.dispInt(angle,DEC);
      else
        F.dispFloat(F.trimVolt());
    }
    bool changed=false;
    static byte repeatskip=0;
    if (F.buttPressed(2)) {
      angle++; // to left
      F.timerStart(0,100); // 10 per sec autorepeat
      repeatskip=6;
      F.led(2);
      changed=true;
    } else if (F.buttReleased(2)) {
      F.timerStop(0);
      F.led(2,false);
    } else if (F.isTimer(0)) {
      if (repeatskip)
        repeatskip--;
      else {
        angle++;
        changed=true;
      }
    }
    if (F.buttPressed(3)) {
      if ((F.buttons()&6)==0) // both buttons
        angle=90;
      else
        angle--; // to right
      F.timerStart(1,100); // 10 per sec autorepeat
      repeatskip=6;
      F.led(3);
      changed=true;
    } else if (F.buttReleased(3)) {
      F.timerStop(1);
      F.led(3,false);
    } else if (F.isTimer(1)) {
      if (repeatskip)
        repeatskip--;
      else {
        angle--;
        changed=true;
      }
    }
    if (changed) {
      angle=constrain(angle,1,179);
      FS1.write(angle);
    }
  }
}
