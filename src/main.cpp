#include <Arduino.h>
#include <AccelStepper.h>
#include <Inputs/Inputs_Multiplex.h>
#include "MIDIUSB/MIDIUSB.h"
#include <Wire.h>
#include <PWM/Adafruit_PWMServoDriver.h>
#include "ProcessMenu.h"

ProcessMenu DispMenu(0,1);
// A0,A1,A2,A3  Channel pins Multiplexers
// 1 = Display
// 2 = Display
// 7 = Stepper Step
// 8 = Stepper Direction
// 9 = Stepper Enable

// Aux buttons
// 0 = -
// 1 = +
// 2 = <
// 3 = >

// To be arranged paramters
bool AutoSync     = true;
long TimeCDPos    = 0;
long TimeMidiPos  = 0;
int midi_CH       = 2;
int Preview_CH    = 1;
int N_Laser       = 16;
int DebounceTimeMs = 150;

//Define PWM controllers
Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver LED_R = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver LED_G = Adafruit_PWMServoDriver(0x42);
Adafruit_PWMServoDriver LED_B = Adafruit_PWMServoDriver(0x43);

//Define vectors
int NightRider[31] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
bool Laserstate[16] = {1,1,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
bool PickNote[16];
//StartStop vectors
int FreqGainStart[10] = {0,5,15,25,32,38,34,30,25,20};
int FreqGainStop[10] = {20,20,20,15,15,15,10,10,5,5};
//Potmeter vector
int PotAll[16];
//Initialize Stepper Freq (160Hz = 100BPM)
int Freq  = 160;
int FreqP = 160;
int FreqCnt = 0;
int FreqOut = 0;
int FreqOutP = 0;

//StartStop Commmand
bool StartCD = false;
bool StopCD = false;

// CD offset manual
int OffsetManual = 0;

//Define MIDI Notes
int Notes[16] = {0x24,0x25,0x26,0x27,0x28,0x29,
                 0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
                 0x30,0x31,0x32,0x33};  
              
//int ActiveNote[16]    = {0,0,0,2, 2,2,1,3, 6,6,10,15, 12,11,9,14};
//int ActiveNoteTmp[16] = {0,0,0,2, 2,2,1,3, 6,6,10,15, 12,11,9,14};
int ActiveNote[16]    = {0,0,8,9, 0,1,0,1, 2,3,2,3, 6,7,6,7};
int ActiveNoteTmp[16] = {0,0,8,9, 0,1,0,1, 2,3,2,3, 6,7,6,7};
//char* NoteCh[16] = {"C1","C#1","C1","C1","C1","C1","C1","C1"
//                    "C1","C1","C1","C1","C1","C1","C1","C1",};

bool DispCurrentNote;
int NoteChanged;

// Define inputs (sw_up,sw_dn,PhTr,Pot,Aux,ChannelPins)
Inputs_Multiplex Inputs(14,15,16,A10,8,A3,A2,A1,A0);

//State Machine Stepper
int State = 0;      //0 == Stopped ; 1 == Stopping ; 2 = Starting ; 3 = Running
//Menu parameters of the system
int Menu = 0;
int SubMenu;
int Clock = 0;
int Sync = 0;
long T_SubMenu = 0;
long T_FreqTimer = 0;

//TimerLoop_0 (button read)
uint32_t T_0 = 0;uint16_t Ts_0 = 1;
//TimerLoop_1 (Stepper motor controller)
uint32_t T_1 = 0;uint16_t Ts_1 = 20;
//TimerLoop_2 (button debounce) // todo not needed maybe use timer for other purposes
//uint32_t T_2 = 0;uint16_t Ts_2 = 50;
//TimerLoop_3 (Display)
uint32_t T_3 = 0;uint16_t Ts_3 = 300;
//TimerLoop_4 (Menu Button Timers)
uint32_t T_4 = 0;uint16_t Ts_4 = 200;

int KittTimer = 0;

//Speed control parameters
int MIDI_Pos = 0;
int BPMTimeDiff = 0;
int CDPos    = 0;
int CDOffset = 0;
int BPM = 0;
int BPM_p = 0;

//LED Brightness
int Brightness =  4000; 

//Midi input parameters
midiEventPacket_t midiIn;
const byte MidiStop   = 0xFC;
const byte MidiStart  = 0xFA;
const byte MidiClock  = 0xf8;
uint32_t  MidiTime    = 0;
uint32_t DebounceTime[16];

//==== TEMP MIDI SHIZZLE (Maak hier een library van)
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}
void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
void TimerPulse(){
  midiEventPacket_t TimerPulse = {0x0F, 0xF8, 0, 0};
  MidiUSB.sendMIDI(TimerPulse);
}

