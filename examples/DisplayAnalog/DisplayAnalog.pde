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
}

void loop() {
  static byte angle=90;
  static bool mode=false;
  F.step();
  if (F.mili>1000) { // Sekundu ceka na zacatku
    byte v=F.mili & 0xFF;
    if (F.butt_pressed(1)) {
      F.led(1);
      mode=!mode;
    } else if (F.butt_released(1)) 
      F.led(1,false);
    if ((v & 0xFF) == 0) { // kazdych 256 milisekund cca 4 x za sekundu
      if (mode)
        F.dispInt(angle,DEC);
      else
        F.dispFloat(F.trimVolt());
    }
    bool changed=false;
    static byte repeatskip=0;
    if (F.butt_pressed(2)) {
      angle++; // to left
      F.timer_start(0,100); // 10 per sec autorepeat
      repeatskip=6;
      F.led(2);
      changed=true;
    } else if (F.butt_released(2)) {
      F.timer_stop(0);
      F.led(2,false);
    } else if (F.is_timer(0)) {
      if (repeatskip)
        repeatskip--;
      else {
        angle++;
        changed=true;
      }
    }
    if (F.butt_pressed(3)) {
      if ((F.buttons()&6)==0) // both buttons
        angle=90;
      else
        angle--; // to right
      F.timer_start(1,100); // 10 per sec autorepeat
      repeatskip=6;
      F.led(3);
      changed=true;
    } else if (F.butt_released(3)) {
      F.timer_stop(1);
      F.led(3,false);
    } else if (F.is_timer(1)) {
      if (repeatskip)
        repeatskip--;
      else {
        angle--;
        changed=true;
      }
    }
    if (changed) {
      angle=constrain(angle,10,170);
      FS1.write(angle);
    }
  }
}
