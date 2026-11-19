// Program demonstrating how to control a powerSTEP01-based ST X-NUCLEO-IHM03A1 
// stepper motor driver shield on an Arduino Uno-compatible board

#include <Ponoor_PowerSTEP01Library.h>
#include <SPI.h>

// Pin definitions for the X-NUCLEO-IHM03A1 connected to an Uno-compatible board
#define nCS_PIN 10
#define STCK_PIN 9
#define nSTBY_nRESET_PIN 8
#define nBUSY_PIN 4
#define FLAG_PIN 2

// powerSTEP library instance, parameters are distance from the end of a daisy-chain
// of drivers, !CS pin, !STBY/!Reset pin
powerSTEP driver(0, nCS_PIN, nSTBY_nRESET_PIN);

// State variables
bool motorRunning = false; // Indicates whether the motor is running
bool movingForward = true; // Direction of the motor
bool lastButtonState = LOW; // Last state of the button
unsigned long lastDebounceTime = 0; // Last time the button state was checked
unsigned long debounceDelay = 100; // Debounce time in milliseconds

void setup() 
{
  // Start serial
  Serial.begin(9600);
  Serial.println("powerSTEP01 Arduino control initialising...");

  // Prepare pins
  pinMode(nSTBY_nRESET_PIN, OUTPUT);
  pinMode(nCS_PIN, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(FLAG_PIN, INPUT); // Set FLAG_PIN as input with pull-up resistor

  // Reset powerSTEP and set CS
  digitalWrite(nSTBY_nRESET_PIN, HIGH);
  digitalWrite(nSTBY_nRESET_PIN, LOW);
  digitalWrite(nSTBY_nRESET_PIN, HIGH);
  digitalWrite(nCS_PIN, HIGH);

  // Start SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);

  // Configure powerSTEP
  driver.SPIPortConnect(&SPI); // give library the SPI port (only the one on an Uno)
  
  driver.configSyncPin(BUSY_PIN, 0); // use SYNC/nBUSY pin as nBUSY, 
  driver.configStepMode(STEP_FS_128); // 1/128 microstepping, full steps = STEP_FS
  driver.setMaxSpeed(200); // max speed in units of full steps/s 
  driver.setFullSpeed(400); // full steps/s threshold for disabling microstepping
  driver.setAcc(2000); // full steps/s^2 acceleration
  driver.setDec(2000); // full steps/s^2 deceleration
  driver.setSlewRate(SR_520V_us); // slew rate setting
  driver.setOCThreshold(8); // over-current threshold
  driver.setOCShutdown(OC_SD_ENABLE); // shutdown motor bridge on over-current
  driver.setPWMFreq(PWM_DIV_1, PWM_MUL_0_75); // PWM frequency setting
  driver.setVoltageComp(VS_COMP_DISABLE); // no voltage compensation
  driver.setSwitchMode(SW_USER); // switch doesn't trigger stop
  driver.setOscMode(INT_16MHZ); // 16MHz internal oscillator as clock source

  // KVAL registers for motor power settings
  driver.setRunKVAL(64);
  driver.setAccKVAL(64);
  driver.setDecKVAL(64);
  driver.setHoldKVAL(8);
  
  driver.setParam(ALARM_EN, 0x8F); // disable alarm settings
  driver.getStatus(); // clears error flags

  Serial.println(F("Initialisation complete"));
}

// Zusätzliche Variable für die Anzahl der Tastendrücke
int buttonPressCount = 0;

void loop() 
{ 
  // Aktuellen Tasterstatus lesen
  int buttonState = digitalRead(FLAG_PIN);

  // Überprüfen, ob der Taster gedrückt wurde
  if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    buttonPressCount++; // Zähler für Tastendrücke erhöhen
    lastDebounceTime = millis(); // Entprellzeit aktualisieren

    if (buttonPressCount >= 3) {
      // Motor stoppen nach drei Tastendrücken
      driver.softStop();
      Serial.println("Motor angehalten nach drei Tastendrücken.");
      motorRunning = false; // Motorstatus aktualisieren
      buttonPressCount = 0; // Zähler zurücksetzen
    } else {
      // Richtung ändern und Motor starten
      movingForward = !movingForward;
      Serial.println(movingForward ? "Ändere Richtung: Vorwärts" : "Ändere Richtung: Rückwärts");

      // Motor mit fixer Geschwindigkeit starten
      driver.run(movingForward ? FWD : REV, 51); //2.28=0.05mm/S, 4.48 = 0.1mm/s, 12.5 = 0.25mm/s,  25 = 0.5mm/s, 51 = 1mm/s, 102 =2mm/s
    }
  }

  lastButtonState = buttonState; // Aktuellen Tasterstatus speichern
}



