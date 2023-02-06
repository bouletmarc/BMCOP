/**********************************************************************
       __________  ______      ____  _   __   ____  __    __  ________
      / ____/ __ \/  _/ /     / __ \/ | / /  / __ \/ /   / / / / ____/
     / /   / / / // // /     / / / /  |/ /  / /_/ / /   / / / / / __  
    / /___/ /_/ // // /___  / /_/ / /|  /  / ____/ /___/ /_/ / /_/ /  
    \____/\____/___/_____/  \____/_/ |_/  /_/   /_____/\____/\____/   
    
COIL ON PLUG Script V1.3 for Atmega328P made by Bouletmarc (BMDevs)

This Coil On Plug script are intended to be used on Honda/Acura 4cylinders cars running on a distributor normally. (Mostly Serie B-D-F-H Engines)

This application samples require two signals, (ICM & FC1 Pin3)
It syncs the firing sequence to the specific Cylinder from the FC1 Pin3 signal and then control the firing sequence to the coils with the ICM signal.
It activate 4x Digital ouputs pins (used to trigger each coils) depending on which cylinder should fire.

Connection wiring:
On the ECU, at the FC1 header location, connect the pin1 to 5V on the arduino
On the ECU, at the FC1 header location, connect the pin3 to D2 on the arduino
On the ECU, at the FC1 header location, connect the pin12 to GND on the arduino
On the ECU, at the big connectors outputs, on the big connector A, CUT the ICM wire going to the body/engine bay and connect the wire coming from the ECU connector to D3 on the arduino
On the Arduino, connect D8 with the ICM wire previously cut going to your body (to power the tachometer)
On the Arduino, connect D4 with the Coil1 5v signal
On the Arduino, connect D5 with the Coil2 5v signal
On the Arduino, connect D6 with the Coil3 5v signal
On the Arduino, connect D7 with the Coil4 5v signal

RPM/SYNC Informations:
-The ICM signal become High state first
-The FC1 signal will trigger 6times High-Low state
-The ICM signal become Low then rapididly High state again
-The FC1 signal will trigger 12times High-Low state
-The ICM signal become Low then rapididly High state again
-The FC1 signal will trigger 4times High-Low state
--The ICM signal become Low then rapididly High state again --> but this time it's to fire the first coil!!
-The FC1 signal will trigger 5-6times High-Low state (depending on ignition timing) between each coils firing
-The FC1 signal doesn't loose timing while under ignition cut circonstances

----------------------------------------------------------------
ICM Signal:  ____---------------_-----------------------_-------_-----------_-----------_-----------_-----------_-----------_-----------_
FC1 Signal:  ________-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
----------------------------------------------------------------

***********************************************************************/

#include <digitalWriteFast.h>

// MODIFIABLE PARAMETERS VARS
const bool WastedSpark          = false;        //wasted spark fire each cylinders on 2x of the 4x stroke rather than only 1x stroke
const bool InternalDWellTime    = false;        //set this to true to spark the coils with the internal DWell times or to false to use the 'external' DWell time incoming from the ICM interrupt
const int DWELL_INTERNAL        = 2500;         // DWell in Microseconds (Sparking time for coils), this value enter in count only if using the Internal DWell Time
const byte fireOrder[]          = {1, 3, 4, 2}; //Basicly the firing order 1-3-4-2 can it be offseted!  ##### {4, 2, 1, 3} ##### {3, 4, 2, 1} ##### {2, 1, 3, 4}

// MODIFIABLE OUTPUT PINS (just make sure you aren't using a pins listed in the Inputs Pins bellow
const byte CoilP1 = 4;      //Atmega328P=4  ATTiny44=3
const byte CoilP2 = 5;      //Atmega328P=5  ATTiny44=4
const byte CoilP3 = 6;      //Atmega328P=6  ATTiny44=5
const byte CoilP4 = 7;      //Atmega328P=7  ATTiny44=6
const byte RPMOut = 8;      //Atmega328P=8  ATTiny44=6
//const byte Status = 10;      //Atmega328P=10  ATTiny44=8

