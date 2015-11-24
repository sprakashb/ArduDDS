//Arduino based dds 
// nothing here is claimed my original
// Ideas of various Hams used in this set of progs, 
// thanks to all those who have published their codes and various libraries for the benefit of other enthusiasts and hams.
// Copy left ... all are free to copy and use this prog for Ham radio, personal, educational or even commercial applications
// just dont forget the contribution of great people who allowed us to get benefit of their hard work.
// By : VU2SPF / SP Bhatnagar, qth at  Bhavnagar, India. PCB artwork and details also on vu2spf.blogspot.in 
// Please send error reports / suggestions/ mods to vu2spf@gmail.com
//
#include <rotary.h>
#include <EEPROM.h>
#include "EEPROMAnything.h" 
#include <LiquidCrystal.h>
#include <OnewireKeypad.h>
#include <BitBool.h>

// ------------ band inits some of these may be country / region specific, adjust accordingly
char* BandNames[] = { 
  "136k", "160m", " 80m", " 60m", " 40m", " 30m", " 20m", " 17m", " 15m", " 12m", " 10m", "Cont"  };
int Bands[] = { 
  136, 160, 80, 60, 40, 30, 20, 17, 15, 12, 10, 0};
long BandBases[]={
  135700L, 1810000L, 3500000L, 5258500L, 7000000L, 10100000L, 14000000L, 18068000L, 21000000L, 24890000L, 28000000L, 0L};
long BandTops[]={
  137800L, 2000000L, 3800000L, 5406500L, 7200000L, 10150000L, 14350000L, 18168000L, 21450000L, 24990000L, 29700000L, 30000000L};
long BandWkgFreqs[]={
  135700L, 1836000L, 3560000L, 5258500L, 7035000L, 10106000L, 14200000L, 18086000L, 21060000L, 24906000L, 28060000L, 1000000L};  
int bindex=4; // initial band selection index, on startup 40 m selected ,  can the last used freq can be stored on eprom and retrievd 

//----------- LCD, Encoder &  DDS defs
// LCD connections
// db4 - db7 = D6 - 9, En = D5, RS = D4
LiquidCrystal lcd(4, 5, 6, 7, 8, 9); 
// lcd screen layout for 20X 4 displ can have another set for 16x2 
//row 0 the top line
int mcbmrRow=0; // for display of VFO/mem ch band mode rit
int memVfoCol=0;
int chCol=3;
int bandCol=6;
int modeCol=11;
int ritCol = 16;

// row 1 freq and step size
int freqCol= 0;
int freqRow = 1;
int stepCol = 16;

// messages on 3rd  row
int msgCol=0;
int msgRow = 2;

//data entered in line 4 eg if_offset rit etc with kpd entry
int dataCol=2;
int dataRow=3;

// DDS connections
// Reset = D10, Data = D11, FQ_UD = D12, W_CLK = D13
#define W_CLK 13   //  module word load clock pin (CLK)
#define FQ_UD 12   //  freq update pin (FQ)
#define DATA 11   //  serial data load pin (DATA)
#define RESET 10  //  reset pin (RST) 

Rotary encoder=Rotary(2,3);
// int EncoderSw = A5; // if used

//--------- Analog Keybaord defs thanks to arduino libr
#define Rows 4
#define Cols 4
#define kpdPin A0
#define Col_Res 4900  // 4k7 to adjust for Resist value for proper key o/p
#define Row_Res 1000
// -----------kpd layout
char KEYS[]= 
{  
  '1','2','3','A',
  '4','5','6','B',
  '7','8','9','C',
  '*','0','#','D'
};

OnewireKeypad <LiquidCrystal, 16>   Kpd(lcd,  KEYS, Rows, Cols, kpdPin, Col_Res, Row_Res );
char key;

//------------- generating pulse 
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }
// ==============

// ---- freqs VFO mem ch side band variables  inits
unsigned long currFreq;
long freq1, freq2;
long if_offset = 10000000;
int ritFreq = 600; 
int currSB = 1; 
int ssb_offset = 1500;

