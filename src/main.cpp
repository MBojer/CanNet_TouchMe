
/* Colors:

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

#include <Arduino.h>

#define Max_Number_Of_Pages 15 // Used all over


// -------------------------------------------- SD Card --------------------------------------------
#include <SD.h>


// -------------------------------------------- Files --------------------------------------------
#define Setting_File_Path "/TouchMe/Settings.txt"
String Settings_File_Content;

#define Top_Bar_File_Path "/TouchMe/Top_Bar.txt"
String Top_Bar_File_Content;

#define Page_File_Path "/TouchMe/" // Adding the page names later in a loop
String Page_File_Content[Max_Number_Of_Pages];


// -------------------------------------------- LCD --------------------------------------------
#include <UTFT.h>
UTFT myGLCD(CTE50, 38, 39, 40, 41);
extern uint8_t GroteskBold16x32[];


// -------------------------------------------- Touch --------------------------------------------
#include <URTouch.h>
URTouch myTouch( 6, 5, 4, 3, 2);


// -------------------------------------------- MISC --------------------------------------------
int Current_Page;


// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h>
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 2000
// --------------------- REMOVE ME - End ---------------------

unsigned int hexToDec(String hexString) {

  unsigned int decValue = 0;
  int nextInt;

  for (unsigned int i = 0; i < hexString.length(); i++) {

    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);

    decValue = (decValue * 16) + nextInt;
  }

  return decValue;
}

String decToHex(byte decValue, byte desiredStringLength) {
  // Converting from Decimal to Hex:

  // NOTE: This function can handle a positive decimal value from 0 - 255, and it will pad it
  //       with 0's (on the left) if it is less than the desired string length.
  //       For larger/longer values, change "byte" to "unsigned int" or "long" for the decValue parameter.


  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;

  return hexString;
}

void Error_Mode(byte Error_Type, String Error_Text) {

  // ADD ME - Support for write log file to sd if avalible
  // ADD ME - Support for write error on screen

  if (Error_Type == 1) {
    if (Serial) {
      Serial.print("FATAL ERROR: ");
      Serial.println(Error_Text);
      Serial.println("HALTING");
    }

    myGLCD.fillScr(0xF800);
    myGLCD.print("FATAL ERROR: " + Error_Text, 20, 20);

    while (1) {
      delay(1000);
    }

  } // END MARKER - if (Error_Type == 1) {

  if (Error_Type == 2) {
      Serial.print("ERROR: ");
      Serial.println(Error_Text);
  } // END MARKER - if (Error_Type == 2) {

} // END MARKER - Error_Mode


String Read_Conf_File(String File_Path, bool Error_Message) {

    File Temp_File;
    String File_Content;

    // Checks if the file exists
    if (SD.exists(File_Path) == false) {
      if (Error_Message == true) Error_Mode(2, "File missing: " + File_Path);
      return "";
    }

    Temp_File = SD.open(File_Path); // Open the file

    // Reads the file and puts it into a string
    while (Temp_File.available()) {
      char Letter = Temp_File.read();
      File_Content += Letter;
    }

    Temp_File.close(); // Close the file

    File_Content = File_Content.substring(0, File_Content.indexOf("Comments:"));

    while (true) {
      if (File_Content.indexOf("\r\n\r\n") == -1) break;
      File_Content.replace("\r\n\r\n", "\r\n");
    }

    return File_Content;
}

String Read_Conf_File(String File_Path) {
  return Read_Conf_File(File_Path, true);
} // END MARKER - Read_Conf_File

String Find_Setting(String &Settings_File_Content, String Settings_Name) {

  String Search_String = "\r\n" + Settings_Name + " = ";

  if (Settings_File_Content.indexOf(Search_String) == -1) {
    return "";
  }



  int Settings_Position = Settings_File_Content.indexOf(Search_String) + Search_String.length();

  return Settings_File_Content.substring(
                                          Settings_Position,
                                          Settings_File_Content.indexOf("\r\n", Settings_Position)
                                        );

} // END MARKER - Find_Setting

word Find_Setting_Color(String &Settings_File_Content, String Settings_Name) {

  String Search_String = "\r\n" + Settings_Name + " = ";

  if (Settings_File_Content.indexOf(Search_String) == -1) {
    return -1;
  }

  int Settings_Position = Settings_File_Content.indexOf(Search_String) + Search_String.length();

  String Temp_String = Settings_File_Content.substring(
                                          Settings_Position,
                                          Settings_File_Content.indexOf("\r\n", Settings_Position)
                                        );

  Temp_String.replace("0x", "");

  // int testint = hexToDec(teststring);

  return word(hexToDec(Temp_String));

} // END MARKER - Find_Setting




void Top_Bar() {

  // String teststring = "001F";
  //
  // // long long number = strtoll( &teststring, NULL, 16);
  //
  // int testint = hexToDec(teststring);
  // word testword = word(testint);

  myGLCD.clrScr();


  if (Top_Bar_File_Content != "") {

    myGLCD.setColor(Find_Setting_Color(Settings_File_Content, "Button Color"));

    myGLCD.fillRoundRect(0, 0, myGLCD.getDisplayXSize() - 1, Find_Setting(Top_Bar_File_Content, "Size").toInt());

    


  }






  // myGLCD.fillScr(testword);
  // myGLCD.fillScr(word(Find_Setting(Settings_File_Content, "Color")));





} // END MARKER - Top_Bar


void setup() {

  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);

  if (!Serial) { // REMOVE ME
    for (int x = 0; x < 10; x++) {
      delay(100);
    }
  }


  // -------------------------------------------- SD Card --------------------------------------------
  if (!SD.begin(53)) {
    Error_Mode(1, "Initializing SD card");
  }


  // -------------------------------------------- LCD --------------------------------------------
  myGLCD.InitLCD();
  myGLCD.setFont(GroteskBold16x32);


  // -------------------------------------------- Touch --------------------------------------------
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);


  // -------------------------------------------- Boot Message --------------------------------------------
  myGLCD.fillScr(0xF800);
  myGLCD.setBackColor(0x0000);
  myGLCD.print(String("Booting"), CENTER, myGLCD.getDisplayYSize() / 2 - 5);
  delay(500);


  // -------------------------------------------- Files --------------------------------------------

  Serial.print("Before: "); // REMOVE ME
  Serial.println(freeMemory()); // REMOVE ME

  Settings_File_Content = Read_Conf_File(Setting_File_Path);


  Serial.print("Settings File: "); // REMOVE ME
  Serial.println(freeMemory()); // REMOVE ME

  Top_Bar_File_Content = Read_Conf_File(Top_Bar_File_Path);

  Serial.print("Top Bar: "); // REMOVE ME
  Serial.println(freeMemory()); // REMOVE ME

  for (int x = 1; x < Max_Number_Of_Pages; x++) {

    Page_File_Content[x] = Read_Conf_File(String(Page_File_Path) + "Page " + x + ".txt", false);

    if (Page_File_Content[x] == "") break;

    Serial.println(Page_File_Content[x]); // REMOVE ME
  }

  Serial.print("Pages: "); // REMOVE ME
  Serial.println(freeMemory()); // REMOVE ME


  // -------------------------------------------- Boot Message - End --------------------------------------------
  myGLCD.print(String("Boot Done"), CENTER, myGLCD.getDisplayYSize() / 2 - 5);
  Serial.println("Boot Done");

  delay(500);

  Top_Bar();

} // END MARKER - setup

void loop() {


} // END MARKER - loop
