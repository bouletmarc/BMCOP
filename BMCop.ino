/**********************************************************************
       __________  ______      ____  _   __   ____  __    __  ________
      / ____/ __ \/  _/ /     / __ \/ | / /  / __ \/ /   / / / / ____/
     / /   / / / // // /     / / / /  |/ /  / /_/ / /   / / / / / __  
    / /___/ /_/ // // /___  / /_/ / /|  /  / ____/ /___/ /_/ / /_/ /  
    \____/\____/___/_____/  \____/_/ |_/  /_/   /_____/\____/\____/   
    
COIL ON PLUG Script V1.0 for Atmega328P made by Bouletmarc (BMDevs)

This Coil On Plug script are intended to be used on Honda/Acura 4cylinders cars running on a distributor normally. (Mostly Serie B-D-F-H Engines)

This application samples in two distributor signals, (ICM & CYP), Ignition Control Module & Camshaft Position Positive
It syncs the firing sequence to the specific Cylinder from the CYP signal and then control the firing sequence with the ICM signal.
It activate 4x Digital ouputs pins (used to trigger the coils) depending on which cylinder should fire.
***********************************************************************/

#include <digitalWriteFast.h>

// MODIFIABLE PARAMETERS VARS
//bool WastedSpark        = true;        //wasted spark fire each cylinders on 2x of the 4x stroke rather than only 1x stroke
bool InternalDWellTime  = false;         //set time to true to spark the coils for the internal DWell time or to false to use the 'external' DWell time incoming from the interrupt
//int MAXDWELL            = 2900;         //Maximum DWell in Microseconds (Sparking time for coils), this value enter in count only if using the Internal DWell Time
//int MINDWELL            = 2100;         //Minimum DWell in Microseconds (Sparking time for coils), this value enter in count only if using the Internal DWell Time
//uint16_t BreakpointRPM  = 1300;         //The Breaking Point RPM where the CYP get offseted on the cylinders syncronization
//const byte fireOrder[]  = {4, 2, 1, 3}; //Basicly the firing order 1-3-4-2 can it be offseted!
const byte fireOrder[]  = {1, 3, 4, 2}; //Basicly the firing order 1-3-4-2 can it be offseted!
//const byte fireOrder[]  = {3, 4, 2, 1}; //Basicly the firing order 1-3-4-2 can it be offseted!
//const byte fireOrder[]  = {2, 1, 3, 4}; //Basicly the firing order 1-3-4-2 can it be offseted!

// MODIFIABLE OUTPUT PINS (just make sure you aren't using a pins listed in the Inputs Pins bellow
const byte CoilP1 = 3;      //Atmega328P=5  ATTiny44=3
const byte CoilP2 = 4;      //Atmega328P=6  ATTiny44=4
const byte CoilP3 = 5;      //Atmega328P=7  ATTiny44=5
const byte CoilP4 = 6;      //Atmega328P=8  ATTiny44=6
//const byte Status = 10;      //Atmega328P=10  ATTiny44=8

//#############################################################################################################
// INPUT PINS (DO NOT MODIFY, INPUT PINS ARE SPECIFIC FOR THE INTERRUPTS AND OTHER IMPORTANT FUNCTIONS)
const byte CYP = 2;         //Atmega328P=2  ATTiny44=1
const byte IGN = 3;         //Atmega328P=3  ATTiny44=2
//const byte WastedSparkIn = 9;//Atmega328P=9  ATTiny44=7
//const byte IntDWellIn = 11; //Atmega328P=9  ATTiny44=7

// VARS (DO NOT MODIFY)
uint8_t CYLINDER        = 0;
uint8_t CYLINDERTDC     = 1;  //Starting sync cylinder
uint8_t SPARKS          = 0;
uint8_t SPARKSCYP       = 0;
uint8_t SPARKSOUT       = 0;
uint16_t RPM            = 0;
uint16_t RPM2            = 0;
bool desynced           = true;
unsigned long LASTMILLIS = 0;
unsigned long LOWRPMCOUNT = 0;
int DWELL               = 2450;
int SyncCount           = 0;
int FailSyncTry         = 1;

int TestVal             = -1;
int TestVal2             = -1;
int TestVal3             = -1;


//unsigned long ontime,offtime,duty;
//float freq,period;

