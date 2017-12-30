
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
UTFT lcd(CTE50, 38, 39, 40, 41);
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

    lcd.fillScr(0xF800);
    lcd.print("FATAL ERROR: " + Error_Text, 20, 20);

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

String Read_Conf_File(String File_Path) { // Referance only
  return Read_Conf_File(File_Path, true);
} // END MARKER - Read_Conf_File


String Find_Setting(String &Settings_File_Content, String Setting_Name) {

  String Search_String = "\r\n" + Setting_Name + " = ";

  if (Settings_File_Content.indexOf(Search_String) == -1) {
    return "";
  }



  int Settings_Position = Settings_File_Content.indexOf(Search_String) + Search_String.length();

  return Settings_File_Content.substring(
                                          Settings_Position,
                                          Settings_File_Content.indexOf("\r\n", Settings_Position)
                                        );

} // END MARKER - Find_Setting

word Find_Setting_Color(String &Settings_File_Content, String Setting_Name) {

  String Search_String = "\r\n" + Setting_Name + " = ";

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

int Find_Setting_Int(String &Settings_File_Content, String Setting_Name) {

  Setting_Name = Find_Setting(Settings_File_Content, Setting_Name);

  return Setting_Name.toInt();

} // END MARKER - Find_Setting



void Top_Bar() {

  lcd.clrScr();

  if (Top_Bar_File_Content != "") {
    lcd.Draw_Top_Bar(Find_Setting(Page_File_Content[lcd.Current_Page], "Name"));
  }








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
  lcd.InitLCD();
  lcd.setFont(GroteskBold16x32);


  // -------------------------------------------- Touch --------------------------------------------
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);


  // -------------------------------------------- Boot Message --------------------------------------------
  lcd.fillScr(0xF800);
  lcd.setBackColor(0x0000);
  lcd.print(String("Booting"), CENTER, lcd.getDisplayYSize() / 2 - 5);
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

    Page_File_Content[x] = Read_Conf_File(String(Page_File_Path) + "Page_" + x + ".txt", false);

    if (Page_File_Content[x] == "") break;
  }

  Serial.print("Pages: "); // REMOVE ME
  Serial.println(freeMemory()); // REMOVE ME


  // -------------------------------------------- Settings file import --------------------------------------------
  if (Settings_File_Content.indexOf("\r\nText Color = ") != -1) {
    lcd.Text_Color = Find_Setting_Color(Settings_File_Content, "Text Color");
  }

  if (Settings_File_Content.indexOf("\r\nEdge Color = ") != -1) {
    lcd.Edge_Color = Find_Setting_Color(Settings_File_Content, "Edge Color");
  }

  if (Settings_File_Content.indexOf("\r\nEdge Size = ") != -1) {
    lcd.Edge_Size = Find_Setting_Int(Settings_File_Content, "Edge Size");
  }

  if (Settings_File_Content.indexOf("\r\nButton Center Text = ") != -1) {
    lcd.Button_Center_Text = Find_Setting_Int(Settings_File_Content, "Button Center Text");
  }


  // -------------------------------------------- Top Bar file import --------------------------------------------
  if (Top_Bar_File_Content.indexOf("\r\nSize = ") != -1) {
    lcd.Top_Bar_Size = Find_Setting_Int(Top_Bar_File_Content, "Size");
  }

  if (Top_Bar_File_Content.indexOf("\r\nButton Size = ") != -1) {
    lcd.Top_Bar_Button_Size = Find_Setting_Int(Top_Bar_File_Content, "Button Size");
  }


  // -------------------------------------------- Boot Message - End --------------------------------------------
  lcd.print(String("Boot Done"), CENTER, lcd.getDisplayYSize() / 2 - 5);
  Serial.println("Boot Done");

  delay(500);

  Top_Bar();

} // END MARKER - setup

void loop() {


} // END MARKER - loop


// // --------------------------------------------------------------------------------------------
// // ------------------------------------------ CanNet - UTFT.h ------------------------------------------
// // --------------------------------------------------------------------------------------------
//
// public:
//
// 	void Draw_Button(String Button_Text, int Start_X, int Start_Y);
// 	int Button_Size_X;
// 	int Button_Size_Y;
//
// 	word Button_Color;
// 	// word Button_Text_Color;
//
// 	word Button_Edge_Color;
// 	int Button_Edge_Size;
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