//#############################################################################################################
// INPUT PINS (DO NOT MODIFY, INPUT PINS ARE SPECIFIC FOR THE INTERRUPTS AND OTHER IMPORTANT FUNCTIONS)
//#############################################################################################################
const byte FC1_Pin3 = 2;         //Atmega328P=2  ATTiny44=1
const byte ICM_Pin = 3;         //Atmega328P=3  ATTiny44=2

// VARS (DO NOT MODIFY)
uint8_t CYLINDER        = 0;
bool desynced           = true;
int SyncCount           = 0;
int SyncCountICM        = 0;
int LastSyncCount       = 0;
int DesyncCount         = 0;
int SyncMode            = 0;
const int MaxDesyncTime = 500; //max desync timer in milliseconds

int LogsyncCount         = 0;

// VARS FOR RPM SERIAL LOGGING
//uint8_t SPARKS          = 0;
//uint16_t RPM            = 0;
//unsigned long LASTMILLIS = 0;

//#############################################################################################################
// SETUP/STARTING CODES
//#############################################################################################################
void setup() {
  CYLINDER = 0;

  //Set Outputs
  Ignite_Off();     //Turn Off all Coils before enabling Outputs
  pinModeFast(CoilP1, OUTPUT);
  pinModeFast(CoilP2, OUTPUT);
  pinModeFast(CoilP3, OUTPUT);
  pinModeFast(CoilP4, OUTPUT);
  pinModeFast(RPMOut, OUTPUT);
  //pinModeFast(Status, OUTPUT);

  //Set Inputs
  pinMode(FC1_Pin3, INPUT);
  pinMode(ICM_Pin, INPUT);

  //Attach Interrupts
  attachInterrupt(digitalPinToInterrupt(FC1_Pin3), ISR_FC1_Pin3, RISING);
  attachInterrupt(digitalPinToInterrupt(ICM_Pin), ISR_ICM_Pin, FALLING);

  //Set RPM Serial logging Timer
  //SetTimer();
  //Serial.begin(115200);
}

//###############################################################
// MAIN RUNNING CODES
//###############################################################
void loop() {
  delay(1);
  CalculateIsSynced();

  //Calucalute average DWell time with Engine RPM
  //DWELL_INTERNAL  = map(RPM, 0, 11000, MAXDWELL, MINDWELL);

  //Serial.println("SyncCount=" + String(SyncCount) + ", SyncCountICM=" + String(SyncCountICM) + ", LogsyncCount=" + String(LogsyncCount));
}

void CalculateIsSynced() {
  //if (!desynced) {
    if (SyncCount == LastSyncCount) {
      DesyncCount++;
    }
    if (SyncCount != LastSyncCount) {
      DesyncCount = 0;
    }
    if (DesyncCount >= MaxDesyncTime) {
      SetDecynced();
    }
    
    LastSyncCount = SyncCount;
  //}
}

void SetDecynced() {
  desynced = true;
  Ignite_Off();
  CYLINDER = 0;
  SyncCount = 0;
  SyncCountICM = 0;
  DesyncCount = 0;
  LastSyncCount = 0;
  SyncMode = 0;
}

//###############################################################
// COILS FIRING FUNCTIONS
//###############################################################
void FIRE_COIL1(){
  digitalWriteFast(CoilP1, HIGH);
  if (WastedSpark) {
    digitalWriteFast(CoilP4, HIGH);
  }
}

void FIRE_COIL2(){
  digitalWriteFast(CoilP2, HIGH);
  if (WastedSpark) {
    digitalWriteFast(CoilP3, HIGH);
  }
}

void FIRE_COIL3(){
  digitalWriteFast(CoilP3, HIGH);
  if (WastedSpark) {
    digitalWriteFast(CoilP2, HIGH);
  }
}

void FIRE_COIL4(){
  digitalWriteFast(CoilP4, HIGH);
  if (WastedSpark) {
    digitalWriteFast(CoilP1, HIGH);
  }
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

    //Fire ICM Out for Tachometer
    digitalWriteFast(RPMOut, LOW);
  }
}

