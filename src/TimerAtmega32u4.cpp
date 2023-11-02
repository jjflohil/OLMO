#include "TimerAtmega32u4.h"
#include "arduino.h"

// The function can be used for On/Off Switches, On/Off/On switches and potmeters
// For On/Off switch Give the SwitchUpPin the same number as the SwitchDnPin
// The data is read by a multiplexer

TimerAtmega32u4::TimerAtmega32u4()
{

}

void TimerAtmega32u4::SetupTimer(int factor)
{
  cli();//stop interrupts
   //set timer1interrupt at 8kHz
  TCCR1A = 0;// set entire TCCR2A register to 0
  TCCR1B = 0;// same for TCCR2B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1MHz increments
  OCR1A = factor;// = (16*10^6) / (1000000 *1) - 1 (must be <256)
  // turn on CTC mode
  TCCR1A |= (1 << WGM11);
  // Set CS21 bit for 64 prescaler
  TCCR1B |= (1 << CS10);  
  TCCR1B |= (0 << CS11);  
  TCCR1B |= (0 << CS12);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}



