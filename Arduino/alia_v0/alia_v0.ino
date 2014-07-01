/* Alia: Bringing back human connection (2nd place winner of #ExtremeWearables designathon)
 author: matt pinner @mpinner
 intent: connect exactly two people with wearable devices via xbees.
 FuelGauge: shows the sensors readings of the other person's device 
 Emote : recieve a message to the other
 Nudge : send a message to the other
 Sense : read local sensors
 Update : send sensor reading
 
 */
 
#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

#define INITIALSTATE 49

#define DEBUG false


Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);


int speakerPin = 13;	// speaker connected to digital pin 9

int increaseRate = 30000;
long lastincrease = increaseRate;


char inByte = 48;         // incoming serial byte

char state = INITIALSTATE;

#define BUTTON_PIN 2
#define LED_PIN 13



// Instantiate a Bounce object
Bounce debouncer = Bounce(); 

void setup()
{
  // Setup the button
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  //Setup the LED
  pinMode(LED_PIN,OUTPUT);

  // start serial port at 9600 bps and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  establishContact();  // send a byte to establish contact until receiver responds 
}

void loop()
{
  debouncer.update();
  //int value = debouncer.read();

  // Turn on or off the LED
  if ( digitalRead(BUTTON_PIN) == HIGH) {
    // Serial.println('0');
  } 
  else {

    // button press
    if(DEBUG)  Serial.println("button press");

    Serial.write(48);

    nudge();


  }

  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();

    if(DEBUG) {
      Serial.print(" IN:");
      Serial.println(inByte);
    }

    if ('0' == inByte) {
      emote();
    } 

    // updates the leds
    update();

  }

  sense();

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


  delay(250);

}

void emote() {

  lightEmUp(8);
  beep(speakerPin,50,145);	
  off();	
  beep(speakerPin,100,145);
  lightEmUp(8);

  beep(speakerPin,200,145);	
  off();	
  beep(speakerPin,400,145);	
  lightEmUp(8);
  beep(speakerPin,800,145);	
  beep(speakerPin,1600,1000);
  off();	

  state=INITIALSTATE;

  return;

}


void nudge() {

  lightEmUp(8);
  beep(speakerPin,50,500);	
  beep(speakerPin,100,500);	
  off();	


  return;

}
void sense() {
  // send sensor values:
  //if (DEBUG) 
  Serial.write(state);
}

void update() {

  int fuelLevel = inByte - 48;

  if (DEBUG) Serial.println("fuel:" + fuelLevel);


  for(int i=0; i< 16; i++) {
    strip.setPixelColor(i, 0, 0, 0);

    if (fuelLevel > i) {
      strip.setPixelColor(i, 30, 50, 60);
    }


  }

  strip.show();

  return;

}

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




void off (){

  lightEmUp(0);
  return;

}