//#############################################################################################################
void setup() {
  CYLINDER = 0;

  //Set Outputs
  Ignite_Off();     //Turn Off all Coils before enabling Outputs
  pinModeFast(CoilP1, OUTPUT);
  pinModeFast(CoilP2, OUTPUT);
  pinModeFast(CoilP3, OUTPUT);
  pinModeFast(CoilP4, OUTPUT);
  //pinModeFast(Status, OUTPUT);

  //Set Inputs
  pinMode(CYP, INPUT);
  pinMode(IGN, INPUT);
  //pinMode(WastedSparkIn, INPUT_PULLUP);
  //pinMode(IntDWellIn, INPUT_PULLUP);

  //Attach Interrupts
  attachInterrupt(digitalPinToInterrupt(CYP), ISR_CYP, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IGN), ISR_IGN, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(CYP), ISR_CYP, FALLING);
  //attachInterrupt(digitalPinToInterrupt(IGN), ISR_IGN, FALLING);

  //Set RPM calculation and Serial logging Timer
  SetTimer();
  Serial.begin(115200);
}

void SetTimer() {
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0


  //################################################
  // set compare match register for 1hz increments
  //Formula : ((Clock) / (Prescalar * desired interrupt frequency))
  //          ((16,000,000) / (1 * (XXX Frequency))
  //          ((16*10^6)) / (1*1024) - 1 (must be <65536)
  //################################################
  uint16_t FreqHZ = 5;  //frequency in Hz
  OCR1A = ((16000000) / (1*(FreqHZ * 1024))) - 1;
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);


  //set timer2 interrupt at 8kHz
  /*TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);*/
  
  sei();//allow interrupts
}

//###############################################################
ISR(TIMER1_COMPA_vect) {
  //Turn off interrupts since we use Serial commands, this also ensure we calculate the RPM when nothing trigger the interrupts (interrupts turned off)
  noInterrupts();

  GetRPM();

  //Compare RPM calculated from CYP and RPM calulated from IGN to check if their any difference (differences occurs when the ignition cut get active)
  //int DiffRPM = (int) (RPM2 - RPM);
  //if (DiffRPM > 1500 || DiffRPM < -1500) {
    //SetDecynced();
  //}*/
  
  //Serial.println("RPM: " + String(RPM) + "rpm, RPM2: " + String(RPM2) + "rpm");
  //Serial.println(CYLINDERTDC);
  if (TestVal3 > 0) {
    Serial.println("RPM: " + String(RPM) + "rpm, RPM2: " + String(RPM2) + "rpm, " + String(TestVal) + "Cyl, " + String(TestVal2) + "Sparks, " + String(TestVal3) + "Sync");
  }

  TestVal3 = 0;

  //Attach again the interrupts since we finished calculating RPM and possibly sending Serial datas
  attachInterrupt(digitalPinToInterrupt(CYP), ISR_CYP, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IGN), ISR_IGN, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(CYP), ISR_CYP, FALLING);
  //attachInterrupt(digitalPinToInterrupt(IGN), ISR_IGN, FALLING);
}

