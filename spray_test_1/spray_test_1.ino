
//#define LCD_ON 1

#ifdef LCD_ON
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#endif

#include <NewPing.h>
#include <IRremote.h>

/************ 
 *    LCD
 ************/
#ifdef LCD_ON
#define LCD_I2C_ADDR	0x3F  // Define I2C Address where the PCF8574A is
#define LCD_BACKLIGHT_PIN 	3
#define LCD_EN_PIN  2
#define LCD_RW_PIN  1
#define LCD_RS_PIN  0
#define LCD_D4_PIN  4
#define LCD_D5_PIN  5
#define LCD_D6_PIN  6
#define LCD_D7_PIN  7

LiquidCrystal_I2C     lcd(LCD_I2C_ADDR,LCD_EN_PIN,LCD_RW_PIN,LCD_RS_PIN,LCD_D4_PIN,LCD_D5_PIN,LCD_D6_PIN,LCD_D7_PIN);
#endif

//Distance sensor
#define TRIGGER_PIN 12
#define ECHO_PIN 8
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define PING_SPEED 50


#define SPRAY_DELAY 500
#define SPRAY_DISTANCE 140

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


//Flashing LED on Arduino board
#define LED_PIN 13
#define SPRAY_PIN 2
#define BUZZ_PIN 10

//IRremote
#define RECV_PIN 9

IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned long pingTimer, sprayTimer;     // Holds the next ping time.
unsigned long distance;
bool was_changed;


bool on,goSpray;


void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  pingTimer = sprayTimer = millis(); // Start now.
  was_changed = false;
#ifdef LCD_ON
  lcd.begin (16,2);  //Size of LCD
// Switch on the backlight
  lcd.setBacklightPin(LCD_BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
#endif
  pinMode(SPRAY_PIN, OUTPUT);  
  pinMode (BUZZ_PIN, OUTPUT) ;
  
  irrecv.enableIRIn(); // Initialise le recepteur
  on = false;
  goSpray = false;
}

void loop() {
  // Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
  unsigned long current = millis();
  if (current >= pingTimer && on) {   // PING_SPEED milliseconds since last ping, do another ping.
    pingTimer += PING_SPEED;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
  }
#ifdef LCD_ON
 //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print("cm");
   // Do other stuff here, really. Think of it as multi-tasking.
#endif

  if(current >= sprayTimer ) {
    if((distance <= SPRAY_DISTANCE && distance > 30 && was_changed && on) || goSpray ) {
    Serial.println("I'm going to spray !");
    Serial.print("distance: ");Serial.print(distance);Serial.println(" cm");
      digitalWrite(SPRAY_PIN, HIGH);
      sprayTimer = current + SPRAY_DELAY;
      was_changed = false;
      goSpray = false;
    }
    else {
      digitalWrite(SPRAY_PIN, LOW);
    }
  }
  
  if (irrecv.decode(&results)) {
    switch(results.value) {
      case 0xffa25d:
        on = !on;
        goSpray = false;
        if(on) {
          digitalWrite(BUZZ_PIN, HIGH);
          delay(100);
          digitalWrite(BUZZ_PIN, LOW);
        }
        else {
          digitalWrite(BUZZ_PIN, HIGH);
          delay(75);
          digitalWrite(BUZZ_PIN, LOW);
          delay(50);
          digitalWrite(BUZZ_PIN, HIGH);
          delay(75);
          digitalWrite(BUZZ_PIN, LOW);
        }
        Serial.println(on?"ON!!!":"OFF!!!");
        break;
      case 0xff02fd:
        goSpray = true;
        Serial.println("GO Spray!!!");
        break;
      default:
      Serial.println(results.value, HEX);
        break;
    }
    irrecv.resume(); // Recoit la valeur suivante
  }
  
}

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
   // Serial.print("Ping: ");
    distance = sonar.ping_result / US_ROUNDTRIP_CM;
    //Serial.print(distance); // Ping returned, uS result in ping_result, convert to cm with US_ROUNDTRIP_CM.
    //Serial.println("cm  ");
    was_changed = true;
  
  }
}
