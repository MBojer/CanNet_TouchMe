
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

#define Max_For_Loop_Runs 101


// -------------------------------------------- SD Card --------------------------------------------
#include <SD.h>


// -------------------------------------------- Files --------------------------------------------
String File_Content;

#define Setting_File_Path "/TouchMe/Settings.txt"

#define Top_Bar_File_Path "/TouchMe/Top_Bar.txt"
bool Top_Bar_Present = false;

#define Page_File_Path "/TouchMe/" // Adding the page names later in a loop
String Page_File_Content;


// -------------------------------------------- LCD --------------------------------------------
#include <UTFT.h>
UTFT lcd(CTE50, 38, 39, 40, 41);
extern uint8_t GroteskBold16x32[];


// -------------------------------------------- Touch --------------------------------------------
#include <URTouch.h>
URTouch touch( 6, 5, 4, 3, 2);

int Touch_Input_X;
int Touch_Input_Y;

// -------------------------------------------- Touch - Top Bar --------------------------------------------
int Top_Bar_Button_Spaceing;
byte Top_Bar_Button_Pressed;


// -------------------------------------------- Touch - Millis --------------------------------------------
#define Top_Bar_Ignore_Input_For 1000
unsigned long Top_Bar_Ignore_Input_Untill;


// -------------------------------------------- MISC --------------------------------------------
int Current_Page = 1;
int Last_Page = 1;


// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h>
unsigned long freeMemory_Last;
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 2000
// --------------------- REMOVE ME - End ---------------------


// -------------------------------------------- CanNet - UTFT --------------------------------------------
word Text_Color;
word Button_Color;

byte Edge_Size;
word Edge_Color;

bool Button_Center_Text = true;

int Top_Bar_Size;
int Top_Bar_Button_Size;

int Matrix_Spacing;

int Button_Size_X;
int Button_Size_Y;


// --------------------- Draw_Button ---------------------
int Draw_Size_X;
int Draw_Size_Y;

byte Draw_Edge_Size;


// -------------------------------------------- CanNet - URTouch --------------------------------------------
bool Flip_Touch = false;

// -------------------------------------------- CanNet - MISC --------------------------------------------
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

    lcd.fillScr(0xF800);
    lcd.print("FATAL ERROR: " + Error_Text, 20, 20);

    while (1) {
      delay(1000);
    }

  } // if (Error_Type == 1) {

  if (Error_Type == 2) {
      Serial.print("ERROR: ");
      Serial.println(Error_Text);
  } // if (Error_Type == 2) {

} // Error_Mode

