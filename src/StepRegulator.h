#ifndef StepRegulator_h
#define StepRegulator_h

class StepRegulator
{
  public:
    StepRegulator(int En_pin, int Dir_pin, int Step_pin);
    void ControlBPM(float BPM_ref);
    float GetBPM();

  private:
    int _En_pin;
    int _Dir_pin;
    int _Step_pin;

    bool  _Pulse;
    float _BPM_ref;
    float _BPM_act;
    unsigned long _Time = 0;
    unsigned long _Ts;

    //Parameters get BPM
    bool _Laser0;
    bool _Laser0P;
    bool _Laser1;
    bool _Laser1P;
    unsigned long _TLaser0 = 0;
    unsigned long _TLaser0P = 0;
    unsigned long _TLaser1 = 0;
    unsigned long _TLaser1P = 0;

};

#endif