struct all_infos{
  unsigned long currFreq_s;
  long if_offset_s;
  int ritFreq_s; 
  int currSB_s; 
  int ssb_offset_s;
} 
currInfo;

boolean VFO_mode = false;
char actvVFO = 'A'; // initial VFO selected all VFOs have initial settings of diff freqs
char* SBList[] = {  
  "LSB ", "USB ", "CW  ", "AM  "};
int currCh = 0; // working memory channel no 0 -39 (40 ch)
boolean ritActv = true;
long UpdTime; // time of last freq / ch mem/  SB / RIT updation so that last used data can be saved if not changed for 10 sec
boolean Updated=false; // flag to indicate new frq
int maxCh = 39 ; // max no of mem channels 

// ===== modes of operation
enum op_modes{ 
  displ_mode, freq_adj_mode, if_adj_mode, step_adj_mode, kpd_entry_mode,  band_sel_mode, rit_adj_mode, ch_sel_mode, ssb_adj_mode
} 
currMode = displ_mode; // currently selected mode

long modeTime = 0; // time of entering a mode for timed exit

unsigned long deltaf[8] = { 
  1,10,100,1000,10000, 100000, 1000000, 10000000};   // freq change steps
String deltafStr[8] = { 
  "   1", "  10", " 100", "  1k", " 10k", "100k", "  1M", " 10M"}; // to be displayed steps
// inits
int steps = 3; // initial setting 1000 Hz step change
int dir=0; // direction of motion of rotary encoder

int  VFObaseAdd = maxCh * sizeof(currInfo);  // EEPROM address where VFOs are stored
//// ----------------------------------------  

void setup() {
  lcd.begin(20, 4);
 
  // encoder interrupts
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();
  // works

  // inits
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT); 
  // DDS init
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // this pulse enables serial mode on the AD9850 - Datasheet page 12.

  loadLastSettings(); // initially on power up retrieve last used settings
   load_freq();
  showFreq(currFreq);
  showInfo();
  
  // Analog kpd defs
  Kpd.SetHoldTime(1000); // Key hold in ms
  Kpd.SetDebounceTime(300); // 50 mS
  // Various commands based on key pressed form:(called fn, keypressed)
  Kpd.addEventKey(get_freq, '0');
  Kpd.addEventKey(sel_vfo_mem, '1');
  Kpd.addEventKey(get_mem, '2');  // get mem info on momentary press / save the mem info if pressed for 2 sec
  Kpd.addEventKey(sel_band, '3');
  Kpd.addEventKey(set_sb,'4'); // sideband switching
  Kpd.addEventKey(sel_rit, '5');
  Kpd.addEventKey(set_rit, '6'); 
  Kpd.addEventKey(sel_stepsize, '7'); // step  change size
  Kpd.addEventKey(set_if_offset, '8'); // IF offset
  Kpd.addEventKey(set_ssb_offset, '9');  // ssb offset in Hz
  // Kpd.addEventKey(help, '#');  // help info tbd
  Kpd.addEventKey(set_vfoA, 'A'); 
  Kpd.addEventKey(set_vfoB, 'B');  
  Kpd.addEventKey(set_vfoC, 'C'); 
  Kpd.addEventKey(set_vfoD, 'D');  
}

void loop() {
  Kpd.ListenforEventKey(); // look if any command key pressed
  showFreq(currFreq);
  if(millis() - modeTime >= 5000)
    currMode = displ_mode; // to make sure that it comes out of any mode in 5 sec if not used
 // if(currMode == displ_mode)
    showInfo();
 /* if( Updated &&  (millis() - UpdTime >= 10000)) // if freq or other info changed and remains in same state for 10 sec store it on EPROM   
  {
    Updated = false;
    storeLastSettings();
    store_freq();  // store the freq as per need
  }*/

  freq1 = (currFreq - if_offset + ssb_offset + ritFreq);
  freq2 = abs(freq1);
    sendFrequency(freq2);
 //   lcd.setCursor(msgCol, msgRow);
 //   lcd.print(freq2);
  delay(100);
}


