/* Alia: sharing accountability and bringing back human connection 
 (2nd place winner of #ExtremeWearables designathon)
 Author: matt pinner @mpinner 
 (and team: Ella Jeong, M Asynaps, Jon Hsiung, Mary Kim, Divya Gaitnode)
 
 Intent: connect exactly two people with wearable devices via xbees.
 
 Terms:
 * fuelGauge: shows the sensors readings of the other person's device 
 * emote : recieve a message to the other
 * nudge : send a message to the other
 * sense : read and send local sensors
 * update : send sensor reading
 
 PINS:
 * 0/1 : tx/rx for Serial over XBee 
 * 2   : nudge button (INPUT_PULLUP)
 * 4/5 : adxl345 accelerometer (i2c)
 * 6   : neopixel x8 stick (ws2811)
 * 13  : vibration motor and onboard led (high/low modulated for power savings)
 
 BOM: https://www.sparkfun.com/wish_lists/90529
 * fio: http://arduino.cc/en/Main/ArduinoBoardFio
 * xbee: https://www.sparkfun.com/datasheets/Wireless/Zigbee/XBee-Datasheet.pdf
 * battery: https://www.sparkfun.com/datasheets/Batteries/UnionBattery-1000mAh.pdf 
 * button:https://www.sparkfun.com/datasheets/Components/General/00097.jpg
 * accelerometer: https://www.sparkfun.com/datasheets/Sensors/Accelerometer/ADXL345.pdf
 * leds: https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Components/LED/adafruit-neopixel-uberguide.pdf
 * vibrator: https://www.sparkfun.com/datasheets/Robotics/310-101_datasheet.pdf
 
 PARTS MODELS:
 * fio : https://github.com/sparkfun/3D_Models/tree/master/Products/Prototyping/Arduino%20Fio%20DEV-10116
 * xbee: https://github.com/sparkfun/3D_Models/blob/master/Products/Wireless/XBee%202mW%20PCB%20Antenna%20-%20Series%202%20WRL-11217.skp
 * battery: https://github.com/sparkfun/3D_Models/tree/master/Products/Prototyping/Polymer%20Lithium%20Ion%20Battery%20-%201000mAh%20PRT-00339
 * leds : https://github.com/adafruit/NeoPixel-Sticks | http://www.thingiverse.com/thing:371636
 
 */


#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>

// this NEEDS to be false for anything to work in a P2P mode. 
// this can be on for testing against a Serial monitor
#define DEBUG false

#define BUTTON_PIN 2
#define LEDS_PIN 6
#define VIBE_PIN 13

// We're sending BYTES for our given sensor readings and to nudge
// we save the state of our current reading to send so eventually we can do smoothing
// this defines the initial state for the CONNECTED reading and assumes no data is available
#define INITIALSTATE 49
#define NUDGEMSG 48



// setup the led strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, LEDS_PIN, NEO_GRB + NEO_KHZ800);

// we're currently faking readings for the demo
// we do this by pretending our readings are ever increasing
// this defines how quickly those readings should increase and tracks the last time of increase.
int increaseRate = 30000;
long lastincrease = increaseRate;


// saves the last incoming message from our peer
// 
char inByte = 48;         // incoming serial byte

// tracks our most recently sent data
// ideally this is derived from the current sensor reading with some smoothing
char state = INITIALSTATE;


// Instantiate a Bounce object. this makes using buttons happy.
// not sure this is really needed in this case.
Bounce debouncer = Bounce(); 



// THIS STUFF HAPPENS EXACTLY ONCE UPON INITIAL STARTUP
void setup()
{

  // Setup the button and pull high (hint: this simplifies the electronics)
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  //Setup the VIBRATION MOTOR
  pinMode(VIBE_PIN,OUTPUT);

  // start serial port at 57600 bps and wait for port to open
  // this is also the required baud rate for wireless programming so keep it here
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  establishContact();  // send a byte to establish contact until receiver responds 
}


