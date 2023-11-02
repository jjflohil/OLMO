
#include "ProcessMenu.h"
#include "arduino.h"


ProcessMenu::ProcessMenu(int clk, int data)
{
    clkpin = clk;
    datapin = data;
    pinMode(clkpin, OUTPUT);
    pinMode(datapin, OUTPUT);
    ProcessMenu::init();
    ProcessMenu::set(2);
    ProcessMenu::displayStr(" HI ");
    
 
}
void ProcessMenu::DefMenu(int Index_0, int Index_1, char* Menu1,char* Menu2,char* Menu3,char* Menu4)
{
    
    if(Index_0==0){
        if(Index_1>0){_MenuTxt[0] = Menu1;}
        if(Index_1>1){_MenuTxt[1] = Menu2;}
        if(Index_1>2){_MenuTxt[2] = Menu3;}
    }
    else if(Index_0>0){
        if(Index_1>0){_SubmenuTxt[Index_0][0] = Menu1;}
        if(Index_1>1){_SubmenuTxt[Index_0][1] = Menu2;}
        if(Index_1>2){_SubmenuTxt[Index_0][2] = Menu3;}
    }
}

void ProcessMenu::DispState(int Index_0,int Index_1,int BPM,bool DispNote, int track)
{   

    if(DispNote){
        //ProcessMenu::displayStr("NOTE");
        char* NoteCh[16] = {" C1 "," C*1"," D1 "," D*1"," E1 "," F1 "," F*1"," G1 ",
                    " G*1"," A1 "," A*1"," B1 "," C2 "," C*2"," D2 "," D*2",};
        ProcessMenu::displayStr(NoteCh[track]);
    }
    else{
        if(Index_0 > 0 && Index_1 > 0 ){
            ProcessMenu::displayStr(_SubmenuTxt[Index_0][Index_1-1]);
        }
        else{
            if(BPM==0){
                ProcessMenu::displayStr(_MenuTxt[Index_0]);
            }
            else{
                ProcessMenu::displayNum(BPM);
            }
        }
    }
}


////=====================================///
////=====================================///
////=====================================///
////=====================================///
namespace {
    uint8_t char2segments(char c) {
        switch (c) {
            case '_' : return 0x08;
            case '^' : return 0x01; // ¯
            case '-' : return 0x40;
            case '*' : return 0x63; // °
            case ' ' : return 0x00; // space
            case 'A' : return 0x77; // upper case A
            case 'a' : return 0x5f; // lower case a
            case 'B' :              // lower case b
            case 'b' : return 0x7c; // lower case b
            case 'C' : return 0x39; // upper case C
            case 'c' : return 0x58; // lower case c
            case 'D' :              // lower case d
            case 'd' : return 0x5e; // lower case d
            case 'E' :              // upper case E
            case 'e' : return 0x79; // upper case E
            case 'F' :              // upper case F
            case 'f' : return 0x71; // upper case F
            case 'G' :              // upper case G
            case 'g' : return 0x35; // upper case G
            case 'H' : return 0x76; // upper case H
            case 'h' : return 0x74; // lower case h
            case 'I' : return 0x06; // 1
            case 'i' : return 0x04; // lower case i
            case 'J' : return 0x1e; // upper case J
            case 'j' : return 0x16; // lower case j
            case 'K' :              // upper case K
            case 'k' : return 0x75; // upper case K
            case 'L' :              // upper case L
            case 'l' : return 0x38; // upper case L
            case 'M' :              // twice tall n
            case 'm' : return 0x37; // twice tall ∩
            case 'N' :              // lower case n
            case 'n' : return 0x54; // lower case n
            case 'O' :              // lower case o
            case 'o' : return 0x5c; // lower case o
            case 'P' :              // upper case P
            case 'p' : return 0x73; // upper case P
            case 'Q' : return 0x7b; // upper case Q
            case 'q' : return 0x67; // lower case q
            case 'R' :              // lower case r
            case 'r' : return 0x50; // lower case r
            case 'S' :              // 5
            case 's' : return 0x6d; // 5
            case 'T' :              // lower case t
            case 't' : return 0x78; // lower case t
            case 'U' :              // lower case u
            case 'u' : return 0x1c; // lower case u
            case 'V' :              // twice tall u
            case 'v' : return 0x3e; // twice tall u
            case 'W' : return 0x7e; // upside down A
            case 'w' : return 0x2a; // separated w
            case 'X' :              // upper case H
            case 'x' : return 0x76; // upper case H
            case 'Y' :              // lower case y
            case 'y' : return 0x6e; // lower case y
            case 'Z' :              // separated Z
            case 'z' : return 0x1b; // separated Z
        }
        return 0;
    }
}

static int8_t tube_tab[] = {0x3f, 0x06, 0x5b, 0x4f,
                            0x66, 0x6d, 0x7d, 0x07,
                            0x7f, 0x6f, 0x77, 0x7c,
                            0x39, 0x5e, 0x79, 0x71
                           }; //0~9,A,b,C,d,E,F



void ProcessMenu::init(void) {
    clearDisplay();
}

