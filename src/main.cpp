#include <Arduino.h>

#include <MemoryFree.h>

#include <UTFT.h>
UTFT myGLCD(CTE50, 38, 39, 40, 41);
extern uint8_t GroteskBold16x32[];

#include <URTouch.h>
URTouch  myTouch( 6, 5, 4, 3, 2);

#include <UTFT_Buttons.h>
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

#include <UTFT_Geometry.h>
UTFT_Geometry geo(&myGLCD);

// #include <UTFT_tinyFAT.h>
// UTFT_tinyFAT myFiles(&myGLCD);


/* --- Color Codes ---
#define VGA_BLACK		0x0000
#define VGA_WHITE		0xFFFF
#define VGA_RED			0xF800
#define VGA_GREEN		0x0400
#define VGA_BLUE		0x001F
#define VGA_SILVER		0xC618
#define VGA_GRAY		0x8410
#define VGA_MAROON		0x8000
#define VGA_YELLOW		0xFFE0
#define VGA_OLIVE		0x8400
#define VGA_LIME		0x07E0
#define VGA_AQUA		0x07FF
#define VGA_TEAL		0x0410
#define VGA_NAVY		0x0010
#define VGA_FUCHSIA		0xF81F
#define VGA_PURPLE		0x8010
#define VGA_TRANSPARENT	0xFFFFFFFF
*/


// --------------------- REMOVE ME ---------------------------
// #include <MemoryFree.h>
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 1000

// --------------------- REMOVE ME - End ---------------------


void setup() {

  delay(500);

  Serial.begin(115200);

  while (!Serial) {
    delay(100);
  }

  Serial.println("Booting");

  myGLCD.InitLCD();
  myGLCD.setFont(GroteskBold16x32);

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

  myGLCD.fillScr(0xF800);
  myGLCD.setBackColor(0x0000);
  myGLCD.print(String("Booting"), CENTER, 220);
  delay(500);

  myGLCD.fillScr(0xC618);



} // END MARKER - setup



void loop() {

  if (freeMemory_Delay_Until < millis()) { // REMOVE ME
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());

    myGLCD.print(String(freeMemory()), CENTER, 220);

    freeMemory_Delay_Until = millis() + freeMemory_Delay_For;
  }

} // END MARKER - loop