// -------------------------------------------- CanNet - File Management --------------------------------------------
String Read_Conf_File(String File_Path, bool Error_Message) {

    File Temp_File;
    File_Content = "";

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

String Read_Conf_File(String File_Path) { // Referance only
  return Read_Conf_File(File_Path, true);
} // Read_Conf_File


String Find_Setting(String &File_Content, String Setting_Name) {

  String Search_String = "\r\n" + Setting_Name + " = ";

  if (File_Content.indexOf(Search_String) == -1) {
    return "";
  }


  int Settings_Position = File_Content.indexOf(Search_String) + Search_String.length();

  return File_Content.substring(
                                          Settings_Position,
                                          File_Content.indexOf("\r\n", Settings_Position)
                                        );

} // Find_Setting

bool Find_Setting_Bool(String &File_Content, String Setting_Name) {

  Setting_Name = Find_Setting(File_Content, Setting_Name);

  return Setting_Name;

} // Find_Setting

int Find_Setting_Int(String &File_Content, String Setting_Name) {

  Setting_Name = Find_Setting(File_Content, Setting_Name);

  return Setting_Name.toInt();

} // Find_Setting

word Find_Setting_Word(String &File_Content, String Setting_Name) {

  String Search_String = "\r\n" + Setting_Name + " = ";

  if (File_Content.indexOf(Search_String) == -1) {
    return -1;
  }

  int Settings_Position = File_Content.indexOf(Search_String) + Search_String.length();

  String Temp_String = File_Content.substring(
                                          Settings_Position,
                                          File_Content.indexOf("\r\n", Settings_Position)
                                        );

  Temp_String.replace("0x", "");

  // int testint = hexToDec(teststring);

  return word(hexToDec(Temp_String));

} // Find_Setting





String Find_Sub_Setting(String Setting_Content, String Setting_Name) {

  String Search_String = Setting_Name + ":";

  if (Setting_Content.indexOf(Search_String) == -1) {
    return "";
  }

  int Settings_Position = Setting_Content.indexOf(Search_String) + Search_String.length();

  return Setting_Content.substring(
                                          Settings_Position,
                                          Setting_Content.indexOf(":" + Setting_Name, Settings_Position)
                                        );

} // Find_Setting


int Find_Sub_Setting_Int(String Setting_Content, String Setting_Name) {

  String Sub_Search_String = Setting_Name + ":";

  if (Setting_Content.indexOf(Sub_Search_String) == -1) {
    return -1;
  }

  int Settings_Position = Setting_Content.indexOf(Sub_Search_String) + Sub_Search_String.length();

  return Setting_Content.substring(
                                          Settings_Position,
                                          Setting_Content.indexOf(" ", Settings_Position)
                                        ).toInt();

} // Find_Setting_Int







int Center_Text_Calc_X(String Text, int Button_Size) {

	return Button_Size / 2 - Text.length() * (lcd.getFontXsize() / 2);

} // Center_Text_Calc_X

int Center_Text_Calc_X(String Text) { // Referance only
	return Center_Text_Calc_X(Text, Draw_Size_X);
} // Center_Text_Calc_X - Reff


int Center_Text_Calc_Y(String Text, int Button_Size) {
	return Button_Size / 2 - (lcd.getFontYsize() / 2);
} // Center_Text_Calc_X

int Center_Text_Calc_Y(String Text) { // Referance only
	return Center_Text_Calc_Y(Text, Draw_Size_Y);
} // Center_Text_Calc_Y - Reff



// ---------------------------------- Draw ----------------------------------
void Draw_Button(String Button_Text, int Start_X, int Start_Y) {

	if (true) { // Edge
		lcd.setColor(Edge_Color);

		if (Draw_Edge_Size == 0);

		else if (Draw_Edge_Size == 1) {
			lcd.drawRoundRect(Start_X, Start_Y, Start_X + Draw_Size_X, Start_Y + Draw_Size_Y);
		}

		else {
			lcd.fillRoundRect (Start_X, Start_Y, Start_X + Draw_Size_X, Start_Y + Draw_Size_Y);
		}
  } // Edge

	if (true) { // Button
		lcd.setColor(Button_Color);
		lcd.fillRoundRect (
										Start_X + Draw_Edge_Size,
										Start_Y + Draw_Edge_Size,
										Start_X + Draw_Size_X - Draw_Edge_Size,
										Start_Y + Draw_Size_Y - Draw_Edge_Size
									);
  } // Button

	if (Button_Text != "") { // Text
		lcd.setColor(Text_Color);
		lcd.setBackColor(Button_Color);

			if (Button_Center_Text == true) {
				lcd.print(
					Button_Text,
					Start_X + Center_Text_Calc_X(Button_Text),
					Start_Y + Center_Text_Calc_Y(Button_Text)
				);
			}

			else {
				lcd.print( // CHANGE ME TO SOMETHING USEFUL
					Button_Text,
					Start_X + 15,
					Start_Y + 15
				);
			}
  } // if (Button_Text != "")

} // Draw_Button

void Draw_Button_Matrix(String Button_Text, byte Button_Number_X, byte Button_Number_Y) {

  Draw_Button(
              Button_Text,
              Matrix_Spacing * Button_Number_X + Draw_Size_X * (Button_Number_X - 1),
        	    Top_Bar_Size + Matrix_Spacing * Button_Number_Y + Draw_Size_Y * (Button_Number_Y - 1)
  );

} // Draw_Button_Matrix

void Draw_Top_Bar(String Top_Bar_Text) {

	Draw_Size_X = lcd.getDisplayXSize() - 1;
	Draw_Size_Y = Top_Bar_Size;

	Draw_Edge_Size = 0;

  lcd.setColor(Button_Color);
	lcd.setBackColor(Button_Color);

	Draw_Button(Top_Bar_Text, 0, 0);

  Draw_Edge_Size = Edge_Size;


	//-------------------------------------------------- Drws Page up / Page down --------------------------------------------------
  Draw_Size_X = Top_Bar_Button_Size;
	Draw_Edge_Size = Edge_Size;

  if (Current_Page < 9) {
		Draw_Size_X = 5 * lcd.getFontXsize();
	}

	else {
		Draw_Size_X = 6 * lcd.getFontXsize();
	}


	if (Current_Page != 1) {
		String Page_Number_Text = "<< " + String(Current_Page - 1);

		Draw_Button(Page_Number_Text, 0, 0);
	}

	if (Current_Page != Last_Page) {
		String Page_Number_Text = String(Current_Page + 1) + " >>";

		Draw_Button(Page_Number_Text, lcd.getDisplayXSize() - 1 - Draw_Size_X, 0);
	}


} // Draw_Top_Bar

void Draw_Page(String Page_Content) {

  if (Page_Content == "") return; // Nothing in the file so nothing to do

  Page_Content.replace("\r\nName = " + Find_Setting(Page_Content, "Name"), "");

  for (int x = 0; x < Max_For_Loop_Runs; x++) {

    // ---------------------------------- Matrix Button ----------------------------------
    if (Page_Content.indexOf("\r\nMatrix Button = ") != -1) {

      Draw_Size_X = Button_Size_X;
      Draw_Size_Y = Button_Size_Y;

      String Button_Settings = Find_Setting(Page_Content, "Matrix Button");

      Draw_Button_Matrix(
                          Find_Sub_Setting(Button_Settings, "N"),
                          Find_Sub_Setting_Int(Button_Settings, "X"),
                          Find_Sub_Setting_Int(Button_Settings, "Y")
      );

      Page_Content.replace("\r\nMatrix Button = " + Button_Settings, ""); // Removed the entry that was just drawn
    } // Matrix Button


    // ---------------------------------- Matrix Slider ----------------------------------
    if (Page_Content.indexOf("\r\nMatrix Slider = ") != -1) {

      Draw_Size_Y = Button_Size_Y;

      String Slider_Settings = Find_Setting(Page_Content, "Matrix Slider");

      if (Find_Sub_Setting_Int(Slider_Settings, "X") == 0) {
        Draw_Size_X = lcd.getDisplayXSize() - Matrix_Spacing * 2;

        Draw_Button_Matrix("", 1, Find_Sub_Setting_Int(Slider_Settings, "Y"));
      } // X == 0

      else { // X != 0
        Draw_Size_X = Button_Size_X;

        Draw_Button_Matrix(
                            "",
                            Find_Sub_Setting_Int(Slider_Settings, "X"),
                            Find_Sub_Setting_Int(Slider_Settings, "Y")
        );
      } // X != 0

      Page_Content.replace("\r\nMatrix Slider = " + Slider_Settings, ""); // Removed the entry that was just drawn
    } // Matrix Slider


    // ---------------------------------- End of loop cheks ----------------------------------
    else if (Page_Content == "Settings:\r\n") break; // No entries left in file

    else if (x == Max_For_Loop_Runs - 1) {
      Page_Content.replace("Settings:\r\n", "");
      Error_Mode(2, "Draw_Page, Settings left over:\r\n" + Page_Content);
      break; // Assuming error in page file
    }

  }

} // Draw_Page



// -------------------------------------------- CanNet - URTouch --------------------------------------------




// -------------------------------------------- Top_Bar --------------------------------------------
void Top_Bar() {

  Page_File_Content = Read_Conf_File(String(Page_File_Path) + "Page_" + Current_Page + ".txt", false);

  lcd.clrScr();

  if (Top_Bar_Present == true) {
    Draw_Top_Bar(Find_Setting(Page_File_Content, "Name"));
  }

  Draw_Page(Page_File_Content);

} // Top_Bar


void Top_Bar_Touch() {

  if (
      !touch.dataAvailable() || // Screen not pressed
      Touch_Input_Y == -1 || // Input off screen
      Touch_Input_Y > Top_Bar_Size || // Input not matching top bar
      Top_Bar_Ignore_Input_Untill > millis() // Pressed to soon ignoreing input
  ) return; // No delaied output for Top_Bar_Touch

  Serial.print("Touch_Input_Y: "); // rm
  Serial.println(Touch_Input_Y); // rm
  delay(100); // rm

   if (Touch_Input_Y > 0 && Touch_Input_Y < Top_Bar_Size) { // CHANGE ME - for the line below
  // else if (Touch_Input_Y > 0 && Touch_Input_Y < Top_Bar_Size) { // Y - Input matching top bar
    Serial.println("MARKER 2"); // rm
    delay(750); // rm

    if (Touch_Input_X > 0 && Touch_Input_X < Top_Bar_Button_Size) { // X - Page Down
      Top_Bar_Ignore_Input_Untill = millis() + Top_Bar_Ignore_Input_For;

      if (Current_Page != 1) { // Ingnore input if you are at page 1
        Current_Page = Current_Page - 1;
        Top_Bar();
      } // if (Current_Page == 1)
    } // if (Touch_Input_X > 0 && Touch_Input_X < Top_Bar_Button_Size)

    else if (Touch_Input_X > Top_Bar_Button_Size + Top_Bar_Button_Spaceing &&
             Touch_Input_X < Top_Bar_Button_Size + Top_Bar_Button_Spaceing + Top_Bar_Button_Size
    )
    { // X - Page Up
      Top_Bar_Ignore_Input_Untill = millis() + Top_Bar_Ignore_Input_For;

      if (Current_Page != Last_Page) { // Ingnore input if you are at the last page
        Current_Page = Current_Page + 1;
        Top_Bar();
      } // if (Current_Page != Last_Page)

  } // else if (Touch_Input_Y > 0 && Touch_Input_Y < _Top_Bar_Size)


  // if (Top_Bar_Ignore_Input_Untill > millis()) { // Pressed to soon ignoreing input
  // if (Input_Y > Top_Bar_Size) { // Input not matching top bar
  // }
  //
  // else if (Top_Bar_Ignore_Input_Untill > millis()) { // Pressed to soon ignoreing input
  //   Top_Bar_Button_Pressed = 0;
  // }
  //
  // else if (Input_Y > 0 && Input_Y < Top_Bar_Size) { // Y - Input matching top bar
  //
  //
  //   if (Input_X > 0 && Input_X < Top_Bar_Button_Size) { // X - Page Down
  //     Top_Bar_Ignore_Input_Untill = millis() + Top_Bar_Ignore_Input_For;
  //     Top_Bar_Button_Pressed = 1;
  //   }
  //
  //   else if (
  //             Input_X > Top_Bar_Button_Size + Top_Bar_Button_Spaceing &&
  //             Input_X < Top_Bar_Button_Size + Top_Bar_Button_Spaceing + Top_Bar_Button_Size
  //           ) { // X - Page Up
  //
  //     Top_Bar_Ignore_Input_Untill = millis() + Top_Bar_Ignore_Input_For;
  //     Top_Bar_Button_Pressed = 2;
  //   }
  //
  // } // else if (Input_Y > 0 && Input_Y < _Top_Bar_Size)
  //
  // return Top_Bar_Button_Pressed;



  } // else if (Touch_Input_Y > 0 && Touch_Input_Y < Top_Bar_Size)

} // Top_Bar_Touch


// -------------------------------------------- Touch Check --------------------------------------------
void Touch_Check() {

  touch.read();

  if (Flip_Touch == false) {
    Touch_Input_X = touch.getX();
    Touch_Input_Y = touch.getY();
  }

  else { // Flip_Touch = true
    Touch_Input_X = lcd.getDisplayXSize() - touch.getX();
    Touch_Input_Y = lcd.getDisplayYSize() - touch.getY();
  }


  Top_Bar_Touch();


  // if (myTouch.Stabilize_Input(Touch_Input_X, Touch_Input_Y) == 1)  return; // Touch input diviated to much returning


  // // --------------------------------------------- 1 - Main Page ---------------------------------------------
  // if (Current_Page == 1) {
  // // Main_Page_Touch();
  // }
  //
  // // --------------------------------------------- 2 - Relay Page ---------------------------------------------
  // else if (Current_Page == 2) { // CHANGE BELOW
  // }

} // Touch_Check


// -------------------------------------------- Setup --------------------------------------------
void setup() {

  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);

  if (!Serial) { // rm
    for (int x = 0; x < 10; x++) {
      delay(100);
    }
  }


  // -------------------------------------------- SD Card --------------------------------------------
  if (!SD.begin(53)) {
    Error_Mode(1, "Initializing SD card");
  }


  // -------------------------------------------- LCD --------------------------------------------
  lcd.InitLCD();
  lcd.setFont(GroteskBold16x32);


  // -------------------------------------------- Touch --------------------------------------------
  touch.InitTouch();
  touch.setPrecision(PREC_MEDIUM);


  // -------------------------------------------- Boot Message --------------------------------------------
  lcd.fillScr(0xF800);
  lcd.setBackColor(0x0000);
  lcd.print(String("Booting"), CENTER, lcd.getDisplayYSize() / 2 - 5);
  delay(500);


  // -------------------------------------------- Settings file import --------------------------------------------
  File_Content = Read_Conf_File(Setting_File_Path);


  if (File_Content.indexOf("\r\nText Color = ") != -1) {
    Text_Color = Find_Setting_Word(File_Content, "Text Color");
  }

  if (File_Content.indexOf("\r\nEdge Color = ") != -1) {
    Edge_Color = Find_Setting_Word(File_Content, "Edge Color");
  }

  if (File_Content.indexOf("\r\nButton Color = ") != -1) {
    Button_Color = Find_Setting_Word(File_Content, "Button Color");
  }

  if (File_Content.indexOf("\r\nEdge Size = ") != -1) {
    Edge_Size = Find_Setting_Int(File_Content, "Edge Size");
  }

  if (File_Content.indexOf("\r\nButton Center Text = ") != -1) {
    Button_Center_Text = Find_Setting_Bool(File_Content, "Button Center Text");
  }

  if (File_Content.indexOf("\r\nMatrix Spacing = ") != -1) {
    Matrix_Spacing = Find_Setting_Int(File_Content, "Matrix Spacing");
  }

  if (File_Content.indexOf("\r\nButton Size X = ") != -1) {
    Button_Size_X = Find_Setting_Int(File_Content, "Button Size X");
  }

  if (File_Content.indexOf("\r\nButton Size Y = ") != -1) {
    Button_Size_Y = Find_Setting_Int(File_Content, "Button Size Y");
  }

  if (File_Content.indexOf("\r\nFlip Touch = ") != -1) {
    Flip_Touch = Find_Setting_Bool(File_Content, "Flip Touch");
  }


  // -------------------------------------------- Top Bar file import --------------------------------------------
  File_Content = Read_Conf_File(Top_Bar_File_Path);

  if (File_Content != "") {
    Top_Bar_Present = true;
  }

  if (File_Content.indexOf("\r\nSize = ") != -1) {
    Top_Bar_Size = Find_Setting_Int(File_Content, "Size");
  }

  if (File_Content.indexOf("\r\nButton Size = ") != -1) {
    Top_Bar_Button_Size = Find_Setting_Int(File_Content, "Button Size");
  }

  File_Content = "";


  // -------------------------------------------- Page file import --------------------------------------------
  for (int x = 1; x < Max_For_Loop_Runs; x++) {

    if (SD.exists(String(Page_File_Path) + "Page_" + x + ".txt"));

    else if (x == Max_For_Loop_Runs - 1) {
      Error_Mode(2, "Page file import: for ran 100 loop");
    }

    else {
      Last_Page = x - 1;
      break;
    }


  }


  // -------------------------------------------- Boot Message - End --------------------------------------------
  lcd.print(String("Boot Done"), CENTER, lcd.getDisplayYSize() / 2 - 5);
  Serial.println("Boot Done");

  delay(500);

  Top_Bar();

} // setup