void Ignite_Off() {
  //Turn off All Coils
  digitalWriteFast(CoilP1, LOW);
  digitalWriteFast(CoilP2, LOW);
  digitalWriteFast(CoilP3, LOW);
  digitalWriteFast(CoilP4, LOW);
  digitalWriteFast(RPMOut, HIGH);
}

//###############################################################
// INTERRUPTS FC1 AND ICM INPUTS
//###############################################################
void ISR_FC1_Pin3() {
  SyncCount++;
    
  if (!desynced)
  {
    //Increase Cylinder firing on the 5th FC1 pulse, prepare it for when the next ICM pulse will fire.
    //If the ignition degree are negative, the ICM will pulse slightly before the 6th FC1 pulse, better prepare it in advance on the 5th pulse!
    if (SyncCount == 5)
    {
      CYLINDER++;
      if (CYLINDER > 4) {
        CYLINDER = 1;
      }
      SyncCount = 0;
    }

    //Reset Sync Counting every 6x FC1 pulses
    if (SyncCount == 6) {
      SyncCount = 0;
    }
  }
}

void ISR_ICM_Pin() {
  //START SYNC SEQUENCE
  if (desynced)
  {
    //AFTER UNKNOWN FC1 PULSE
    if (SyncCountICM == 0 && SyncCount > 0)
    {
      SyncCountICM++;
      SyncCount = 0;
    }
    //AFTER 5-6 or 11-12x FC1 PULSE
    if (SyncCountICM == 1 && (SyncCount == 5 || SyncCount == 6 || SyncCount == 11 || SyncCount == 12))
    {
      SyncCountICM++;
      SyncCount = 0;
      if (SyncCount == 5 || SyncCount == 6) {
        SyncMode = 1;
      }
    }
    //AFTER UNKNOWN FC1 PULSE
    if (SyncMode == 0) {
      if (SyncCountICM == 2 && SyncCount > 0)
      {
        SyncCountICM++;
        SyncCount = 0;
        CYLINDER = 1;
        desynced = false;
      }
    }
    
    if (SyncMode == 1) {
      if (SyncCountICM <= 4 && (SyncCount == 5 || SyncCount == 6))
      {
        SyncCountICM++;
        SyncCount = 0;
      }
      if (SyncCountICM == 5 && SyncCount > 0) {
        SyncCountICM++;
        SyncCount = 0;
        CYLINDER = 1;
        desynced = false;
      }
    }
  }
  
  if(!desynced) {
    //Fire the specific Cylinder Coil
    Ignite_ON();

    //Wait for DWell Time
    if (InternalDWellTime) {
      delayMicroseconds(DWELL_INTERNAL);
    }
    else {
      while (digitalRead(ICM_Pin) == LOW) {};
    }

    //Unfire the Coils
    Ignite_Off();
  }
}



//#############################################################################################################
//#############################################################################################################
//#############################################################################################################
//#############################################################################################################
//#############################################################################################################
// RPM SERIAL LOGGING CODES
//#############################################################################################################
/*void SetTimer() {
  cli();//stop interrupts

  //set timer1 interrupt at 5Hz
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
  
  sei();//allow interrupts
}

//###############################################################
ISR(TIMER1_COMPA_vect) {
  //Turn off interrupts since we use Serial commands, this also ensure we calculate the RPM when nothing trigger the interrupts (interrupts turned off)
  noInterrupts();

  //GetRPM();
  //Serial.println("RPM: " + String(RPM) + "rpm");
  Serial.println("SyncCount=" + String(SyncCount) + ", SyncCountICM=" + String(SyncCountICM) + ", LogsyncCount=" + String(LogsyncCount));

  //Attach again the interrupts since we finished calculating RPM and possibly sending Serial datas
  attachInterrupt(digitalPinToInterrupt(FC1_Pin3), ISR_FC1_Pin3, FALLING);
  attachInterrupt(digitalPinToInterrupt(ICM_Pin), ISR_ICM_Pin, RISING);
}

void GetRPM() {
  //Calculate engine RPM
  uint16_t Thistime = millis() - LASTMILLIS;
  RPM = ((30*1000)/(Thistime)*SPARKS) / 1.666;
  
  LASTMILLIS = millis();
  SPARKS = 0;
}*/