void GetRPM() {
  //Calculate engine RPM
  uint16_t Thistime = millis() - LASTMILLIS;
  //RPM2 = (((30*1000)/(Thistime)*(SPARKSCYP)) / 1.71);
  RPM2 = (((30*1000)/(Thistime)*(SPARKSCYP * 4)) / 1.666);   //New FC1 Port Pin4 Method

  RPM = ((30*1000)/(Thistime)*SPARKS) / 1.666;
  /*uint16_t FakeRPM = (((SPARKS*1000)/Thistime)*60); //How much spark in a minutes
  float Divider = (float) ((FakeRPM/SPARKS) / 150.0);
  RPM = (FakeRPM / Divider) * 0.92;

  int OtherDivider = 1;
  if (RPM > 3100) OtherDivider = 2;
  uint16_t FakeRPM2 = ((((SPARKSCYP*4 * OtherDivider)*1000)/Thistime)*60); //How much spark in a minutes
  float Divider2 = (float) ((FakeRPM2/(SPARKSCYP*4 * OtherDivider)) / 150.0);
  RPM2 = (FakeRPM2 / Divider2) * 0.92;*/
  
  LASTMILLIS = millis();
  //Serial.println(String(SPARKSCYP));
  SPARKS = 0;
  SPARKSCYP = 0;

  //Serial.println(String(Thistime));
  
  //Set RPM #1 (RPM from ICM Wire Signal)
  /*ontime = pulseIn(IGN,HIGH);
  offtime = pulseIn(IGN,LOW);
  period = ontime+offtime;
  freq = 1000000.0/period;
  duty = (ontime/period)*100;
  if (duty > 0 && duty < 100) {
    RPM = freq * 27;
  }
  //Serial.println(String(offtime));

  //Set RPM #2 (RPM from CYP Wire Signal)
  //ontime = pulseIn(CYP,LOW);
  //offtime = pulseIn(CYP,HIGH);
  ontime = pulseIn(CYP,HIGH);
  offtime = pulseIn(CYP,LOW);
  period = ontime+offtime;
  freq = 1000000.0/period;
  duty = (ontime/period)*100;
  if (duty > 0 && duty < 100) {
    //RPM2 = (freq * 27) * 4;
    //RPM2 = (freq * 27);
    RPM2 = (freq * 27) * 8;   //Pin#4
    //RPM2 = (freq * 27) / 6 ;  //Pin#3
  }*/

  //Get WastedSparks
  /*if (digitalRead(WastedSparkIn) == LOW) {
    WastedSpark = true;
  }
  if (digitalRead(WastedSparkIn) == HIGH) {
    WastedSpark = false;
  }*/
  
  //Get InternalDWell
  /*if (digitalRead(IntDWellIn) == LOW) {
    InternalDWellTime = true;
    
    //Calucalute average DWell time with Engine RPM
    DWELL  = map(RPM, 0, 11000, MAXDWELL, MINDWELL);
  }
  if (digitalRead(IntDWellIn) == HIGH) {
    InternalDWellTime = false;
  }*/

  //Set Engine Not running (Low RPM detected)
  if (!desynced) {
    if (RPM2 < 60) {
    //if (RPM < 60 || RPM2 < 60) {
      LOWRPMCOUNT++;
      if (LOWRPMCOUNT > 8) {
        SetDecynced();
        LOWRPMCOUNT = 0;
      }
    }
    else {
      LOWRPMCOUNT = 0;
    }
  }
}

void SetDecynced() {
  desynced = true;
  Ignite_Off();
  CYLINDER = 0;
  LOWRPMCOUNT = 0;
  SyncCount = 0;
  SetLED();
}

//###############################################################
void loop() {
  //GetRPM();
  //CheckDesynced();
  //Serial.println(String(desynced));
  //Serial.println("RPM: " + String(RPM) + "rpm, RPM2: " + String(RPM2) + "rpm");
  //delay(1);
}

//###############################################################
void SetLED() {
  /*if (desynced){
    digitalWriteFast(Status, LOW);
  }
  if (!desynced){
    digitalWriteFast(Status, HIGH);
  }*/
}

void FIRE_COIL1(){
  digitalWriteFast(CoilP1, HIGH);
  /*if (WastedSpark) {
    digitalWriteFast(CoilP4, HIGH);
  }*/
}

void FIRE_COIL2(){
  digitalWriteFast(CoilP2, HIGH);
  /*if (WastedSpark) {
    digitalWriteFast(CoilP3, HIGH);
  }*/
}

void FIRE_COIL3(){
  digitalWriteFast(CoilP3, HIGH);
  /*if (WastedSpark) {
    digitalWriteFast(CoilP2, HIGH);
  }*/
}

void FIRE_COIL4(){
  digitalWriteFast(CoilP4, HIGH);
  /*if (WastedSpark) {
    digitalWriteFast(CoilP1, HIGH);
  }*/
}

void Ignite_ON() {
  //Get the specified cylinder to fire with the firing order
  if (CYLINDER > 0) {
    //fire the desired cylinder
    if (fireOrder[CYLINDER - 1] == 1) {
      FIRE_COIL1();
    }
    if (fireOrder[CYLINDER - 1] == 2) {
      FIRE_COIL2();
    }
    if (fireOrder[CYLINDER - 1] == 3) {
      FIRE_COIL3();
    }
    if (fireOrder[CYLINDER - 1] == 4) {
      FIRE_COIL4();
    }
  }
}

void Ignite_Off() {
  //Turn off All Coils
  digitalWriteFast(CoilP1, LOW);
  digitalWriteFast(CoilP2, LOW);
  digitalWriteFast(CoilP3, LOW);
  digitalWriteFast(CoilP4, LOW);
}

