#include "StepRegulator.h"
#include "arduino.h"

StepRegulator::StepRegulator(int EN_PIN, int DIR_PIN, int STEP_PIN)
{
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN,HIGH); //deactivate driver (LOW active)
    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(DIR_PIN, HIGH); //LOW or HIGH
    pinMode(STEP_PIN, OUTPUT);
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(EN_PIN, LOW); //activate driver
    _En_pin = EN_PIN;
    _Dir_pin = DIR_PIN;
    _Step_pin = STEP_PIN;



}
void StepRegulator::ControlBPM(float BPM_ref)
{
    if(BPM_ref>10.0){
        digitalWrite(_En_pin,LOW);
    }
    else{
        digitalWrite(_En_pin,HIGH);
    }
    _Ts = round(3e5 / (BPM_ref));
    if(micros() >= (_Time+_Ts)){
        _Time = micros();
        _Pulse =1-_Pulse;
        digitalWrite(_Step_pin, _Pulse);
    }
}

float StepRegulator::GetBPM()
{
    _Laser0P = _Laser0;
    _Laser0 = digitalRead(A0);
    if(_Laser0 && !_Laser0P){
        _TLaser0P =  _TLaser0;
        _TLaser0  = millis();
        _BPM_act = float((15e3 / (_TLaser0 -_TLaser0P)));
    }
    if(_BPM_act>1000){
        _BPM_act = 0;
    }
    return _BPM_act;
}