//Buttons variables
byte regB[16];
struct Button
{
  bool PF[16];
  bool NF[16];
  bool State[16];
};
Button Up;
Button Dn;
Button Pt;
Button Aux;

//========================================//
void setup() {
  DispMenu.DefMenu(0,4,"PLAY","CLK ","SYNC","NOTA");
  DispMenu.DefMenu(1,2,"EXT ","INT ","","");
  DispMenu.DefMenu(2,2,"HAND","AUTO","","");

  pwm1.begin();
  LED_R.begin();
  LED_G.begin();
  LED_B.begin();

  pwm1.setPWMFreq(1000);  // This is the maximum PWM frequency
  LED_R.setPWMFreq(1000);  // This is the maximum PWM frequency  
  LED_G.setPWMFreq(1000);  // This is the maximum PWM frequency
  LED_B.setPWMFreq(1000);  // This is the maximum PWM frequency
  
  for(int i=0;i<16;i++){
    pwm1.setPWM(i, 4096, 0);
    LED_R.setPWM(i, 4096, 0);
    LED_G.setPWM(i, 4096, 0);
    LED_B.setPWM(i, 4096, 0);
  }

  //Serial.begin(9600);
  DDRF = B11111111;

  pinMode(8,INPUT_PULLUP);

  //Stepper pins
  pinMode(7,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(9,OUTPUT);
  digitalWrite(6,HIGH);
  digitalWrite(9,HIGH);
  
  //Run startup effect
  for(int i=0;i<30;i++){
    for(int j=0;j<16;j++){
      LED_R.setPWM(j, 4096,0);
      pwm1.setPWM(j,4096,0);
    }
    LED_R.setPWM(NightRider[i],0,Brightness);
    pwm1.setPWM(NightRider[i],0,4096);
    delay(50);
  }
  LED_R.setPWM(NightRider[0],0,Brightness);
  pwm1.setPWM(NightRider[0],0,4096);
}

void loop() {

  //==== Read inputs and process all the MIDI logic ====//
  if(millis() > (T_0+Ts_0)){
    T_0 = millis();
    

  //==== Process NOT over all channels
    
    // Mode selection of the system
    if(State == 0 && Aux.PF[3]){
      Menu = (Menu+1) * (Menu<3);
    }
    // Clock type selection of the system (0 = Ext, 1 = Int )
    if(Menu==1){
      if(Aux.PF[0]){Clock = 0;SubMenu=1;T_SubMenu = millis();}
      else if(Aux.PF[1]){Clock = 1;SubMenu=2;T_SubMenu = millis();}
    }
    //Select sync method (0 = Ext, 1 = Int )
    if(Menu==2){
      if(Aux.PF[1]){AutoSync = 0;SubMenu=1;T_SubMenu = millis();}
      if(Aux.PF[0]){AutoSync = 1;SubMenu=2;T_SubMenu = millis();}
    }

    if(Menu == 1 && Dn.State[0]){BPM = OffsetManual;}
    if(Aux.PF[3] || millis()>(T_SubMenu+1000)){SubMenu = 0;}

    if(Clock == 1){
      if(Up.PF[0]){
        Freq = 160;
        StartCD = true;
        FreqCnt = 0;
        State = 3;
      }
      else if(Dn.PF[0]){
        StopCD = true;
        FreqCnt = 0;
        State = 2;
      }
      //Pick Freq with + and -
      if(Menu==0 && Freq > 0 && Freq < 800){
        Freq = Freq + (Aux.PF[1] - Aux.PF[0]) * 32 / 10;
        if(Aux.PF[1] || Aux.PF[0]){
          T_FreqTimer = millis();
        }
      }
    }
    //Read midi input from computer
    midiIn = MidiUSB.read();
    // Handle MIDI clock and adjust speed of motor
    if(midiIn.byte1 == MidiStart && !StopCD && Menu==0){
      StartCD = true;
      FreqCnt = 0;
      FreqOut = 0;
      MidiTime = millis();
      MIDI_Pos = 0;
      CDPos = 0;
      State = 3;
      CDOffset = 0;
    }
    // MIDI clock transition to "Stopping"
    else if(midiIn.byte1 == MidiStop){
      State = 2;
    }
    // MIDI clock is running
    else if(midiIn.byte1 == MidiClock){
      //Calculate the BPM time difference
      if(millis() > MidiTime){
        BPMTimeDiff = millis()-MidiTime;
      }
      MidiTime = millis();
      if((MIDI_Pos % 3) == 0){
        TimeMidiPos = millis();
      }
      MIDI_Pos = MIDI_Pos+1;
      // MIDI in: 96Pulses per quarter note
      if(MIDI_Pos == 96 && AutoSync){
        MIDI_Pos = 0;
      }
    }
    //Generate Offset
    if(Dn.State[0] && !AutoSync){
      OffsetManual = map(PotAll[0],0,1023,-10,10);
      if(OffsetManual > -2 && OffsetManual < 2){
        OffsetManual = 0;
      }
    }
    else{
      OffsetManual = 0;
    }
    //Iterate CD position
    if(Pt.PF[0]){
        CDPos = min(31,CDPos+1);
        TimeCDPos = millis();
    }
    // CD passes through zero position
    if(Pt.PF[1] && AutoSync){
      CDPos = 0;
      if(MIDI_Pos>=50){
        CDOffset = MIDI_Pos-96;
      }
      else if(MIDI_Pos < 50){
        CDOffset = MIDI_Pos;
      }      
    }
    
    //====If manual sync is active
    if(!AutoSync){
      CDOffset = (TimeMidiPos - TimeCDPos);
      if(CDOffset>10){
        CDOffset = 10;
      }
      if(CDOffset<-10){
        CDOffset = -10;
      }
    }
    //==== State change stopping to stopped
    if(State == 2 && CDPos >= 26){
      StopCD = true;
      FreqCnt = 0;
      State = 1;
    }
    //==== Reset all when at zero (State = stopped)
    if(State == 1  and Pt.NF[1] ){
      State = 0;
      CDPos = 0;
    }




    //==== Process Iteration over all channels
    for(int i=0;i<N_Laser;i++){
      // This loop shifts the bits 4 left so that the PORTF can be iterated
      // as follows: PORTF =  B00000000 to PORTF =  B11110000
      PORTF =  i << 4;
      // Check if the Input has changed wrt the last time
      if (!(regB[i] == (PINB | B11100001))){
        Up.PF[i]    =  bitRead(regB[i],3) && !bitRead(PINB,3);
        Up.NF[i]    =  !bitRead(regB[i],3) && bitRead(PINB,3);
        Up.State[i] =  !bitRead(PINB,3);
        Dn.PF[i]    =  bitRead(regB[i],1) && !bitRead(PINB,1);
        Dn.NF[i]    =  !bitRead(regB[i],1) && bitRead(PINB,1);
        Dn.State[i] =  !bitRead(PINB,1);
        Aux.PF[i]   =  bitRead(regB[i],4) && !bitRead(PINB,4);
        Aux.NF[i]    =  !bitRead(regB[i],4) && bitRead(PINB,4);
        Aux.State[i] =  !bitRead(PINB,4);
        Pt.PF[i]    =  bitRead(regB[i],2) && !bitRead(PINB,2);
        Pt.NF[i]    =  !bitRead(regB[i],2) && bitRead(PINB,2);
        Pt.State[i] =  !bitRead(PINB,2);
      }
      else{
        Up.PF[i] =  0;
        Up.NF[i] =  0;
        Dn.PF[i] =  0;
        Dn.NF[i] =  0;
        Pt.PF[i] =  0;
        Pt.NF[i] =  0;
        Aux.PF[i] = 0;
        Aux.NF[i] = 0;
      }
      regB[i] = (PINB | B11100001);
      PotAll[i] = analogRead(A10);
      
      if(i>1){
        //==== PROCESS : Process the channel inputs / Change Midi Note
        if(PickNote[i] && Up.PF[i] && ActiveNoteTmp[i]<16){
          ActiveNoteTmp[i]++;
          DispCurrentNote = true;
          NoteChanged =  ActiveNoteTmp[i];
        }
        if(PickNote[i] && Dn.PF[i] && ActiveNoteTmp[i]>0){
          ActiveNoteTmp[i]--;
          DispCurrentNote = true;
          NoteChanged =  ActiveNoteTmp[i];
        }
        //==== If get out of picknote state then write new note type to output
          if(!PickNote[i]){
            ActiveNote[i] = ActiveNoteTmp[i];
          }
        //==== PROCESS : Set state to pick a different note (PickNote)
        if(Aux.PF[2]){
          PickNote[i] =! PickNote[i];
          if(PickNote[i] ){
            LED_B.setPWM(i, 0,Brightness);
          }
          else{
            LED_B.setPWM(i, 4096, 0);
            DispCurrentNote = false;
          }
        }

        //==== OUTPUTS : Control the LASER and LED channels
        if(Up.PF[i] && !PickNote[i] && (millis()-DebounceTime[i]) > DebounceTimeMs){
          DebounceTime[i] = millis();
          Laserstate[i] = !Laserstate[i];
          if(Laserstate[i]){
            LED_R.setPWM(i,0,Brightness);
            pwm1.setPWM(i,0,4096);
          }
          else{
            LED_R.setPWM(i, 4096,0);
            pwm1.setPWM(i,4096,0);
          }
        }
        if(Dn.PF[i] && !Aux.State[3] ){
          LED_R.setPWM(i,0,Brightness);
          pwm1.setPWM(i,0,4096);
        }
        if(!Laserstate[i] && Dn.NF[i]){
          LED_R.setPWM(i, 4096,0);
          pwm1.setPWM(i,4096,0);
        }
        // Write Note to Midi
        if(!Dn.State[i] && Laserstate[i] && Pt.PF[i]){noteOn(midi_CH, Notes[ActiveNote[i]], PotAll[i]/8);}  //check
        //if(!Dn.State[i] && Pt.NF[i]){noteOff(midi_CH, Notes[ActiveNote[i]], PotAll[i]/16);}
        if(((PickNote[i] && Up.State[i]) || Dn.State[i]) && Pt.PF[i]){noteOn(Preview_CH, Notes[ActiveNoteTmp[i]], PotAll[i]/8);}  //check
        //if(((PickNote[i] && Up.State[i]) || Dn.State[i]) && Pt.NF[i]){noteOff(Preview_CH, Notes[ActiveNoteTmp[i]], PotAll[i]/16);}
      }
    }



    //====Flush MIDI to send MIDI notes 
    if(State != 1){
      MidiUSB.flush();
    }
  }


  //==== Stepper Motor Controller ====//
  if(millis() > (T_1+Ts_1)){
    T_1 = millis();
    if(Clock==0){
      Freq = OffsetManual + (4 * FreqP + 6 * int( CDOffset + (4170) / BPMTimeDiff)) / 10;
    }
    FreqP = Freq;
    //==== Startup curve of the CD
    if(StartCD && (MIDI_Pos > 2 || Clock==1)){
        if(FreqCnt==0){
          digitalWrite(9,LOW);
        }
        if(FreqCnt == 10){
          StartCD = false;
        }
        FreqOut = FreqGainStart[FreqCnt] * Freq / 20;
        FreqCnt++;
      }
    //==== Stopping curve of the CD
    if(StopCD){
      if(FreqCnt < 10){
        FreqOut = FreqGainStop[FreqCnt] * Freq / 20;
        FreqCnt++;
      }
      else{
        FreqOut = FreqGainStop[9] * Freq / 20;
      }
    }
    //==== Normal frequency operation of CD
    if(not StartCD and not StopCD){
      if((Freq-FreqOutP) > 4 && (Freq-FreqOutP) < 30){
        FreqOut = FreqOutP+4;
      }
      else if((Freq-FreqOutP) < -4 && (Freq-FreqOutP) > -30){
        FreqOut = FreqOutP-4;
      }
      else{
        FreqOut = Freq;
      }
    }
    FreqOutP=FreqOut;
    //==== Calculate BPM when CD is turning
    if(State==3){
      //Filter PBM to display smooth signal
      BPM = (9 * BPM_p + 1 * FreqOut * 10/16) / 10;
      BPM_p = BPM;
    }
    else{
      BPM = 0;
      BPM_p = 0;
    }
    if(State == 0){
      digitalWrite(9,HIGH);
      StopCD = false;
      CDOffset = 0;
    }
    //==== Tone function is used to write block wave to stepper motor.. Awesome!
    tone(7,8*FreqOut);
  }

  //==== Call the display library
  if(millis() > (T_3+Ts_3)){
    T_3 = millis();
    DispMenu.DispState(Menu,SubMenu,BPM,DispCurrentNote,NoteChanged);
  }

  //==== Internal Clock Mode : Increment and decrement Frequency manually 
  if(millis() > (T_4+Ts_4) && Menu==0  && Freq > 0 && Freq < 800){
    T_4 = millis();
      Freq = Freq + (Aux.State[1]-Aux.State[0]) *16/10 * (2*(millis() > (T_FreqTimer +4000)) + (millis() >(T_FreqTimer +1000)));
  }

  //==== Run Nightrider effect
  if(Menu == 3){
    for(int j=2;j<16;j++){
      LED_R.setPWM(j, 4096,0);
      pwm1.setPWM(j,4096,0);
    }
    LED_R.setPWM(NightRider[KittTimer],0,Brightness);
    pwm1.setPWM(NightRider[KittTimer],0,4096);
    KittTimer++;
    if(KittTimer == 31){
      KittTimer=0;
    }
  }
}