// // --------------------------------------------------------------------------------------------
// // ------------------------------------------ CanNet - UFTF.cpp ------------------------------------------
// // --------------------------------------------------------------------------------------------
//
// void UTFT::Draw_Button(String Button_Text, int Start_X, int Start_Y) {
//
// 	// Edge
// 		setColor(Button_Edge_Color);
//
// 		if (Button_Edge_Size == 0);
//
// 		else if (Button_Edge_Size == 1) {
// 			drawRoundRect(Start_X, Start_Y, Start_X + Button_Size_X, Start_Y + Button_Size_Y);
// 		}
//
// 		else {
// 			fillRoundRect (Start_X, Start_Y, Start_X + Button_Size_X, Start_Y + Button_Size_Y);
// 		}
//
//
// 	// Button
// 		setColor(Button_Color);
// 		fillRoundRect (
// 										Start_X + Button_Edge_Size,
// 										Start_Y + Button_Edge_Size,
// 										Start_X + Button_Size_X - Button_Edge_Size,
// 										Start_Y + Button_Size_Y - Button_Edge_Size
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
//   } // END MARKER - if (Button_Text != "")
//
// } // END MARKER - Draw_Button
//
//
//
//
// int UTFT::Center_Text_Calc_X(String Text, int Button_Size) {
//
// 	return Button_Size / 2 - Text.length() * (getFontXsize() / 2);
//
// } // END MARKER - Center_Text_Calc_X
//
// int UTFT::Center_Text_Calc_X(String Text) { // Referance only
// 	return Center_Text_Calc_X(Text, Button_Size_X);
// } // END MARKER - Center_Text_Calc_X - Reff
//
//
// int UTFT::Center_Text_Calc_Y(String Text, int Button_Size) {
// 	return Button_Size / 2 - (getFontYsize() / 2);
// } // END MARKER - Center_Text_Calc_X
//
// int UTFT::Center_Text_Calc_Y(String Text) { // Referance only
// 	return Center_Text_Calc_Y(Text, Button_Size_Y);
// } // END MARKER - Center_Text_Calc_Y - Reff
//
//
// // ------------------------------------------ Top Bar ------------------------------------------
// void UTFT::Draw_Top_Bar(String Top_Bar_Text) {
//
// 	Button_Size_X = getDisplayXSize() - 1;
// 	Button_Size_Y = Top_Bar_Size;
//
// 	Button_Edge_Size = 0;
//
// 	setColor(Button_Color);
// 	setBackColor(Button_Color);
//
//
// 	Draw_Button(Top_Bar_Text, 0, 0);
// 	// Draw_Button(String Button_Text, int Start_X, int Start_Y);
// 	// Find_Setting(String &Settings_File_Content, String Setting_Name)
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
// 	// Button_Edge_Size = Top_Bar_Button_Edge_Size;
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
// } // END MARKER - Draw_Top_Bar
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
// // // 	    Button_Matrix_Spacing * Button_Number_X + Button_Size_X * (Button_Number_X - 1),
// // // 	    Top_Bar_Size + Button_Matrix_Spacing * Button_Number_Y + Button_Size_Y * (Button_Number_Y - 1)
// // // 		);
// // //
// // //
// // //
// // //
// // // } // END MARKER - Draw_Button_Matrix
// // //
// // // void UTFT::Draw_Button_Matrix(String Button_Text, byte Button_Number_X, byte Button_Number_Y) { // Referance only
// // //
// // // 	Draw_Button_Matrix(Button_Text, Button_Number_X, Button_Number_Y, false);
// // //
// // // } // END MARKER - Draw_Button_Matrix
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
// // 	// Button_Size_X = getDisplayXSize() - 1;
// // 	// Button_Size_Y = Top_Bar_Size;
// // 	// Button_Edge_Size = 0;
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
// // 	// Button_Edge_Size = Top_Bar_Button_Edge_Size;
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
// // // } // END MARKER - Draw_Top_Bar
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
// // // 	} // END MARKER - for loop
// // //
// // // } // END MARKER - Set_Top_Bar_Text
// //
// // // String UTFT::Get_Top_Bar_Text() {
// // //
// // // 	String Return_String;
// // //
// // // 	for (int x = 0; x < Top_Bar_Page_Number_Last; x++) {
// // //
// // // 		Return_String = Return_String + _Top_Bar_Text_Array[x] + ";";
// // //
// // // 	} // END MARKER - for loop
// // //
// // // 	return Return_String;
// // //
// // // } // END MARKER - Get_Top_Bar_Text
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
// // 		Serial.println("MARKER");
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
// // 	} // END MARKER - else
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
// // } // END MARKER - Draw_Slider
// //
// //
// //
// //
// // // ----------------------------------------- Page_Add_Button -------------------------------------------
// // void UTFT::Page_Add_Button(byte Page_Number, byte Button_ID, String Button_Text, int Position_X, int Position_Y) {
// //
// // 	// if (Page_Buttons[Page_Number].indexOf(String(Page_Number) + ";") != -1) {
// // 	// 	Serial.println("Button exists, removing then addidng"); // REMOVE ME
// // 	// 	Page_Remove_Button(Page_Number, Button_ID);
// // 	// 	Serial.println("Add button again"); // REMOVE ME
// // 	// }
// //   //
// // 	// else {
// // 	// 	Serial.println("Adding"); // REMOVE ME
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
// // 	Serial.println("Page_Draw"); // REMOVE ME
// //
// // };