//////////


ISR(PCINT2_vect)
{
  unsigned char dir = encoder.process();
  if(dir == DIR_NONE)
    return;

  // otherwise if interrupt generated due to encoder rotation then work according to 
  // selected current mode

  switch(currMode)
  {
  // either freq adjustment or normal display mode . change freq by step size
  case freq_adj_mode:
  case displ_mode: // if encoder rotated change freq at current step shown with ul cursor
    {
      modeTime = millis();
      if(dir == DIR_CW)
      { 
        currFreq = currFreq + (long)deltaf[steps] ;
        currFreq=constrain(currFreq, 100000, 30000000); // lower limit can be zero but the upper limit depends on the DDS xtal freq (1/3rd of it)
      }
      else 
      {
        currFreq = currFreq -(long)deltaf[steps] ;
        currFreq = constrain(currFreq, 100000, 30000000);
      }
      Updated = true;
      UpdTime = millis();
      break;
    }
    // when band select key '3' is pressed 
  case band_sel_mode:
    { 
      modeTime = millis();
      if(dir == DIR_CW)
      {
        bindex = bindex + 1;
        if(bindex >= 12)   // total 12 bands defined
          bindex = 0;
      }
      else
      {
        bindex = bindex - 1;
        if(bindex <= 0)
          bindex = 11;
      }
      currFreq = BandWkgFreqs[bindex];
      // switchBands();  // ** output band selection on pins tbd
      Updated = true;
      UpdTime = millis();
      break;
    }

    // when step size key '7' pressed
  case step_adj_mode:
    {
      encoderMsg();
   //   modeTime = millis();
      if(dir == DIR_CW)
      {
        steps = steps + 1;
        if (steps >= 8)
          steps =0;
      }
      else 
      {
        steps = steps - 1;
        if (steps < 0)
          steps =7;
      }
      break;
    }

    // when mem channel select key '2' pressed ( 0 -39 = 40 ch)
  case ch_sel_mode:
    {
      modeTime = millis();
      if(dir == DIR_CW)
      {
        currCh = currCh + 1;
        if (currCh >= maxCh)
          currCh =0;
      }
      else 
      {
        currCh = currCh - 1;
        if (currCh < 0)
          currCh =maxCh;
      }

      Updated = true;
      UpdTime = millis();
      load_freq();
      break;  
    }
  
    showInfo(); // update the changes on display
  }
}
//----------------

void showFreq(long rx)
{ // display freq in HAM style 2 + 3 + 3 digits
  lcd.setCursor(freqCol, freqRow);
  String f( rx);  // convert number to string
  // pad if needed
  byte len = f.length();
  String padding;
  for(int i = 0; i < 7 - len; ++i)
    padding += String("0");
  f = padding + f;
  // split the frequency string
  String k,m,r;
  k = f.substring(f.length() - 3);
  m = f.substring(f.length() - 6, f.length() - 3);
  r = f.substring(0, f.length() - 6);
  if(r.length() <= 1)
    lcd.print(" ");

  // now print the frequency
  lcd.print(r);
  lcd.print('.');
  lcd.print(m);
  lcd.print('.');
  lcd.print(k);
  lcd.print("Hz");
  lcd.print(" Stp");
  lcd.print(deltafStr[steps]);
/*
  switch(steps)
  {
  case 7: 
  case 6: 
    lcd.setCursor((freqCol+7-steps), freqRow);
    lcd.cursor();
    break;

  case 5: 
  case 4: 
  case 3:
    lcd.setCursor( freqCol+ 8 - steps , freqRow);
    lcd.cursor();
    break;

  case 2: 
  case 1: 
  case 0:
    lcd.setCursor( freqCol + 9- steps , freqRow);
    lcd.cursor();
    break; 
  } */
}
//--------------
void encoderMsg()   // displ indicator to rotate encoder <o> 
{ 
  lcd.setCursor(6, msgRow);
  lcd.print("<<o>>");
 } 
