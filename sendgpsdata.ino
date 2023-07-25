#include <SoftwareSerial.h>
#include <avr/interrupt.h>

const int BUFFER_SIZE = 128;
char buffer[BUFFER_SIZE];
int bufferIndex = 0;
SoftwareSerial NMEA(2, 3);

volatile bool minuteElapsed = false;
volatile int seconds = 0;
String rmcData;

void setup() {
  while (!Serial)
    ;
  Serial.begin(9600);
  NMEA.begin(9600);

  // Timer1 konfigürasyonu
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void loop() {
  if (minuteElapsed) {
    minuteElapsed = false;

    if (rmcData.length() > 0) {
      Serial.println(rmcData);
      rmcData = "";
    }
  }

  if (NMEA.available()) {
    char receivedChar = NMEA.read();

    if (receivedChar == '\n') {
      buffer[bufferIndex] = '\0';
      String data = String(buffer);
      bufferIndex = 0;
      String fields[20];
      int fieldCount = 0;

      for (int i = 0; i < data.length(); i++) {
        char c = data.charAt(i);

        if (c == ',') {
          fieldCount++;
        } else {
          fields[fieldCount] += c;
        }
      }

      if (fields[0] == "$GPRMC") {

        String latitude = fields[3].c_str();
        String NS = fields[4].c_str();
        String longitude = fields[5].c_str();
        String EW = fields[6].c_str();
        rmcData = latitude + " " + NS + " " + longitude + " " + EW;
      }
    } else {
      buffer[bufferIndex] = receivedChar;
      bufferIndex++;
      if (bufferIndex >= BUFFER_SIZE) {
        bufferIndex = BUFFER_SIZE - 1;
      }
    }
  }
}
ISR(TIMER1_COMPA_vect) {
  seconds++;
  if (seconds >= 60) {  // time adjust in second
    minuteElapsed = true;
    seconds = 0;
  }
}
