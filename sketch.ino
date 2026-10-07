#include <Servo.h>

// Pin Definitions
const int trigPin = 7;
const int echoPin = 6;

const int servoPin = 5;

const int greenLedPin = 2;
const int redLedPin = 3;

// Servo
Servo steeringServo;

// Moving Average
const int NUM_READINGS = 5;

float readings[NUM_READINGS];
int readingIndex = 0;

float filteredDistance = 0;

// Zone Tracking
int currentZone = 0;
int previousZone = 0;

// Blinking

unsigned long previousBlinkTime = 0;
const unsigned long blinkInterval = 300;

bool greenLedState = false;

// Read Distance from HC-SR04

float readDistance() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  // If no echo is received
  if (duration == 0) {
    return 400;
  }

  float distance = duration * 0.0343 / 2;

  return distance;
}


// Moving Average Filter

float getMovingAverage(float newReading) {

  readings[readingIndex] = newReading;

  readingIndex++;

  if (readingIndex >= NUM_READINGS) {
    readingIndex = 0;
  }

  float sum = 0;

  for (int i = 0; i < NUM_READINGS; i++) {
    sum += readings[i];
  }

  return sum / NUM_READINGS;
}


// Setup

void setup() {

  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  steeringServo.attach(servoPin);

  // Start servo in safe position
  steeringServo.write(90);

  // Start LEDs OFF
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);

  // Initialize moving-average buffer
  float firstReading = readDistance();

  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = firstReading;
  }

  filteredDistance = firstReading;

  Serial.println("MRT Autonomous System Started");
}


// الMain Loop

void loop() {

  // Read sensor
  float distance = readDistance();

  // Apply Moving Average Filter
  filteredDistance = getMovingAverage(distance);

  // Print filtered distance
  Serial.print("Distance: ");
  Serial.print(filteredDistance);
  Serial.println(" cm");


  // Determine Zone
  

  if (filteredDistance > 50) {

    currentZone = 1;

    // Zone 1: Safe Driving
    steeringServo.write(90);

    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);

    Serial.println("ZONE 1 - SAFE DRIVING");
  }

  else if (filteredDistance > 20 && filteredDistance <= 50) {

    currentZone = 2;

    // Zone 2: Evasive Maneuver
    steeringServo.write(45);

    digitalWrite(redLedPin, LOW);

    // Green LED blinking
    if (millis() - previousBlinkTime >= blinkInterval) {

      previousBlinkTime = millis();

      greenLedState = !greenLedState;

      digitalWrite(greenLedPin, greenLedState);
    }

    Serial.println("ZONE 2 - EVASIVE MANEUVER");
  }

  else {

    currentZone = 3;

    // Zone 3: Emergency Brake
    steeringServo.write(135);

    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);

    Serial.println("ZONE 3 - EMERGENCY BRAKE");

    // Print required emergency message
    if (previousZone != 3) {
      Serial.println("EMERGENCY_BRAKE_ACTUATED");
    }
  }


  // Remember previous zone
  previousZone = currentZone;

  delay(100);
}
