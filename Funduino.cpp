#include <Arduino.h>
#include <Servo.h>
#include "Funduino.h"
int Funduino::divmod10(int *i) {
  if (*i==0) return 0;
  bool sig=false;
  int w,u=*i,rv;
  if (u<0) {sig=true;u*=-1;} // to positive num
  rv=(int16_t)((6553UL * (uint16_t) u) >> 16);
  w=rv * 10;
  while ((u - w) >= 10) {w += 10; rv++;}
  w = u - w; // zbytek
  if (sig) {
    *i = - rv;
    return - w;
  }
  *i=rv;
  return w;
}

void Funduino::enableDisplay(bool ena) { 
  if (ena)
    _enables |= 1;
  else
    _enables &= 0xFE;
}

void Funduino::clearDisplay() {
  segment(0xFF);
  disbuff[1]=disbuff[2]=disbuff[3]=0xFF;
}

void Funduino::segment(byte segs,byte digit) {
  digitalWrite(FUNDUI_DISPLATCH,LOW); // Konec Latch
  shiftOut(FUNDUI_DISPDATA,FUNDUI_DISPCLOCK,MSBFIRST,segs);
  shiftOut(FUNDUI_DISPDATA,FUNDUI_DISPCLOCK,MSBFIRST,0xF0|(1<<(digit&3))); // nastavi cislo segmentovky
  digitalWrite(FUNDUI_DISPLATCH,HIGH); // ukonci zapis do latch - zobrazeni
  _flags|=1; // display latch is HIGH - go down
}

void Funduino::timerStart(byte i, int time, bool repeat) {
  if (i>=8) return;
  if (time > 0 && time < 32752) { // Start time in range 1..0x7FF0
    _timers_start[i] = _mimi;
    _timers_togo[i] = _mimi + time;
    _timers_repeat[i]=(repeat?time:0);
  } else {
    _timers_start[i] = _timers_togo[i] = _timers_repeat[i] = 0;
  }
}

int Funduino::timerTime(byte i) {
  if (i>=8 || _timers_togo[i] == _timers_start[i])
    return 0; // disabled timer
  if (_timers_togo[i] > _timers_start[i] || _mimi < _timers_start[i])
    return _timers_togo[i] - _mimi;
  else
    return _timers_togo[i] + (65535U - _mimi) + 1;
}

bool Funduino::isTimer(byte i) {
  if (i>=8) return false;
  byte m=(1<<i);
  if ((_timtrig & m)!=0) {
    _timtrig^=m;
    return true;
  }
  return false;
}

bool Funduino::buttPressed(byte i) {
  if (i==0 || i>3) return false;
  byte m=(1<<(i-1));
  if ((_buttrig & m)!=0) {
    _buttrig^=m;
    return true;
  }
  return false;
}

bool Funduino::buttReleased(byte i) {
  if (i==0 || i>3) return false;
  byte m=(0x08<<i); // 0x10 << (i-1)
  if ((_buttrig & m)!=0) {
    _buttrig^=m;
    return true;
  }
  return false;
}

void Funduino::step() {
  static byte prev=0,pm=0;
  byte wb;
  Mili = millis();
  _mimi=((unsigned long)Mili) & 0xFFFF;
  if (pm==(_mimi & 0xFF)) // same millis - skip checks
    return;
  pm=_mimi & 0xFF;
  if ((Mili & 0x1E) != prev) {
    prev = Mili & 0x1E;
    if ((_flags&1)!=0) {
      digitalWrite(FUNDUI_DISPLATCH,LOW); // Konec Latch
      _flags&=0xFE;
    }
    if ((_enables & 1)!=0) {
      byte i=prev;
      if ((i & 2)==0) {
        i=(i >> 2) & 3; // 0=0..3ms,4..7,8..11,12..15 ms
        segment(disbuff[i],i);
      }
    }
  }
  wb = (digitalRead(FUNDUI_BUTT1)?1:0) |
       (digitalRead(FUNDUI_BUTT2)?2:0) |
       (digitalRead(FUNDUI_BUTT3)?4:0);
  if (wb != (_butstate & 7)) {
    for (byte i=0,m=1; i<3; i++,m<<=1) {
      if ((wb & m) != (_butstate & m)) {
        _timers_start[i+8] = _mimi; /* last changed butt 1 */
        _timers_togo[i+8] = _mimi + 20; /* debounce 20ms */
      }
    }
    _butstate = (_butstate & 0x70) | wb;
  }
  // Timers check
  for (byte i=0,m=1;i<11;i++,m=(i==8?1:(m<<1))) {
    if (_timers_togo[i] == _timers_start[i]) continue; // disabled timer
    if (_timers_togo[i] <= _mimi &&
        (_timers_togo[i] > _timers_start[i] ||
         _timers_start[i] > _mimi)) {
      _timers_start[i] = _timers_togo[i] = 0; // Stop timer
      if (i<8) {
        _timtrig |= m; 
        if (_timers_repeat[i] != 0) { // restart timmer
          //timer_start(i,_timrepeat[i],true);
          _timers_start[i] = _mimi;
          _timers_togo[i] = _mimi + _timers_repeat[i];
        }
      } else {
        wb = _butstate >> 4;
        if ((_butstate & m) != (wb & m)) {
          _butstate ^= (m<<4);
          if ((wb & m)==0) // button released now
            _buttrig |= (m<<4);
          else
            _buttrig |= m; // button pressed
        }
      }
    }
  }
}

Funduino::Funduino() {
}