//-------------------
long get_data(char message[], long currVal, long limit)    // mesg to be displayed, current value and max data limit
{   
  long num = 0;
  int   mult = 1;
  lcd.setCursor(0 ,msgRow);
  lcd.print(message);
  lcd.setCursor(dataCol, dataRow);
  //lcd.print(" :");
  lcd.print(currVal);   // show current value
  lcd.cursor();
  lcd.setCursor(dataCol, dataRow);
  //delay(500);
  key = Kpd.Getkey();
  while(key != '#' && num <= limit)
  {
    switch (key)
    {
    case NO_KEY:
      break;

    case '0': 
    case '1': 
    case '2': 
    case '3':      
    case '4':
    case '5': 
    case '6': 
    case '7': 
    case '8': 
    case '9':
      modeTime = millis();
      num = num * 10 + (key - '0');
      if(num > limit)
      {
        lcd.setCursor(msgCol, msgRow);
        lcd.print("Limit Error");
        delay(1000);
        break ;
      }
      else
      {
        lcd.setCursor(dataCol, dataRow);
        lcd.print(num);
      }
      mult = mult *10;
      break;

    case '*':
      num = num * (-1);
      lcd.setCursor(dataCol-1, dataRow);
      lcd.print(num);
      break;
      
     case 'A':   // escape Key in case of error
     lcd.noCursor();
     cleanupmsg();
     Updated = false;
     return (currVal);
     break;
    }
    delay(200);
    key = Kpd.Getkey();
  }
  lcd.noCursor();
  cleanupmsg();
  // to save changes
  Updated = true;
  UpdTime = millis();
  return num;
}
// -------------

void cleanupmsg()        // clean up the message and data lines
{
  // clean up lines 3 & 4
  lcd.setCursor(0 ,msgRow);
  lcd.print("                    ");
  lcd.setCursor(0,dataRow);
  lcd.print("                    ");
}

//------------------
void showInfo()                // display top line infos
{  
  lcd.setCursor(memVfoCol, mcbmrRow);
  if (VFO_mode)  
  {
    lcd.print("VFO ");
    lcd.print(' ');
    lcd.print(actvVFO);
    lcd.print(' ');
  }
  else
  {
    lcd.print("Mem");
    if(currMode == ch_sel_mode)
      lcd.print('>');              // ready to change ch no using encoder
    else
      lcd.print(' ');

    if(currCh < 10)
      lcd.print(' ');
    lcd.print(currCh);
  }
  chkBand();  // check the correct band as per freq
  lcd.setCursor(bandCol, mcbmrRow);
  lcd.print(BandNames[bindex]);

  lcd.setCursor(modeCol, mcbmrRow);
  lcd.print(SBList[currSB]);
  lcd.print("  ");  // to fill the gap
  lcd.setCursor(ritCol, mcbmrRow);
  if(ritActv)
  {
    if (ritFreq < 0)
      lcd.print(ritFreq);
    else
    {
      if(ritFreq == 0)
        lcd.print(" 000");
      else
      {
        lcd.print(" ") ;
        lcd.print(ritFreq);
      }
    }
  }
  else 
    lcd.print("     ") ;


  // lcd.noCursor();
  if(currMode == displ_mode) 
    cleanupmsg();
}

void chkBand()
{

  for(int i=0; i<=11; i++)       // chk the band and display
  {  
    if (currFreq >= BandBases[i] && currFreq <=BandTops[i])
    {
      bindex = i;
      break;
    }
  }
}
//-------------
// Program DDS chip
void sendFrequency(double frequency) {  
  int  calconstt = 0;
  long freq = (frequency+calconstt) * 4294967295/125000000 ;  // note 125 MHz clock on 9850.  You can make 'slight' tuning variations here by adjusting the clock frequency.
  //disp_freq(freq); // display the freq before programming DDS
  //showFreq(currFreq);
  for (int b=0; b<4; b++, freq>>=8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}
//----------------------
// transfer serially - LSB first to the 9850 via serial DATA line
void tfr_byte(byte data)
{
  for (int i=0; i<8; i++, data>>=1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}
// DDS Chip prog end
//----------------









