#include <Arduino.h>

bool DEBUG = false;

unsigned LOOP_TIME = 33 * 1e3; // 30fps ~= 33 * 1000 us

int POT_PIN = A0;   // select the input pin for the potentiometer
int RPM_PIN = 2;    // select the pin for the RPM signal
int PHASE_PIN = 4;  // select the pin for phase signal

int CYL_QTD = 6;    // Engine cylinders quantity
int RPM_MAX = 6000;
int RPM_MIN = 200;

int rpm_state = LOW;
int phase_state = LOW;

unsigned long current_time = 0;
unsigned long previous_serial_time = 0;
unsigned long previous_rpm_time = 0;
unsigned long previous_phase_time = 0;

void setup()
{
  if (DEBUG)
    Serial.begin(9600); // initialize serial communication at 9600 bits per second

  pinMode(RPM_PIN, OUTPUT); // declare the ledPin as an OUTPUT
  pinMode(PHASE_PIN, OUTPUT);
}

void loop()
{
  unsigned long temp = micros();
  
  if (temp < current_time) // overflow
  {
    previous_serial_time = 0;
    previous_rpm_time = 0;
    previous_phase_time = 0;
  }
  current_time = temp;

  unsigned val = analogRead(POT_PIN); // read the value from the sensor

  unsigned rpm = map(val, 0, 20480, RPM_MIN, RPM_MAX);

  unsigned long rpm_wait = 1e6 / (2 * (CYL_QTD / 2) * rpm / 60); // time to wait for between clock`s edges in microseconds
                                                                 // 2 (high/low) * 3 (distributor pulses for one crankshaft revolution) * rpm / 60 (seconds)
  unsigned long phase_wait = 1e6 / (2 * rpm / 60);

  // Serial output
  if (DEBUG && (current_time - previous_serial_time >= LOOP_TIME) )
  {
    previous_serial_time = current_time;

    Serial.print("Potentiometer: ");
    Serial.print(val);
    Serial.print("\tRPM: ");
    Serial.print(rpm);
    Serial.print("\trpm_wait: ");
    Serial.print(rpm_wait);
    Serial.print("\tphase_wait: ");
    Serial.print(phase_wait);
    Serial.println();
  }

  // RPM output
  if (current_time - previous_rpm_time >= rpm_wait)
  {
    previous_rpm_time = current_time;

    rpm_state = !rpm_state;
    digitalWrite(RPM_PIN, rpm_state);
  }

  // Crankshaft phase output
  if (current_time - previous_phase_time >= phase_wait)
  {
    previous_phase_time = current_time;

    phase_state = !phase_state;
    digitalWrite(PHASE_PIN, phase_state);
  }
}
