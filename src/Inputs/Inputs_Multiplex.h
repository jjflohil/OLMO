#ifndef Inputs_Multiplex_h
#define Inputs_Multiplex_h

class Inputs_Multiplex
{
  public:
    Inputs_Multiplex(int SwitchUpPin, int SwitchDnPin, int PhotoPin, int PotPin,
                     int AuxPin, int ChPin0, int ChPin1, int ChPin2, int ChPin3);
    //Data definition
    struct InData
    {
      bool State[4];
      bool PF[4];
      bool NF[4];
      int Pot;
    };
    //Function definition
    bool* ReadOnOff(int InChannel);
    bool* ReadOnOffOn(int InChannel);
    bool* ReadPhoto(int InChannel);
    int ReadPotMeter(int InChannel, int In0, int In1, int In2);
    InData ReadAll(int InChannel, InData DataInput); 
    
    void DeMux(int InChannel);
  private:
    int _SwitchUpPin;
    int _SwitchDnPin;
    int _PhotoPin;
    int _PotPin;
    int _AuxPin;
    int _ChPin0;
    int _ChPin1;
    int _ChPin2;
    int _ChPin3;
    bool _SwUpState;
    bool _SwUpStateP;
    bool _SwDnState;
    bool _SwDnStateP;
    bool _PhotoState;
    bool _PhotoStateP;
    bool _AuxState;
    bool _OnOffRet[3];   //[State flankP flankN]
    bool _OnOffOnRet[6]; //[StateUp flankPUp flankNUp StateDn flankPDn flankNDn]
};

#endif