void Funduino::begin() {
  _enables=_flags=_buttrig=_timtrig=0;
  _butstate=0x77; // All released
  disbuff[0]=disbuff[1]=disbuff[2]=disbuff[3]=0xFF;
  pinMode(FUNDUI_DISPLATCH,OUTPUT);
  pinMode(FUNDUI_DISPCLOCK,OUTPUT);
  pinMode(FUNDUI_DISPDATA,OUTPUT); // piny pro segmentovku
  digitalWrite(FUNDUI_DISPLATCH,LOW);
  pinMode(FUNDUI_LED1,OUTPUT);
  pinMode(FUNDUI_LED2,OUTPUT);
  pinMode(FUNDUI_LED3,OUTPUT);
  pinMode(FUNDUI_LED4,OUTPUT);
  digitalWrite(FUNDUI_LED1,HIGH);
  digitalWrite(FUNDUI_LED2,HIGH);
  digitalWrite(FUNDUI_LED3,HIGH);
  digitalWrite(FUNDUI_LED4,HIGH);
  Mili = millis();
  memset(_timers_start,0,sizeof(_timers_start));
  memset(_timers_togo,0,sizeof(_timers_togo));
  memset(_timers_repeat,0,sizeof(_timers_repeat));
}

float Funduino::mapf(float x, float a, float b, float c, float d) {
    float f=x/(b-a)*(d-c)+c;
    return f;
}

float Funduino::trimVolt() {
  return 0.0049 * analogRead(A0);
}

void Funduino::dispDigit(byte val, byte pos, bool digit) {
  enableDisplay(true);
  disbuff[pos & 3] = digit?DisTable[val & 0x0F]:val;
}


void Funduino::dispInt(int val,byte fmt,byte minchars) {
  int i;
  enableDisplay(true);
  if (fmt==HEX) {
    disbuff[0]=DisTable[((uint16_t)val)>>12];
    disbuff[1]=DisTable[(((uint16_t)val)>>8)&0x0F];
    disbuff[2]=DisTable[(((uint16_t)val)>>4)&0x0F];
    disbuff[3]=DisTable[((uint16_t)val)&0x0F];
  } else if (fmt==BIN) {
    disbuff[0]=DisTable[(((uint16_t)val)>>3)&1];
    disbuff[1]=DisTable[(((uint16_t)val)>>2)&1];
    disbuff[2]=DisTable[(((uint16_t)val)>>1)&1];
    disbuff[3]=DisTable[((uint16_t)val)&1];
  } else if (fmt==OCT) {
    disbuff[0]=DisTable[(((uint16_t)val)>>9)&0x07];
    disbuff[1]=DisTable[(((uint16_t)val)>>6)&0x07];
    disbuff[2]=DisTable[(((uint16_t)val)>>3)&0x07];
    disbuff[3]=DisTable[((uint16_t)val)&0x07];
  } else { // asi DECIMAL   
    if (val > 9999) {
      disbuff[0]=disbuff[1]=disbuff[2]=disbuff[3]=0x10; // 9.9.9.9. 
      return;
    } else if (val < -999) {
      disbuff[0]=0xBF; // -9.9.9.
      disbuff[1]=disbuff[2]=disbuff[3]=0x10; 
      return;
    }
    if (val < 0) {
      disbuff[0]=0xBF; // -
      i=-val;
    } else {
      i=val;
    }
    disbuff[3]=DisTable[divmod10(&i)] & 0x7F; // des. tecka
    if (i)
      disbuff[2]=DisTable[divmod10(&i)];
    else
      disbuff[2]=minchars<2?0xFF:DisTable[0]; // prazdno / 0
    if (i) 
      disbuff[1]=DisTable[divmod10(&i)];
    else
      disbuff[1]=minchars<3?0xFF:DisTable[0]; // prazdno / 0
    if (i) 
      disbuff[0]=DisTable[divmod10(&i)];
    else if (val >= 0) // uz je tam minus !!!
      disbuff[0]=minchars<4?0xFF:DisTable[0]; // prazdno / 0
  }
}

void Funduino::dispFloat(float val) {
  int ival,decpoint=3;
  if (val < 0.0) {
    if (val >= -9.99) {
      ival=val * 100; // -9.99 -> -999
      decpoint=1;
    } else if (val >= -99.9) {
      ival=val * 10; // -99.9 -> -999
      decpoint=2;
    } else if (val >= -999.0) {
      ival=val;
    } else {
      dispInt(-1000);
      return;
    }
  } else {
    if (val <= 9.999) {
      ival=val * 1000; // 9.999 -> 9999
      decpoint=0;
    } else if (val <= 99.99) {
      ival=val * 100; // 99.99 -> 9999
      decpoint=1;
    } else if (val <= 999.9) {
      ival=val * 10; // 999.9 -> 9999
      decpoint=2;
    } else if (val <= 9999.0) {
      ival=val;
    } else {
      dispInt(10000);
      return;
    }
  }
  dispInt(ival,DEC,4 - decpoint);
  if (decpoint!=3) {
    disbuff[3]|=0x80;
    disbuff[decpoint]&=0x7F;
  }
}

void Funduino::display(float val) {dispFloat(val);}
void Funduino::display(double val) {dispFloat((float)val);}
void Funduino::display(int val) {dispInt(val,DEC,1);}

bool Funduino::butt(byte button) {
  if (button==2)
    return (_butstate & 0x20) == 0;
  else if (button==3)
    return (_butstate & 0x40) == 0;
  return (_butstate & 0x10) == 0;
}

byte Funduino::buttons() {
  return _butstate >> 4;
}

bool Funduino::butt1() {return (_butstate & 0x10) == 0;}
bool Funduino::butt2() {return (_butstate & 0x20) == 0;}
bool Funduino::butt3() {return (_butstate & 0x40) == 0;}

constexpr const unsigned char Funduino::DisTable[16] = 
  {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,0x83,0xC6,0xA1,0x86,0x8E};