int ProcessMenu::writeByte(int8_t wr_data) {
    for (uint8_t i = 0; i < 8; i++) { // Sent 8bit data
        digitalWrite(clkpin, LOW);

        if (wr_data & 0x01) {
            digitalWrite(datapin, HIGH);    // LSB first
        } else {
            digitalWrite(datapin, LOW);
        }

        wr_data >>= 1;
        digitalWrite(clkpin, HIGH);
    }

    digitalWrite(clkpin, LOW); // Wait for the ACK
    digitalWrite(datapin, HIGH);
    digitalWrite(clkpin, HIGH);
    pinMode(datapin, INPUT);

    bitDelay();
    uint8_t ack = digitalRead(datapin);

    if (ack == 0) {
        pinMode(datapin, OUTPUT);
        digitalWrite(datapin, LOW);
    }

    bitDelay();
    pinMode(datapin, OUTPUT);
    bitDelay();

    return ack;
}

// Send start signal to ProcessMenu (start = when both pins goes low)
void ProcessMenu::start(void) {
    digitalWrite(clkpin, HIGH);
    digitalWrite(datapin, HIGH);
    digitalWrite(datapin, LOW);
    digitalWrite(clkpin, LOW);
}

// End of transmission (stop = when both pins goes high)
void ProcessMenu::stop(void) {
    digitalWrite(clkpin, LOW);
    digitalWrite(datapin, LOW);
    digitalWrite(clkpin, HIGH);
    digitalWrite(datapin, HIGH);
}

// Display function.Write to full-screen.
void ProcessMenu::display(int8_t disp_data[]) {
    int8_t seg_data[DIGITS];
    uint8_t i;

    for (i = 0; i < DIGITS; i++) {
        seg_data[i] = disp_data[i];
    }

    coding(seg_data);
    start();              // Start signal sent to ProcessMenu from MCU
    writeByte(ADDR_AUTO); // Command1: Set data
    stop();
    start();
    writeByte(cmd_set_addr); // Command2: Set address (automatic address adding)

    for (i = 0; i < DIGITS; i++) {
        writeByte(seg_data[i]);    // Transfer display data (8 bits x num_of_digits)
    }

    stop();
    start();
    writeByte(cmd_disp_ctrl); // Control display
    stop();
}

//******************************************
void ProcessMenu::display(uint8_t bit_addr, int8_t disp_data) {
    int8_t seg_data;

    seg_data = coding(disp_data);
    start();               // Start signal sent to ProcessMenu from MCU
    writeByte(ADDR_FIXED); // Command1: Set data
    stop();
    start();
    writeByte(bit_addr | 0xc0); // Command2: Set data (fixed address)
    writeByte(seg_data);        // Transfer display data 8 bits
    stop();
    start();
    writeByte(cmd_disp_ctrl); // Control display
    stop();
}

//--------------------------------------------------------

void ProcessMenu::displayNum(float num, int decimal, bool show_minus) {
    // Displays number with decimal places (no decimal point implementation)
    // Colon is used instead of decimal point if decimal == 2
    // Be aware of int size limitations (up to +-2^15 = +-32767)

    int number = round(fabs(num) * pow(10, decimal));

    for (int i = 0; i < DIGITS - (show_minus && num < 0 ? 1 : 0); ++i) {
        int j = DIGITS - i - 1;

        if (number != 0) {
            display(j, number % 10);
        } else {
            display(j, 0x7f);    // display nothing
        }

        number /= 10;
    }

    if (show_minus && num < 0) {
        display(0, '-');    // Display '-'
    }

    if (decimal == 2) {
        point(true);
    } else {
        point(false);
    }
}

void ProcessMenu::displayStr(char str[], uint16_t loop_delay) {
    for (int i = 0; i < strlen(str); i++) {
        if (i + 1 > DIGITS) {
            delay(loop_delay); //loop delay
            for (int d = 0; d < DIGITS; d++) {
                display(d, str[d + i + 1 - DIGITS]); //loop display
            }
        } else {
            display(i, str[i]);
        }
    }

    // display nothing
    for (int i = strlen(str); i < DIGITS; i++) {
        display(i, 0x7f);
    }
}

void ProcessMenu::clearDisplay(void) {
    display(0x00, 0x7f);
    display(0x01, 0x7f);
    display(0x02, 0x7f);
    display(0x03, 0x7f);
}

// To take effect the next time it displays.
void ProcessMenu::set(uint8_t brightness, uint8_t set_data, uint8_t set_addr) {
    cmd_set_data = set_data;
    cmd_set_addr = set_addr;
    //Set the brightness and it takes effect the next time it displays.
    cmd_disp_ctrl = 0x88 + brightness;
}

// Whether to light the clock point ":".
// To take effect the next time it displays.
void ProcessMenu::point(boolean PointFlag) {
    _PointFlag = PointFlag;
}

void ProcessMenu::coding(int8_t disp_data[]) {
    for (uint8_t i = 0; i < DIGITS; i++) {
        disp_data[i] = coding(disp_data[i]);
    }
}

int8_t ProcessMenu::coding(int8_t disp_data) {
    if (disp_data == 0x7f) {
        disp_data = 0x00;    // Clear digit
    } else if (disp_data >= 0 && disp_data < int(sizeof(tube_tab) / sizeof(*tube_tab))) {
        disp_data = tube_tab[disp_data];
    } else if (disp_data >= '0' && disp_data <= '9') {
        disp_data = tube_tab[int(disp_data) - 48];    // char to int (char "0" = ASCII 48)
    } else {
        disp_data = char2segments(disp_data);
    }
    disp_data += _PointFlag == POINT_ON ? 0x80 : 0;

    return disp_data;
}

void ProcessMenu::bitDelay(void) {
    delayMicroseconds(50);
}
