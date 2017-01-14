#ifndef EL_COIL_H
#define EL_COIL_H
#include <Arduino.h>


class ELCOIL {
public:
  ELCOIL(uint8_t line, byte pos);
  ~ELCOIL();
  
  void on();
  void off();
  void dFlash(); //dFlash sets the coil to "on" each time it's called for 4 times. Then it sets the coil to "off" for 4 cycles. 
  byte getPos(); //Returns pos

private:
  unsigned long cycles; //how many cycles have passed since 
  byte pos; //0 right arm, 1 left arm, 2 right leg 3 left leg
  uint8_t line;
  
    
  };
  
  ELCOIL::ELCOIL(uint8_t line, byte pos){
    pinMode(line, OUTPUT); //make that pin an OUTPUT
    this->pos = pos;
    this->cycles = 0;
  }

  ELCOIL::~ELCOIL(){};// nothing to destruct! 
  
  void ELCOIL::on() {
    digitalWrite(this->line,HIGH);  
  }
  void ELCOIL::off() {
    digitalWrite(this->line,LOW);  
  }

  void ELCOIL::dFlash() {
    if (this->cycles < 10000) {
      this->on();
    } else if (this->cycles < 20000){
      this->off();
    } else {
      this->cycles = 0;
    }
  }

  byte  ELCOIL::getPos() {
    return this->pos;
  }
    
// end of class
#endif
