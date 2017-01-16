/* 
Meant for the remote reception of instructions to control an EL Sequencer. Based partially on Example code from:
YourDuinoStarter terry@yourduino.com
Uses the RF24 Library by TMRH20 and Maniacbug: https://github.com/TMRh20/RF24
Pinout goes here

brendan.hinman@gmail.com
*/

/*-----( Import needed libraries )-----*/
#include <SPI.h>   // Comes with Arduino IDE
#include "RF24.h"  // Download and Install (See above)
#include "printf.h" // Needed for "printDetails" Takes up some memory
#include "ELCOIL.h" // Needed for "printDetails" Takes up some memory
/*-----( Declare Constants and Pin Numbers )-----*/
#define  CE_PIN  9   // The pins to be used for CE and SN
#define  CSN_PIN 10

/*-----( Declare objects )-----*/
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus (usually) pins 7 & 8 (Can be changed) */
RF24 radio(CE_PIN, CSN_PIN);


/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node", "2Node"}; // These will be the names of the "Pipes"

// Allows testing of radios and code without servo hardware. Set 'true' when servos connected
//boolean hasHardware = false;  // Allows testing of radios and code without Joystick hardware.
boolean hasHardware = true;

/**
  Create a data structure for transmitting and receiving data
  This allows many variables to be easily sent and received in a single transmission
  See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct {
  uint8_t switchOn;          // Mode 1 switch
  uint8_t switchOn2;          // Mode 2 switch
} myData;                 //

  uint8_t prevMode;
  uint8_t currMode;
  ELCOIL redRightArm(2,0);
  ELCOIL greenRightArm(3,0);
  ELCOIL redLeftArm(4,1);
  ELCOIL greenLeftArm(5,1);

  //Looks like we're going to need to split the EL cables such that they go up and down the legs. Not enough channels for full control of each limb plus anything else. 
  //Do we want something else? A white EL line? 
  //We could setup another separate line entirely with its own power supply. It'd be manual. Ugly! Interesting idea though. 
    
  ELCOIL redRightLeg(6,2);
  ELCOIL greenRightLeg(7,2);
  ELCOIL redLeftLeg(8,3);
  ELCOIL greenLeftLeg(9,3);

ELCOIL coils[8] = {redRightArm, greenRightArm, redLeftArm, greenLeftArm, redRightLeg, greenRightLeg, redLeftLeg, greenLeftLeg};

void setup()   /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(115200);   // MUST reset the Serial Monitor to 115200 (lower right of window )
  // NOTE: The "F" in the print statements means "unchangable data; save in Flash Memory to conserve SRAM"
  Serial.println(F("Starting up the receiver!"));  
  //printf_begin(); // Needed for "printDetails" Takes up some memory

/*
  pinMode(2, OUTPUT);  // channel A  
  pinMode(3, OUTPUT);  // channel B   
  pinMode(4, OUTPUT);  // channel C
  pinMode(5, OUTPUT);  // channel D    
  pinMode(6, OUTPUT);  // channel E
  pinMode(7, OUTPUT);  // channel F
  pinMode(8, OUTPUT);  // channel G
  pinMode(9, OUTPUT);  // channel H
*/
 greenRightArm.on();
  prevMode = 0;
  currMode = 1;
  radio.begin();          // Initialize the nRF24L01 Radio
  radio.setChannel(108);  // 2.508 Ghz - Above most Wifi Channels
  radio.setDataRate(RF24_250KBPS); // Fast enough.. Better range

  // Set the Power Amplifier Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  // PALevelcan be one of four levels: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  //radio.setPALevel(RF24_PA_LOW);
    radio.setPALevel(RF24_PA_MAX);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start the radio listening for data
  radio.startListening();
 
//  radio.printDetails(); //Uncomment to show LOTS of debugging information
}//--(end setup )---

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{

  if ( radio.available())
  {

    while (radio.available())   // While there is data ready to be retrieved from the receive pipe
    {
      radio.read( &myData, sizeof(myData) );             // Get the data

    }

    radio.stopListening();                               // First, stop listening so we can transmit
    radio.write( &myData, sizeof(myData) );              // Send the received data back.
    radio.startListening();                              // Now, resume listening so we catch the next packets.

    Serial.println(F("Packet Received - Sent response "));  // Print the received packet data
    Serial.print("here is the mode from that packet!");
    Serial.println(myData.switchOn);
    currMode = myData.switchOn;

/*



    
    if ( myData.switchOn == 1)
    {    
      digitalWrite(2, HIGH);
      digitalWrite(5, HIGH);
      Serial.println(F(" Switch ON"));
    }
    else
    {
      digitalWrite(2,LOW);  
      digitalWrite(5, LOW);    
    }

    if ( myData.switchOn2 == 1)
    {    
      digitalWrite(3, HIGH);
      digitalWrite(4, HIGH);
      Serial.println(F(" Switch ON"));
    }
    else
    {
      digitalWrite(3,LOW);      
      digitalWrite(4,LOW);      
    }
*/
  } // END radio available



if (currMode != prevMode) {
  
    for (uint8_t i = 0; i <= 8; i++) {
      coils[i].off();
    }
    prevMode = currMode;
    Serial.println("mode changed!");
}


/*
1 All green on (2 - 4 coils) "good skip"
2 All red on (2 - 4 coils) "bad skip"
3 Red and Green on both sides "double"
4 Red on left, green on right "split"
5 Red on right, green on left "r_split"
6 all red distress flash "red_distress"
7 all green distress flash "green_distress"
8 green/red distress flash "double_distress"
9 All coils on with potentially an extra color ///means we have to split some channels up and down leg. Redundancy says we make it one color on one side and another color on the other. 
10 All coils off
*/

switch (myData.switchOn) {
    case 1:
      //1 All green on (2 - 4 coils) "good skip"
    
       greenRightArm.on();
       greenLeftArm.on();
       greenRightLeg.on();
       greenLeftLeg.on();
      break;
    case 2:
    
       redRightArm.on();
       redLeftArm.on();
       redRightLeg.on();
       redLeftLeg.on();
      break;
    case 3:
    
    for (uint8_t i = 0; i <= 8; i++) {
      coils[i].on();
    }
    break;
    case 4:
       redLeftLeg.on();
       redLeftArm.on();
       greenRightLeg.on();
       greenRightArm.on();
    break;
    case 5:
       greenLeftArm.on();
       greenLeftLeg.on();
       redRightArm.on();
       redRightLeg.on();
    break;
    case 6:
       redRightArm.dFlash();
       redLeftArm.dFlash();
       redRightLeg.dFlash();
       redLeftLeg.dFlash();
    break;
    case 7:
       greenRightArm.dFlash();
       greenLeftArm.dFlash();
       greenRightLeg.dFlash();
       greenLeftLeg.dFlash();
    break;
    case 8:
       redRightArm.dFlash();
       redLeftArm.dFlash();
       redRightLeg.dFlash();
       redLeftLeg.dFlash();
       greenRightArm.dFlash();
       greenLeftArm.dFlash();
       greenRightLeg.dFlash();
       greenLeftLeg.dFlash();
    break;  
    default:   
      
    break;
  }


 /* if (hasHardware)
  {
    
    if ( myData.switchOn == 1)
      {
      //  digitalWrite(2, HIGH);
        digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
          digitalWrite(9, HIGH);   // turn the LED on (HIGH is the voltage level)
      }
    else
      {
        digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
       digitalWrite(9, LOW);   // turn the LED on (HIGH is the voltage level)
      }
  } */ // END hasHardware 
}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/

// NONE YET
//*********( THE END )***********
