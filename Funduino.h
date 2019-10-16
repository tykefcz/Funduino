#if !defined(FNDUINO_H)
#if !defined(ARDUINO_AVR_UNO)
  #error "This library only supports Arduino Uno boards"
#endif
#define FNDUINO_H 1

#define FUNDUI_DISPLATCH 4
#define FUNDUI_DISPCLOCK 7
#define FUNDUI_DISPDATA 8

#define FUNDUI_BUTT1  A1
#define FUNDUI_BUTT2  A2
#define FUNDUI_BUTT3  A3

#define FUNDUI_LED1 13
#define FUNDUI_LED2 12
#define FUNDUI_LED3 11
#define FUNDUI_LED4 10

#define FUNDUI_SERVO1 5
#define FUNDUI_SERVO2 6
#define FUNDUI_SERVO3 9
#define FUNDUI_SERVO4 A5

#define FUNDUI_BUZZER 3
#define FUNDUI_TRIMMER	A0

#define FUNDUI_CONN7A 2
#define FUNDUI_CONN7B A4

class Funduino {
 public:
  static int divmod10(int *i);
  static float mapf(float x, float a, float b, float c, float d);
  static const unsigned char Dis_table[16];

  // data pro segmentovky
  unsigned char disbuff[4];
  long mili;

  void step();
  Funduino();

  void begin();
  void enableDisplay(bool ena=true);
  void segment(byte segs, byte digit=0);
  void clearDisplay();
  float trimVolt();
  void dispInt(int val,byte fmt=DEC,byte minchars=1);
  void dispFloat(float val);
  void display(float val);
  void display(int val);
  bool butt1();
  bool butt2();
  bool butt3();
  bool butt(byte button);
  byte buttons();
  void timer_start(byte i, int time, bool repeat=true);
  inline void timer_stop(byte i) {timer_start(i,0,false);}
  int timer_time(byte i);
  bool is_timer(byte i);
  bool butt_pressed(byte i);
  bool butt_released(byte i);
  inline void led(byte i,bool on=true) {
    digitalWrite(
       i==2?FUNDUI_LED2:(i==3?FUNDUI_LED3:(i==4?FUNDUI_LED4:FUNDUI_LED1))
      ,on?LOW:HIGH); }
 private:
  byte _enables,_flags,_butstate,_timtrig,_buttrig;
  unsigned int _timers_start[11]={}, /* 8 timers + 3 buttons */
               _timers_togo[11]={},
               _timers_repeat[8]={},
               _mimi;
};
#endif
