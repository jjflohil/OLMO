
#include "Inputs_Multiplex.h"
#include <digitalWriteFast.h>
#include "arduino.h"

// The function can be used for On/Off Switches, On/Off/On switches and potmeters
// For On/Off switch Give the SwitchUpPin the same number as the SwitchDnPin
// The data is read by a multiplexer

Inputs_Multiplex::Inputs_Multiplex(int SwitchUpPin, int SwitchDnPin, int PhotoPin, int PotPin,
                                    int AuxPin, int ChPin0, int ChPin1, int ChPin2, int ChPin3)
{
    pinMode(SwitchUpPin, INPUT_PULLUP);
    pinMode(SwitchDnPin, INPUT_PULLUP);
    pinMode(PhotoPin, INPUT_PULLUP);
    pinMode(AuxPin, INPUT_PULLUP);
    
    pinMode(ChPin0,OUTPUT);
    pinMode(ChPin1,OUTPUT);
    pinMode(ChPin2,OUTPUT);
    pinMode(ChPin3,OUTPUT);
    _SwitchUpPin = SwitchUpPin;
    _SwitchDnPin = SwitchDnPin;
    _PotPin      = PotPin;
    _PhotoPin    = PhotoPin;
    _AuxPin      = AuxPin;
    _ChPin0      = ChPin0; 
    _ChPin1      = ChPin1; 
    _ChPin2      = ChPin2; 
    _ChPin3      = ChPin3; 
}

//Demultiplex the channel to control the multiplex boards
void Inputs_Multiplex::DeMux(int InChannel)
{   bool PinMap[4];
    if(InChannel<16)
    {   for(int k = 0;k<4;k++)
        {   int mask =  1 << k;
            int masked_n = InChannel & mask;
             PinMap[k]  = masked_n >> k;
        }
        digitalWrite(_ChPin0,PinMap[0]);
        digitalWrite(_ChPin1,PinMap[1]);
        digitalWrite(_ChPin2,PinMap[2]);
        digitalWrite(_ChPin3,PinMap[3]);
    }
}
//Function to read OnOff Switch
bool* Inputs_Multiplex::ReadOnOff(int InChannel)
{   DeMux(InChannel);
    _SwUpStateP = _SwUpState;
    _SwUpState = digitalRead(_SwitchUpPin);

    _OnOffRet[0] = _SwUpState;                   // Actual state
    _OnOffRet[1] = _SwUpStateP && !_SwUpState;   // Positive Flank
    _OnOffRet[2] = _SwUpState && !_SwUpStateP;   // Negative Flank
    return _OnOffRet;
}
//Function to read OnOffOn switch
bool* Inputs_Multiplex::ReadOnOffOn(int InChannel)
{   DeMux(InChannel);
    _SwUpStateP = _SwUpState;
    _SwUpState = digitalRead(_SwitchUpPin);
    _SwDnStateP = _SwDnState;
    _SwDnState = digitalRead(_SwitchDnPin);

    _OnOffOnRet[0] = _SwUpState;                   // Actual state
    _OnOffOnRet[1] = _SwUpStateP && !_SwUpState;   // Positive Flank
    _OnOffOnRet[2] = _SwUpState && !_SwUpStateP;   // Negative Flank
    _OnOffOnRet[3] = _SwDnState;   
    _OnOffOnRet[4] = _SwDnStateP && !_SwDnState;   // Positive Flank
    _OnOffOnRet[5] = _SwDnState && !_SwDnStateP;   // Negative Flank
    return _OnOffOnRet;
}

//In case of potmeter the range and the desired range can be given as:
//Input[0 , 1023 , 0 , 100]
int Inputs_Multiplex::ReadPotMeter(int InChannel,int In1, int In2, int In3)
{   DeMux(InChannel);
    int ReturnValue;
    if(In1 == 0){
        ReturnValue = map(analogRead(_PotPin),0,1023,0,100);
    }
    else if(In1 > 0){
        ReturnValue = map(analogRead(_PotPin),0,In1,In2,In3);
    }
    else{
        ReturnValue = 0;
    }
    
    return ReturnValue;
}
//Function to read OnOff Switch
bool* Inputs_Multiplex::ReadPhoto(int InChannel)
{   DeMux(InChannel);
    _PhotoStateP = _PhotoState;
    _PhotoState = digitalRead(_PhotoPin);
    _OnOffRet[0] = _PhotoState;                   // Actual state
    _OnOffRet[1] = _PhotoStateP && !_PhotoState;   // Positive Flank
    _OnOffRet[2] = _PhotoState && !_PhotoStateP;   // Negative Flank
    return _OnOffRet;
}

 Inputs_Multiplex::InData Inputs_Multiplex::ReadAll(int InChannel,struct InData DataInput)
 {  DeMux(InChannel);
    _SwUpState = digitalRead(_SwitchUpPin);
    _SwDnState = digitalRead(_SwitchDnPin);
    _PhotoState = digitalRead(_PhotoPin);
    _AuxState   = digitalRead(_AuxPin);
    //Determine the state and flanks adn write to Data
    struct InData DataOut;

    DataOut.State[0] = _SwUpState;      //SwUp
    DataOut.State[1] = _SwDnState;      //SwDn
    DataOut.State[2] = _PhotoState;     //PhTr
    DataOut.State[3] = _AuxState;       //Aux

    DataOut.PF[0]    = DataInput.State[0]  &&  !_SwUpState;      
    DataOut.PF[1]    = DataInput.State[1]  &&  !_SwDnState;
    DataOut.PF[2]    = DataInput.State[2]  &&  !_PhotoState;
    DataOut.PF[3]    = DataInput.State[3]  &&  !_AuxState;

    DataOut.NF[0]    = _SwUpState   &&  !DataInput.State[0];      
    DataOut.NF[1]    = _SwDnState   &&  !DataInput.State[1];
    DataOut.NF[2]    = _PhotoState  &&  !DataInput.State[2];
    DataOut.NF[3]    = _AuxState    &&  !DataInput.State[3];

    DataOut.Pot      = map(analogRead(_PotPin),0,1023,0,255);
    return DataOut;
}