//###############################################################
void ISR_CYP() {
  if(digitalRead(CYP) == LOW) {
    //if (desynced) {
      TestVal = CYLINDER;
      TestVal3 = 555;
      CYLINDER = 1;

      //Check if the ICM signal fired the cylinders on the Next pulse (SyncCount==1)
      if (SyncCount == 0) {
        //if (SPARKSOUT == 4 || SPARKSOUT == 8) {
          desynced = false;
          SetLED();
          SyncCount++;
        //}
      }
    /*}
    else {
      if (CYLINDER == 1) {
        SyncCount = 1;
      }
      if (CYLINDER != 1) {
        if (SyncCount > FailSyncTry) {
          //TestVal = CYLINDER;
          CYLINDER = 1;
          TestVal3 = 444;
        }
        SyncCount++;
      }
    }*/

      /*if(digitalRead(IGN) == LOW) {
        CYLINDER = 4;
      }*/

      
    //}
    //else {
      //if (SPARKSOUT != 4 && SPARKSOUT != 8) {
        /*if ((SPARKSOUT == 7 && CYLINDER != 4) || (SPARKSOUT == 9 && CYLINDER != 2) || (SPARKSOUT != 7 && SPARKSOUT != 9)) {
          TestVal = CYLINDER;
          CYLINDER = 1;
          TestVal3 = 333;
        }*/
        
        //Check Diff
        /*if (SPARKSOUT > 4) SPARKSOUT = SPARKSOUT - 4;
        int OffsetCyl = 4 - SPARKSOUT;

        int NewCYL = (1 + 4) - OffsetCyl;
        if (NewCYL > 4) NewCYL = NewCYL - 4;
        if (CYLINDER == NewCYL) {
          TestVal3 = 444;
        }
        else {
          TestVal3 = 666;
        }
        CYLINDERTDC = CYLINDER;*/
        //###################################################
        /*bool IsLow = digitalRead(IGN) == LOW;
        if (CYLINDER != 1 && IsLow) {
          TestVal = CYLINDER;
          CYLINDER = 1;
          TestVal3 = 666;
        }
        if (CYLINDER != 4 && !IsLow) {
          TestVal = CYLINDER;
          CYLINDER = 4;
          TestVal3 = 444;
        }*/
      //}
      /*if (CYLINDER == 1) {
        SyncCount = 1;
      }
      if (CYLINDER != 1) {
        if (SyncCount > FailSyncTry) {
          //TestVal = CYLINDER;
          CYLINDER = 1;
          TestVal3 = 444;
        }
        SyncCount++;
      }*/

      
      /*if (CYLINDER != 4 && CYLINDER != 1) {
        TestVal = CYLINDER;
        CYLINDER = 1;
        TestVal3 = 444;
      }*/
      /*if (CYLINDER != 1) {
        TestVal = CYLINDER;
        CYLINDER = 1;
        TestVal3 = 555;
      }*/
    //}
    
    //TestVal = CYLINDER;
    //TestVal2 = SPARKSOUT;

    //apply 'sparks' for rpm calculation
    SPARKSCYP++;
    SPARKSOUT = 0;
  }
}

void ISR_IGN() {
  if(digitalRead(IGN) == LOW && CYLINDER > 0) {
    //Fire the specific Cylinder Coil
    if (!desynced) {
      Ignite_ON();
    }

    //Wait for DWell Time
    if (InternalDWellTime) {
      delayMicroseconds(DWELL); //##############
    }
    else {
      while (digitalRead(IGN) == LOW) {};
    }

    //Unfire the Coil, pass to next cylinder and apply 'sparks' for rpm calculation
    Ignite_Off();
    CYLINDER++;
    SPARKS++;     //Used for Engine RPM Calculation
    SPARKSOUT++;  //Used for Cylinder Resyncing
    
    //Turn back to cylinder #1 if we lost CYP signal
    if (CYLINDER > 4) {
      CYLINDER = 1;
    }
  }
  /*if(digitalRead(IGN) == HIGH) {
    Ignite_Off();
    CYLINDER++;
    SPARKS++;     //Used for Engine RPM Calculation
    SPARKSOUT++;  //Used for Cylinder Resyncing
    
    //Turn back to cylinder #1 if we lost CYP signal
    if (CYLINDER > 4) {
      CYLINDER = 1;
    }
  }*/
}
