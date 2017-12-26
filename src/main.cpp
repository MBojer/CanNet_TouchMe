#include <Arduino.h>

#include <SD.h>
File Test_txt;


void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // wait for serial line to be ready
  }

    Serial.print("Initializing SD card...");
    if (!SD.begin(53)) {
      Serial.println("failed!");
      Serial.println("Halting");
      while (1) delay(1000);
    }
    Serial.println("done");

    Test_txt = SD.open("test.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (Test_txt) {
      Serial.print("Writing to test.txt...");
      Test_txt.println("testing 1, 2, 3.");
      // close the file:
      Test_txt.close();
      Serial.println("done.");
    }

    else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }


}

void loop() {
  Serial.println("loop");




  delay(5000);
}