// THIS STUFF HAPPENS OVER AND OVER AS FAST AS POSSIBLE
// hint: there is a bit of a delay in the end to avoid over saturating the serial buffers
void loop()
{
  // not used yet...
  //debouncer.update();
  //int value = debouncer.read();


  // Detect user's nudge
  if ( digitalRead(BUTTON_PIN) == HIGH) {
    // button is not pressed, do nothing
  } 
  else {

    // button pressed
    if(DEBUG)  Serial.println("button press");

    // nudge our peer and react locally
    nudge();

  }


  // if we see data from our peer do stuff
  if (Serial.available() > 0) {

    // save incoming byte data
    inByte = Serial.read();

    if(DEBUG) {
      Serial.print(" IN:");
      Serial.println(inByte);
    }

    // if they send us a nudge
    if (NUDGEMSG == inByte) {
      emote();
    } 

    // updates the leds
    update();

  }

  // read and send local sensed data
  sense();


  //avoids over saturating the serial buffers
  delay(250);

  return;
} // END OF loop()


// emote : recieve a nudge from the other
void emote() {

  lightEmUp(8);
  beep(VIBE_PIN,50,145);	
  off();	
  beep(VIBE_PIN,100,145);
  lightEmUp(8);

  beep(VIBE_PIN,200,145);	
  off();	
  beep(VIBE_PIN,400,145);	
  lightEmUp(8);
  beep(VIBE_PIN,800,145);	
  beep(VIBE_PIN,1600,1000);
  off();	

  state=INITIALSTATE;

  return;

}


// nudge our peer and react locally
void nudge() {


  // send peer our nudge 
  Serial.write(NUDGEMSG);

  lightEmUp(8);
  beep(VIBE_PIN,50,500);	
  beep(VIBE_PIN,100,500);	
  off();	


  return;

}


// sense : read and send local sensors
void sense() {
  // send sensor values:
  //if (DEBUG) 
  Serial.write(state);

  if ((millis() - increaseRate) > lastincrease) {
    if(DEBUG) {
      Serial.print(" STATE UPDATE:");
    }
    if (state < (8 + INITIALSTATE)) {
      state++;
      lastincrease = millis();
    }

    if(DEBUG) {
      Serial.println(state);
    }
  }
}


// update: shows the sensors readings of the other person's device 
void update() {

  // calc fuelLevel starting from 1
  int fuelLevel = inByte - NUDGEMSG;

  if (DEBUG) Serial.println("fuel:" + fuelLevel);

  // set all pixels
  for(int i=0; i< 16; i++) {
    strip.setPixelColor(i, 0, 0, 0);

    // turn on lower portion upto fuel level
    if (fuelLevel > i) {
      // pick a nice color
      strip.setPixelColor(i, 30, 50, 60);
    }

  }

  // communicate with pixels
  strip.show();

  return;

}

// make a sound (if speaker or buzzer) or vibration (if motor) or led flash at certain frequency.
// this is nice for power savings and to minimize the current draws
// i was a bit afraid of pulling too much current from my digial pins on the fio during testing with lots of different output devices
void beep (unsigned char speakerPin, int frequencyInHertz, long timeInMilliseconds)     // the sound producing function
{	 
  int x;	 
  long delayAmount = (long)(1000000/frequencyInHertz);
  long loopTime = (long)((timeInMilliseconds*1000)/(delayAmount*2));
  for (x=0;x<loopTime;x++)	 
  {	 
    digitalWrite(speakerPin,HIGH);
    delayMicroseconds(delayAmount);
    digitalWrite(speakerPin,LOW);
    delayMicroseconds(delayAmount);
  }	 
}

// sorta nice to see data upon completion of setup incase your loop is super screwed.
void establishContact() {
  // say hello    
  Serial.write(INITIALSTATE);   // send an initial string

  // not required
  /* while (Serial.available() <= 0) {
   Serial.write(49);   // send an initial string
   delay(500);
   }
   */
}

// util to color all the leds
void lightEmUp (int lights) {

  for(int i=0; i< 16; i++) {
    strip.setPixelColor(i, 0, 0, 0);

    if (lights > i) {
      strip.setPixelColor(i, 180, 200, 255);
    }

  }

  strip.show();
  return;

}



// util to off all the leds
void off (){

  lightEmUp(0);
  return;

}


















