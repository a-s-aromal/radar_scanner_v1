const uint8_t PIN_SERVO = 9, PIN_TRIG = 10, PIN_ECHO = 11;
const int ANGLE_MIN = 15, ANGLE_MAX = 165, ANGLE_STEP = 1, SETTLE_MS = 20;
const unsigned long ECHO_TIMEOUT_US = 25000UL;

bool scanning = true;
int angle = ANGLE_MIN;
int dir = 1;

void servoWrite(int a) { OCR1A = (544 + a * 1856L / 180) * 2; }
void servoOn(bool on)  { if (on) TCCR1A |= _BV(COM1A1); else TCCR1A &= ~_BV(COM1A1); }

long ping() {
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long us = pulseIn(PIN_ECHO, HIGH, ECHO_TIMEOUT_US);
  return us ? (long)((us * 343UL + 1000UL) / 2000UL) : -1;
}

long measure() {
  long a = ping(); delay(8);
  long b = ping(); delay(8);
  long c = ping();
  long lo = min(a, b), hi = max(a, b);
  return max(lo, min(hi, c));
}

void report(long d) {
  Serial.print(angle);
  Serial.print(',');
  if (d < 0) { Serial.println(-1); return; }
  Serial.print(d / 10);
  Serial.print('.');
  Serial.println(d % 10);
}

void setScanning(bool on) {
  if (on == scanning) return;
  scanning = on;
  servoOn(on);                     
  if (on) delay(300);              
  Serial.println(on ? F("#run") : F("#idle"));
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_SERVO, OUTPUT);

  TCCR1A = _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11);
  ICR1 = 39999;
  servoWrite(angle);
  servoOn(true);

  delay(600);
  Serial.println(F("#run"));
  report(measure());              
}

void loop() {
  while (Serial.available()) {
    switch (Serial.read() | 0x20) { 
      case 'g': setScanning(true);       break;
      case 's': setScanning(false);      break;
      case 't': setScanning(!scanning);  break;
    }
  }

  if (scanning) {
    angle = constrain(angle + dir * ANGLE_STEP, ANGLE_MIN, ANGLE_MAX);
    if (angle == ANGLE_MIN || angle == ANGLE_MAX) dir = -dir;
    servoWrite(angle);
  }

  delay(SETTLE_MS);
  report(measure());
}
