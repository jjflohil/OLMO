#include <Arduino.h>
#include <StepRegulator.h>
#include <Inputs.h>

// Define the setp regulater
// (enable, direction, step)
StepRegulator StepDrive(7,8,9);

// Define inputs
Inputs Inputs0(3,4,10,A2);
Inputs Inputs1(5,6,11,A3);

#define ZeroPin  A1
#define BPMPin   A0
#define MotorPot A3

uint32_t T = 0;
uint16_t Ts = 100;

bool Pulse;
long counter = 0;
float BPM_ref = 80;
float BPM_actP = 0;
float BPM_act = 0;
float BPM_offset = 0;
uint16_t vel_ref = 0;

bool pulse1  = false;
bool pulse1p = false;
uint16_t T_pulse  = 0;
uint16_t T_pulseP = 0;


bool plus  = false;
bool plusP  = false;
bool minus = false;
bool minusP = false;
bool nulPoint = false;
bool nulPointP = false;

int count = 0;
//set timer2 interrupt at 8kHz


ISR(TIMER2_COMPA_vect)
{ //timer1 interrupt 8kHz toggles pin 9
  //generates pulse wave of frequency 8kHz/2 = 4kHz (takes two cycles for full wave- toggle high then toggle low)
  counter++;
  if(counter == 41){
    counter = 0;
    PORTB = PORTB xor B0000010;
  }
}

void debug(bool* PhotoT)
{
  //Some timed loop with 100ms loop time
  if(millis() > (T+Ts)){
    T = millis();
    //Serial.println(digitalRead(10));
  }
  //Some printing of values to debug code
  if(PhotoT[1]){Serial.println(count);count++;}
  if(PhotoT[2]){Serial.println(count);count--;}
}


void setup() {
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  
  //Read double (OnOFfOn) Switch [State1 P1 N1 State2 P2 N2]
  bool* switch0 = Inputs0.ReadOnOffOn();
  //Read potmeter
  int Potresult = Inputs0.ReadPotMeter(1023,0,255);
  // Read Phototransistor
  bool* PhotoT = Inputs0.ReadPhoto();

  //StepDrive.ControlBPM(BPM_ref);
  BPM_act = StepDrive.GetBPM();

  debug(PhotoT);
}