void loop() {

  // --------------------- REMOVE ME ---------------------------
  if (freeMemory_Delay_Until < millis()) { // rm

    unsigned long mesurement = freeMemory();

    if (freeMemory_Last != mesurement) {
      Serial.print("freeMemory()=");
      Serial.println(mesurement);
      freeMemory_Last = mesurement;
    }

    freeMemory_Delay_Until = millis() + freeMemory_Delay_For;
  }
  // --------------------- REMOVE ME - End ---------------------------



  Touch_Check();






} // loop




// // --------------------------------------------------------------------------------------------
// // ------------------------------------------ CanNet - UTFT.h ------------------------------------------
// // --------------------------------------------------------------------------------------------
//
// public:
//
// 	void Draw_Button(String Button_Text, int Start_X, int Start_Y);
// 	int Draw_Size_X;
// 	int Draw_Size_Y;
//
// 	word Button_Color;
// 	// word Button_Text_Color;
//
// 	word Button_Edge_Color;
// 	int Draw_Edge_Size;
//
//
// 	bool Button_Center_Text = true;
//
//
// 	int Center_Text_Calc_X(String Text, int Button_Size);
// 	int Center_Text_Calc_X(String Text);
//
// 	int Center_Text_Calc_Y(String Text, int Button_Size);
// 	int Center_Text_Calc_Y(String Text);
//
//
// 	// ------------------------------------------ Settings File ------------------------------------------
// 	word Text_Color;
//
// 	word Edge_Color;
// 	byte Edge_Size;
//
//
// 	// ------------------------------------------ Top Bar ------------------------------------------
// 	void Draw_Top_Bar(String Top_Bar_Text);
//
// 	int Top_Bar_Size;
// 	int Top_Bar_Button_Size;
//
//
// 	// ------------------------------------------ MISC ------------------------------------------
// 	int Current_Page = 1;
//
//
//
// // --------------------------------------------------------------------------------------------
// // ------------------------------------------ CanNet - UFTF.cpp ------------------------------------------
// // --------------------------------------------------------------------------------------------
//
// void UTFT::Draw_Button(String Button_Text, int Start_X, int Start_Y) {
//
// 	// Edge
// 		setColor(Button_Edge_Color);
//
// 		if (Draw_Edge_Size == 0);
//
// 		else if (Draw_Edge_Size == 1) {
// 			drawRoundRect(Start_X, Start_Y, Start_X + Draw_Size_X, Start_Y + Draw_Size_Y);
// 		}
//
// 		else {
// 			fillRoundRect (Start_X, Start_Y, Start_X + Draw_Size_X, Start_Y + Draw_Size_Y);
// 		}
//
//
// 	// Button
// 		setColor(Button_Color);
// 		fillRoundRect (
// 										Start_X + Draw_Edge_Size,
// 										Start_Y + Draw_Edge_Size,
// 										Start_X + Draw_Size_X - Draw_Edge_Size,
// 										Start_Y + Draw_Size_Y - Draw_Edge_Size
// 									);
//
// 	// Text
// 	if (Button_Text != "") {
// 		setColor(Text_Color);
// 		setBackColor(Button_Color);
//
// 			if (Button_Center_Text == true) {
// 				print(
// 					Button_Text,
// 					Start_X + Center_Text_Calc_X(Button_Text),
// 					Start_Y + Center_Text_Calc_Y(Button_Text)
// 				);
// 			}
//
// 			else {
// 				print( // CHANGE ME TO SOMETHING USEFUL
// 					Button_Text,
// 					Start_X + 15,
// 					Start_Y + 15
// 				);
// 			}
//   } // if (Button_Text != "")
//
// } // Draw_Button
//
//
//
//
// int UTFT::Center_Text_Calc_X(String Text, int Button_Size) {
//
// 	return Button_Size / 2 - Text.length() * (getFontXsize() / 2);
//
// } // Center_Text_Calc_X
//
// int UTFT::Center_Text_Calc_X(String Text) { // Referance only
// 	return Center_Text_Calc_X(Text, Draw_Size_X);
// } // Center_Text_Calc_X - Reff
//
//
// int UTFT::Center_Text_Calc_Y(String Text, int Button_Size) {
// 	return Button_Size / 2 - (getFontYsize() / 2);
// } // Center_Text_Calc_X
//
// int UTFT::Center_Text_Calc_Y(String Text) { // Referance only
// 	return Center_Text_Calc_Y(Text, Draw_Size_Y);
// } // Center_Text_Calc_Y - Reff
//
//
// // ------------------------------------------ Top Bar ------------------------------------------
// void UTFT::Draw_Top_Bar(String Top_Bar_Text) {
//
// 	Draw_Size_X = getDisplayXSize() - 1;
// 	Draw_Size_Y = Top_Bar_Size;
//
// 	Draw_Edge_Size = 0;
//
// 	setColor(Button_Color);
// 	setBackColor(Button_Color);
//
//
// 	Draw_Button(Top_Bar_Text, 0, 0);
// 	// Draw_Button(String Button_Text, int Start_X, int Start_Y);
// 	// Find_Setting(String &File_Content, String Setting_Name)
//
//
//   //
//   //
// 	// //-------------------------------------------------- Drws Page up / Page down --------------------------------------------------
// 	// if (Top_Bar_Page_Number < 9) {
// 	// 	Button_Size_2_X = 5 * getFontXsize();
// 	// 	Button_Size_2_Y = Top_Bar_Size;
// 	// }
//   //
// 	// else {
// 	// 	Button_Size_2_X = 6 * getFontXsize();
// 	// 	Button_Size_2_Y = Top_Bar_Size;
// 	// }
//   //
// 	// Draw_Edge_Size = Top_Bar_Draw_Edge_Size;
//   //
// 	// if (Top_Bar_Page_Number != 1) {
// 	// 	String Page_Number_Text = "<< " + String(Top_Bar_Page_Number - 1);
//   //
// 	// 	Draw_Button(Page_Number_Text, 0, 0, true); // Button_Size_2
// 	// }
//   //
// 	// if (Top_Bar_Page_Number != Top_Bar_Page_Number_Last) {
// 	// 	String Page_Number_Text = String(Top_Bar_Page_Number + 1) + " >>";
//   //
// 	// 	Draw_Button(Page_Number_Text, getDisplayXSize() - 1 - Button_Size_2_X, 0, true); // Button_Size_2
// 	// }
//
// } // Draw_Top_Bar
//
//
//
// //
// //
// //
// // // void UTFT::Draw_Button_Matrix(String Button_Text, byte Button_Number_X, byte Button_Number_Y, bool Use_Button_Size_2) {
// // //
// // //
// // // 		Draw_Button(
// // // 	    Button_Matrix_Spacing * Button_Number_X + Draw_Size_X * (Button_Number_X - 1),
// // // 	    Top_Bar_Size + Button_Matrix_Spacing * Button_Number_Y + Draw_Size_Y * (Button_Number_Y - 1)
// // // 		);
// // //
// // //
// // //
// // //
// // // } // Draw_Button_Matrix
// // //
// // // void UTFT::Draw_Button_Matrix(String Button_Text, byte Button_Number_X, byte Button_Number_Y) { // Referance only
// // //
// // // 	Draw_Button_Matrix(Button_Text, Button_Number_X, Button_Number_Y, false);
// // //
// // // } // Draw_Button_Matrix
// //
// //
// //
// //
// // // -----------------------------------------------------------------------------------------------------------------
// // // ---------------------------------------------------- Top Bar ----------------------------------------------------
// // // -----------------------------------------------------------------------------------------------------------------
// //
// // // void UTFT::Draw_Top_Bar() {
// // //
// //   //
// // 	// //-------------------------------------------------- Drws the top bar --------------------------------------------------
// // 	// Draw_Size_X = getDisplayXSize() - 1;
// // 	// Draw_Size_Y = Top_Bar_Size;
// // 	// Draw_Edge_Size = 0;
// //   //
// // 	// Draw_Button(_Top_Bar_Text_Array[Top_Bar_Page_Number - 1], 0, 0);
// //   //
// //   //
// // 	// //-------------------------------------------------- Drws Page up / Page down --------------------------------------------------
// // 	// if (Top_Bar_Page_Number < 9) {
// // 	// 	Button_Size_2_X = 5 * getFontXsize();
// // 	// 	Button_Size_2_Y = Top_Bar_Size;
// // 	// }
// //   //
// // 	// else {
// // 	// 	Button_Size_2_X = 6 * getFontXsize();
// // 	// 	Button_Size_2_Y = Top_Bar_Size;
// // 	// }
// //   //
// // 	// Draw_Edge_Size = Top_Bar_Draw_Edge_Size;
// //   //
// // 	// if (Top_Bar_Page_Number != 1) {
// // 	// 	String Page_Number_Text = "<< " + String(Top_Bar_Page_Number - 1);
// //   //
// // 	// 	Draw_Button(Page_Number_Text, 0, 0, true); // Button_Size_2
// // 	// }
// //   //
// // 	// if (Top_Bar_Page_Number != Top_Bar_Page_Number_Last) {
// // 	// 	String Page_Number_Text = String(Top_Bar_Page_Number + 1) + " >>";
// //   //
// // 	// 	Draw_Button(Page_Number_Text, getDisplayXSize() - 1 - Button_Size_2_X, 0, true); // Button_Size_2
// // 	// }
// // //
// // // } // Draw_Top_Bar
// //
// //
// // // void UTFT::Set_Top_Bar_Text(String Top_Bar_Text) {
// // //
// // // 	Top_Bar_Page_Number_Last = 0;
// // //
// // // 	for (int x = 0; x < 15; x++) {
// // //
// // // 		Top_Bar_Page_Number_Last = Top_Bar_Text.indexOf(";", Top_Bar_Page_Number_Last + 1);
// // //
// // // 		if (Top_Bar_Page_Number_Last == -1 || Top_Bar_Text.length() == 0) {
// // // 			Top_Bar_Page_Number_Last = x;
// // // 			break;
// // // 		}
// // //
// // // 		_Top_Bar_Text_Array[x] = Top_Bar_Text.substring(0, Top_Bar_Text.indexOf(";"));
// // //
// // // 		Top_Bar_Text.replace(_Top_Bar_Text_Array[x] + ";", "");
// // //
// // // 	} // for loop
// // //
// // // } // Set_Top_Bar_Text
// //
// // // String UTFT::Get_Top_Bar_Text() {
// // //
// // // 	String Return_String;
// // //
// // // 	for (int x = 0; x < Top_Bar_Page_Number_Last; x++) {
// // //
// // // 		Return_String = Return_String + _Top_Bar_Text_Array[x] + ";";
// // //
// // // 	} // for loop
// // //
// // // 	return Return_String;
// // //
// // // } // Get_Top_Bar_Text
// //
// //
// //
// // // -------------------------- Slider --------------------------
// // void UTFT::Draw_Slider(int Potition_X) {
// //
// // 	int x = Potition_X;
// //
// // 	if (_Slider_Dont_Move_Until > millis()) return;
// //
// // 	if (x < Slider_Restrict_X_Begin) {
// // 		x = Slider_Restrict_X_Begin;
// // 	}
// //
// // 	else if (x > Slider_Restrict_X_End) {
// // 		x = Slider_Restrict_X_End - Slider_Size_X * 10 - Slider_Spacing;
// // 	}
// //
// // 	// ************ Removing the old slider ************
// // 	if (_Slider_Last_Position == -9999); // -9999 = No marker present
// //
// // 	else {
// // 		setColor(Slider_Color_Replace);
// // 		fillRoundRect(
// // 									_Slider_Last_Position,											// x1
// // 									Slider_Y_Axis,															// y1
// // 									_Slider_Last_Position + Slider_Size_X,			// x2
// // 									Slider_Y_Axis + Slider_Size_Y	 							// y2
// // 									);
// //
// // 	} // else
// //
// //
// // 	// ************ Draws the new slider ************
// // 	setColor(Slider_Color);
// // 	fillRoundRect(
// // 								x,											// x1
// // 								Slider_Y_Axis,									// y1
// // 								x + Slider_Size_X,			// x2
// // 								Slider_Y_Axis + Slider_Size_Y	 	// y2
// // 								);
// //
// // 	_Slider_Last_Position = x;
// // 	_Slider_Dont_Move_Until = millis() + Slider_Dont_Move_For;
// //
// // } // Draw_Slider
// //
// //
// //
// //
// // // ----------------------------------------- Page_Add_Button -------------------------------------------
// // void UTFT::Page_Add_Button(byte Page_Number, byte Button_ID, String Button_Text, int Position_X, int Position_Y) {
// //
// // 	// if (Page_Buttons[Page_Number].indexOf(String(Page_Number) + ";") != -1) {
// // 	// 	Serial.println("Button exists, removing then addidng"); // rm
// // 	// 	Page_Remove_Button(Page_Number, Button_ID);
// // 	// 	Serial.println("Add button again"); // rm
// // 	// }
// //   //
// // 	// else {
// // 	// 	Serial.println("Adding"); // rm
// //   //
// // 	// 	Page_Buttons[Page_Number] =
// // 	// 															Page_Buttons[Page_Number] + ";-" +
// // 	// 															Page_Number + ";" +
// // 	// 															Button_Text + ";" +
// // 	// 															Position_X + ";" +
// // 	// 															Position_Y +
// // 	// 															"-;";
// //   //
// //   //
// //   //
// //   //
// //   //
// //   //
// // 	// }
// //
// // 	// ID;;Text;;Position_X;;Position_Y
// // 	// ;-01;;Button 1;;400;;200-;
// //
// // };
// //
// //
// // // ----------------------------------------- Page_Enable_Button -------------------------------------------
// // void UTFT::Page_Enable_Button(byte Page_Number, byte Button_ID) {
// //
// // };
// //
// // // ----------------------------------------- Page_Disable_Button -------------------------------------------
// // void UTFT::Page_Disable_Button(byte Page_Number, byte Button_ID) {
// //
// // };
// //
// // // ----------------------------------------- Page_Remove_Button -------------------------------------------
// // void UTFT::Page_Remove_Button(byte Page_Number, byte Button_ID) {
// //
// // };
// //
// // // ----------------------------------------- Page_Draw -------------------------------------------
// // void UTFT::Page_Draw(byte Page_Number) {
// //
// // 	Serial.println("Page_Draw"); // rm
// //
// // };
