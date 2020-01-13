#include "SevSeg.h"

// -- Compiler Options --
//#define TRACE      // Used for debugging
#define USE_7_SEG  // If defined Display will show light intensity value (as Percentage)
#define USE_AD_BOX // If defined LED will only activate when signaled by AD box
// ----------------------

#define LED_PIN 5
#define AD_INPUT A5 //28 



SevSeg sevseg; //Instantiate a seven segment controller object


boolean A_set;
boolean B_set;

volatile boolean fired = false;
volatile long rotaryCount = 0;

void setup() {
  //-------------- Encoder -------------------------
 digitalWrite (2, HIGH);   // activate pull-up resistors
 digitalWrite (3, HIGH); 
 
 attachInterrupt (0, isr, CHANGE);   // pin 2
 attachInterrupt (1, isr, CHANGE);   // pin 3
  //------------------------------------------------
  
  //----------------- 7 Seg Display --------------------------------
  byte numDigits = 3;
  byte digitPins[] = {14, 15, 4};
  byte segmentPins[] = {6, 8, 11, 10, 9, 7, 12, 13};
  bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(90);
  //-----------------------------------------------------------


  pinMode(LED_PIN, OUTPUT); // Initialize Led Pin
  #ifdef USE_AD_BOX
    pinMode(AD_INPUT, INPUT); // Initialize AD input pin
  #endif

  #ifdef TRACE
    Serial.begin (9600);
  #endif
}

void loop() {
  #ifdef USE_7_SEG
    sevseg.setNumber(rotaryCount, 0); //* 100 / 255, 0); //Set 7 Seg Display - Convert range 0-255 to range 0-100
    sevseg.refreshDisplay();          // Must run repeatedly
  #endif

  #ifdef USE_AD_BOX
    if(digitalRead(AD_INPUT)){   
      analogWrite(LED_PIN, rotaryCount); // Turn On LED if AD Box has signaled active
    }else
    {
      analogWrite(LED_PIN, 0); //Trun off LED
    }
    
  #else
    analogWrite(LED_PIN, rotaryCount); // Turn On LED
  #endif

  if (fired)
   {
   #ifdef TRACE
    Serial.print ("Count = ");  
    Serial.println (rotaryCount);
   #endif
   fired = false;
   }  // end if fired
}

// Interrupt Service Routine
void isr ()
{
 
static boolean ready;
static unsigned long lastFiredTime;
static byte pinA, pinB;  

// wait for main program to process it
 if (fired)
   return;
   
 byte newPinA = digitalRead (2);
 byte newPinB = digitalRead (3);
 
 // Forward is: LH/HH or HL/LL
 // Reverse is: HL/HH or LH/LL
 
 // so we only record a turn on both the same (HH or LL)
 
 if (newPinA == newPinB)
   {
   if (ready)
     {
     int increment = 1;
       
     // if they turn the encoder faster, make the count go up more
     // (use for humans, not for measuring ticks on a machine)
     unsigned long now = millis ();
     unsigned long interval = now - lastFiredTime;
     lastFiredTime = now;

     if (interval < 10)
       increment = 5;
     else if (interval < 20)
       increment = 3;
     else if (interval < 50)
       increment = 2;
     

      if (newPinA == HIGH)  // must be HH now
       {
       if (pinA == LOW)  
         //rotaryCount -= increment;
         AddCount(increment * -1);
       else
         //rotaryCount += increment;  
         AddCount(increment);      
       }
     else
       {                  // must be LL now
       if (pinA == LOW)  
         //rotaryCount += increment;  
         AddCount(increment);
       else
         //rotaryCount -= increment;
         AddCount(increment * -1);   
       }
     

     
     
     fired = true;
     ready = false;
     }  // end of being ready
   }  // end of completed click
 else
   ready = true;
   
 pinA = newPinA;
 pinB = newPinB;
}  // end of isr


static int sudoCount = 0;
void AddCount(int num)
{
  int offset = 4;
  //static int sudoCount = rotaryCount * offset;  
  
  if(num >= 0){
    if(sudoCount + num > 255 * offset){
      sudoCount = 255 * offset;
    }else{
      sudoCount += num;
    }
  }else{
    if(sudoCount + num < 0){
      rotaryCount = 0;
    }else{
      sudoCount += num;
    }
  }

  rotaryCount = sudoCount / offset;
}
