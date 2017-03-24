
#include <NewPing.h>
#include <IRremote.h>

//#define DEBUG

#define MAX_DISTANCE   200   // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define PING_SPEED    50    // Ping minimum frequences 

#define SPRAY_DELAY   500   // Spray duration
#define SPRAY_MAX_DIST  140   // Spray max distance to trigger
#define SPRAY_MIN_DIST  30    // Spray min distance to trigger

/*********************
 * DEFINE THE PINS   *
 *********************/
// pin to activate spray : HIGH = Trigged 
#define SPRAY_PIN   2
// pin to do a sound signal to determine the state of device ON(one bip)/OFF(two bip)
#define BUZZ_PIN  10
// IR received signal pin
#define RECV_PIN  9
// Distance sensor trigger and echo pins
#define TRIGGER_PIN 12
#define ECHO_PIN 8

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

IRrecv irrecv(RECV_PIN); // IR setup receive pin

decode_results results;

unsigned long pingTimer;  // Holds the next ping time.
unsigned long sprayTimer;   // Holds the spray duration, it should be no more than SPRAY_DURATION.     
unsigned long distance;   // current distance.

bool on; // device is live or not
bool was_changed; // determine if distance sensor was get
bool goSpray; //to force spray


void setup() {
#ifdef DEBUG
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
#endif
  // at init we don't spray and device state is off
  was_changed = false;
  on = false;
  // force the spray even is device state is off
  goSpray = false;

  //init pingTimer and spary timer
  pingTimer = sprayTimer = millis(); // Start now.

  //init spray and buzz pin to output
  pinMode(SPRAY_PIN, OUTPUT);  
  pinMode(BUZZ_PIN, OUTPUT) ;
  
  irrecv.enableIRIn(); // Initialise le recepteur
}

void loop() {
  // Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
  unsigned long current = millis();
    
  if (current >= pingTimer && on) {   // PING_SPEED milliseconds since last ping, do another ping.
    pingTimer += PING_SPEED;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
  }

  if(current >= sprayTimer ) {  // SPRAY_DELAY milliseconds since last ping, check people standed behind.
    if((distance <= SPRAY_MAX_DIST && distance > SPRAY_MIN_DIST && was_changed && on) || goSpray ) {
#ifdef DEBUG
      Serial.println("I'm going to spray !");
      Serial.print("distance: ");Serial.print(distance);Serial.println(" cm");
#endif 
      // start the spray
      digitalWrite(SPRAY_PIN, HIGH);
    // set the spray timer to stop
      sprayTimer = current + SPRAY_DELAY;
      // set people standed behind to state not changed 
      was_changed = false;
      // set gospray to no, cause the spray is spraying
      goSpray = false;
    }
    else {
      // stop spray (time > spraytimer)
      digitalWrite(SPRAY_PIN, LOW);
    }
  }

  // REMOTE results decoded
  if (irrecv.decode(&results)) {
    switch(results.value) {
      // received power pushed 
      case 0xffa25d:
        // change device state on <=> off
        on = !on;
        
        if(on) {
        // one long beep to say i'm ON  
          digitalWrite(BUZZ_PIN, HIGH);
          delay(100);
          digitalWrite(BUZZ_PIN, LOW);
        }
        else {
        // two short beeps to say i'm OFF  
          digitalWrite(BUZZ_PIN, HIGH);
          delay(75);
          digitalWrite(BUZZ_PIN, LOW);
          delay(50);
          digitalWrite(BUZZ_PIN, HIGH);
          delay(75);
          digitalWrite(BUZZ_PIN, LOW);
        }
#ifdef DEBUG
        Serial.println(on?"ON!!!":"OFF!!!");
#endif
        break;
      // received start pushed 
      case 0xff02fd:
        goSpray = true;
#ifdef DEBUG
        Serial.println("GO Spray!!!");
#endif
        break;
      default:
      Serial.println(results.value, HEX);
        break;
    }
    irrecv.resume(); // Recoit la valeur suivante
  }
  
}

//I changed the file IRRemoteint.h to listen on timer 1

void echoCheck() { // Timer2 (changed to timer1) interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    distance = sonar.ping_result / US_ROUNDTRIP_CM;
    was_changed = true;
  }
}
