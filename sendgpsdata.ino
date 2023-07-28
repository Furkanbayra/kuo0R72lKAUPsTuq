#include <SoftwareSerial.h>
#include <avr/interrupt.h>

const int BUFFER_SIZE = 128;
char buffer[BUFFER_SIZE];
int bufferIndex = 0;
SoftwareSerial NMEA(2, 3);

volatile bool minuteElapsed = false;
volatile int seconds = 0;
String rmcData;
String ggaData;
String gllData;
String vtgData;
String finally;
String finallyRaw;
String RAWrmcData;
String RAWggaData;
String RAWgllData;
String RAWvtgData;
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
    printfinalString();
    minuteElapsed = false;
  }
  readData();
  createFinalString();
}

void readData() {
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
        String SOG = fields[7].c_str();
        String COG = fields[8].c_str();
        rmcData = latitude + " " + NS + " " + longitude + " " + EW + " " + SOG + " " + COG;
        RAWrmcData = "";
        for (int i = 0; i <= 11; i++) {
          RAWrmcData += fields[i].c_str();
          if (i < 11) {
            RAWrmcData += ",";
          }
        }
      } else if (fields[0] == "$GPGGA") {
        String latitude = fields[2].c_str();
        String NS = fields[3].c_str();
        String longitude = fields[4].c_str();
        String EW = fields[5].c_str();
        ggaData = latitude + " " + NS + " " + longitude + " " + EW;
        RAWggaData = "";
        for (int i = 0; i <= 11; i++) {
          RAWggaData += fields[i].c_str();
          if (i < 11) {
            RAWggaData += ",";
          }
        }
      } else if (fields[0] == "$GPGLL") {
        String latitude1 = fields[3].c_str();
        String NS1 = fields[2].c_str();
        String longitude1 = fields[5].c_str();
        String EW1 = fields[4].c_str();
        gllData = latitude1 + " " + NS1 + " " + longitude1 + " " + EW1;
        RAWgllData = "";
        for (int i = 0; i <= 6; i++) {
          RAWgllData += fields[i].c_str();
          if (i < 6) {
            RAWgllData += ",";
          }
        }
      } else if (fields[0] == "$GPVTG") {
        String course = fields[1].c_str();
        String SOG = fields[5].c_str();
        vtgData = course + " " + SOG;
        RAWvtgData = "";
        for (int i = 0; i <= 8; i++) {
          RAWvtgData += fields[i].c_str();
          if (i < 8) {
            RAWvtgData += ",";
          }
        }
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

void createFinalString() {
  if (rmcData.length() > 0) {
    finally = rmcData;
    finallyRaw = RAWrmcData;
  } else if (ggaData.length() > 0) {
    finally = ggaData;
    finallyRaw = RAWggaData;
  } else if (gllData.length() > 0) {
    finally = gllData;
    finallyRaw = RAWgllData;
  } else if (vtgData.length() > 0) {
    finally = vtgData;
    finallyRaw = RAWvtgData;
  } else {
    finally = "NO CONNECTION";
  }
}
void printfinalString() {
  if (finally.length() > 0) {
    Serial.println(finally);    
    finally = "";
  }
}
void printfinalRawString() {
  if (finallyRaw.length() > 0) {
    Serial.println(finallyRaw);
    finallyRaw = "";
  }
}

ISR(TIMER1_COMPA_vect) {
  seconds++;
  if (seconds >= 10) {  // time adjust in second
    minuteElapsed = true;
    seconds = 0;
  }
}
