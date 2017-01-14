/* 
Meant for the transmission of instructions to control an EL Sequencer. Based partially on Example code from:
YourDuinoStarter terry@yourduino.com
Uses the RF24 Library by TMRH20 and Maniacbug: https://github.com/TMRh20/RF24
Pinout goes here
  - CONNECTIONS:
   - nRF24L01 Radio Module: See http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo  // We're now using the +
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin 7
   4 - CSN to Arduino pin 8
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED
*/

/*-----( Import needed libraries )-----*/
#include <SPI.h>   // Comes with Arduino IDE
#include "RF24.h"  // Download and Install (See above)
#include "printf.h" // Needed for "printDetails" Takes up some memory
/*-----( Declare Constants and Pin Numbers )-----*/
#define  CE_PIN  7   // The pins to be used for CE and SN
#define  CSN_PIN 8
#define  FEEDBACK 6

#define MODE1_SW  A2  // The Joystick push-down switch, will be used as a Digital input
#define MODE2_SW  A3  // The Joystick push-down switch, will be used as a Digital input

#define ERRORPIN 3
#define STATUSPIN 4 

/*-----( Declare objects )-----*/
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus (usually) pins 7 & 8 (Can be changed) */
RF24 radio(CE_PIN, CSN_PIN);

/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node", "2Node"}; // These will be the names of the "Pipes"

unsigned long timeNow;  // Used to grab the current time, calculate delays
unsigned long started_waiting_at;
boolean timeout;       // Timeout? True or False
uint8_t counter;   //For auto swapping modes
//boolean hasHardware = false;
boolean hasHardware = true;

/**
  Create a data structure for transmitting and receiving data
  This allows many variables to be easily sent and received in a single transmission
  See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct {
 // unsigned long _micros;  // to save response times
  bool switchOn;          // The Joystick push-down switch
  bool switchOn2;          // The Joystick push-down switch
} myData;                 // This can be accessed in the form:  myData.Xposition  etc.


void setup()   /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(115200);  // MUST reset the Serial Monitor to 115200 (lower right of window )
  // NOTE: The "F" in the print statements means "unchangable data; save in Flash Memory to conserve SRAM"
  Serial.println(F("Setting up the transmitter. Get ready!"));
  printf_begin(); // Needed for "printDetails" Takes up some memory
  pinMode(MODE1_SW, INPUT_PULLUP);  // Pin A2 will be used as a digital input
  pinMode(MODE2_SW, INPUT_PULLUP);  // Pin A2 will be used as a digital input
  pinMode(ERRORPIN, OUTPUT);  
  pinMode(STATUSPIN, OUTPUT); 
  pinMode(FEEDBACK, OUTPUT);
  digitalWrite(FEEDBACK, LOW);
  radio.begin();          // Initialize the nRF24L01 Radio
  radio.setChannel(108);  // Above most WiFi frequencies
  radio.setDataRate(RF24_250KBPS); // Fast enough.. Better range

  // Set the Power Amplifier Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  // PALevelcan be one of four levels: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  //radio.setPALevel(RF24_PA_LOW);
   radio.setPALevel(RF24_PA_MAX);
 

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);

  // Start the radio listening for data
  radio.startListening();
counter = 0;
//  radio.printDetails(); //Uncomment to show LOTS of debugging information
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  radio.stopListening();                                    // First, stop listening so we can talk.

  if (hasHardware)  // Set in variables at top
  {
    /*********************( Read the Joystick positions )*************************/
    myData.switchOn  = !digitalRead(MODE1_SW);  // Invert the pulldown switch
    myData.switchOn2  = !digitalRead(MODE2_SW);  // Invert the pulldown switch

if (counter <= 10) {
  myData.switchOn = false;
  myData.switchOn2 = true;
} else {
  myData.switchOn = true;
  myData.switchOn2 = false;
}
 
  }
  else
  {
  myData.switchOn = true;
  myData.switchOn2 = false;
  }

 // myData._micros = micros();  // Send back for timing  //been having some issues with microseconds. Something about the difference between lilly pad and the uno

  if (myData.switchOn) {
     digitalWrite(STATUSPIN, HIGH);    
  } else {
     digitalWrite(STATUSPIN, LOW);
  }


  Serial.print(F("Now sending  -  "));
  Serial.print(myData.switchOn);
  Serial.println(myData.switchOn2);
  if (!radio.write( &myData, sizeof(myData) )) {            // Send data, checking for error ("!" means NOT)
    Serial.println(F("Transmit failed "));
    digitalWrite(FEEDBACK, LOW);
  } else {
  digitalWrite(FEEDBACK, HIGH);
  }

  radio.startListening();                                    // Now, continue listening

  started_waiting_at = micros();               // timeout period, get the current microseconds
  timeout = false;                            //  variable to indicate if a response was received or not

  while ( ! radio.available() ) {                            // While nothing is received
    if (micros() - started_waiting_at > 200000 ) {           // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;
      break;
    }
  }

  if ( timeout )
  { // Describe the results
    Serial.println(F("Response timed out -  no Acknowledge."));
    digitalWrite(ERRORPIN, HIGH);   // Only for transmission errors. For non matching data, use feedback pin
  }
  else
  {
    int tempx  = myData.switchOn;
    // Grab the response, compare, and send to Serial Monitor
    radio.read( &myData, sizeof(myData) );
    timeNow = micros();

    // Show it
    Serial.print(F("Sent the switch as "));
    Serial.println(tempx); 
    
    digitalWrite(ERRORPIN, LOW); 
    if (tempx != myData.switchOn) {
      Serial.println("MISMATCH!!!!");
      digitalWrite(FEEDBACK, HIGH);
    } else {
      Serial.println("good");
      digitalWrite(ERRORPIN, LOW); 
      digitalWrite(FEEDBACK, LOW); 
     // counter++;
    }
    //Serial.print(F(", Round-trip delay "));
  //  Serial.print(timeNow - myData._micros);
    //Serial.println(F(" microseconds "));
 


  }
   counter++;
  if (counter >= 20) {
    counter = 0;
  }
  // Send again after delay. When working OK, change to something like 100
  delay(600);

}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/

// NONE YET
//*********( THE END )***********

