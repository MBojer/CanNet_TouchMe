
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

#define Max_Pages 50

#define Max_Touch_Object 50

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
URTouch touch(6, 5, 4, 3, 2);

int Touch_Input_X;
int Touch_Input_Y;


// -------------------------------------------- Touch - Top Bar --------------------------------------------
int Top_Bar_Button_Spaceing = -1;


// -------------------------------------------- Touch - Page X - Matrix --------------------------------------------



// -------------------------------------------- Touch - Ignore input - Millis --------------------------------------------
int Top_Bar_Ignore_Input_For = 1000;
unsigned long Top_Bar_Ignore_Input_Until;

int Page_Ignore_Input_For = 500;
unsigned long Page_Ignore_Input_Until;

#define Slider_Dont_Move_For 100
unsigned long Slider_Dont_Move_Until;

// -------------------------------------------- Matrix Calc --------------------------------------------
int Matrix_Calc_Array[4];

#define X_Begin 0
#define X_End 1

#define Y_Begin 2
#define Y_End 3


// -------------------------------------------- Matrix Calc Pos --------------------------------------------
int Matrix_Calc_Pos_X;
int Matrix_Calc_Pos_Y;
int Matrix_Calc_Pos_List;


// -------------------------------------------- Find Matrix Action --------------------------------------------
String Find_Matrix_Action_Line;


// -------------------------------------------- MISC --------------------------------------------
int Current_Page = 1;
int Last_Page = 1;


// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h>
unsigned long freeMemory_Last;
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 250
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


// -------------------------------------------- CanNet - Draw Slider Matrix --------------------------------------------
int Full_Screen_Button_Size = 0;

#define Slider_Size_X 10
#define Slider_Spazing 2

int Slider_Last_Position = -1;



// -------------------------------------------- CanNet - URTouch --------------------------------------------
bool Flip_Touch = false;


// -------------------------------------------- CanNet - Matrix_Object_List --------------------------------------------
String _Matrix_Object_List[2];

// Name of lists:
#define Matrix_Button 0
#define Matrix_Slider 1



// -------------------------------------------- CanNet - MISC --------------------------------------------
String _Slider_Last_Position;





// -------------------------------------------------------------------------------------------------------


// -------------------------------------------- CanNet - MISC --------------------------------------------
void Error_Mode(byte Error_Type, String Error_Text) {

  // Error_Type:
  //  1 = Halt
  //  2 = Continues

  // ADD ME - Support for write log file to sd if avalible
  // ADD ME - Support for write error on screen
  // ADD ME - When error type 1, turn the screen and unit off

  if (Error_Type == 1) {
    if (Serial) {
      Serial.print("FATAL ERROR: ");
      Serial.println(Error_Text);
      Serial.println("HALTING");
    }

    lcd.fillScr(0xF800);
    lcd.setBackColor(0x0000);
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

void Matrix_Object_List(byte List, int X_Matrix, int Y_Matrix) {

  _Matrix_Object_List[List] = _Matrix_Object_List[List] + X_Matrix + "-" + Y_Matrix + ":";

} // Matrix_Object_List

String Matrix_Object_List(byte List) {

  return _Matrix_Object_List[List];

} // Matrix_Object_List

void Matrix_Object_List_Reset() {

  // CHANGE ME - to a loop

  _Matrix_Object_List[Matrix_Button] = ":";
  _Matrix_Object_List[Matrix_Slider] = ":";
} // Matrix_Object_List_Reset


void Slider_Last_Position_Set(int &X_Position) {

  String Search_For = ":" + String(Matrix_Calc_Pos_X) + "-" + String(Matrix_Calc_Pos_Y);

  if (_Slider_Last_Position.indexOf(Search_For + "-") != -1) { // Value alreay on list

    int Temp_Int = _Slider_Last_Position.indexOf(Search_For);

    String Replace_String = _Slider_Last_Position.substring(Temp_Int, _Slider_Last_Position.indexOf(":", Temp_Int + 1) + 1);

    _Slider_Last_Position.replace(Replace_String , Search_For + "-" + X_Position + ":");

  } // Value alreay on list


  else { // Not on list
    _Slider_Last_Position = _Slider_Last_Position + Search_For + "-" + X_Position + ":";
  } // Not on list


}

int Slider_Last_Position_Get() {

  String Temp_String = ":" + String(Matrix_Calc_Pos_X) + "-" + String(Matrix_Calc_Pos_Y) + "-";

  int Temp_Int = _Slider_Last_Position.indexOf(Temp_String);

  if (Temp_Int != -1) { // Value on list

    Temp_String = _Slider_Last_Position.substring(Temp_Int + Temp_String.length(), _Slider_Last_Position.indexOf(":", Temp_Int + 1));

    Slider_Last_Position = Temp_String.toInt();
    return Slider_Last_Position;

  }
  return -1;
} // Slider_Last_Position


// -------------------------------------------- CanNet - Calculator --------------------------------------------
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


void Matrix_Calc(int X_Number, int Y_Number) {

  // ---------- X ----------
  Matrix_Calc_Array[X_Begin] = Matrix_Spacing * X_Number + Draw_Size_X * (X_Number - 1);
  Matrix_Calc_Array[X_End] = Matrix_Spacing * X_Number + Draw_Size_X * X_Number;

  // ---------- Y ----------
  Matrix_Calc_Array[Y_Begin] = Top_Bar_Size + Matrix_Spacing * Y_Number + Draw_Size_Y * (Y_Number - 1);
  Matrix_Calc_Array[Y_End] = Top_Bar_Size + Matrix_Spacing * Y_Number + Draw_Size_Y * Y_Number;

} // Matrix_Calc

int Matrix_Calc_X(int X_Number) {
  return Matrix_Spacing * X_Number + Draw_Size_X * (X_Number - 1);
} // Matrix_Calc_X

int Matrix_Calc_Y(int Y_Number) {
  return Top_Bar_Size + Matrix_Spacing * Y_Number + Draw_Size_Y * (Y_Number - 1);
} // Matrix_Calc_Y

bool Matrix_Calc_Pos(int X_Input, int Y_Input) {

  // -------------------------------------------- Y --------------------------------------------
  Y_Input = Y_Input - Top_Bar_Size - Matrix_Spacing;

  bool Match_Found = false;

  // -------------------------------------------- Y --------------------------------------------
  for (int Loop_Count = 1; Loop_Count < Max_Touch_Object + 1; Loop_Count++) {

    if (Y_Input < 0) { // No Match = return
      return false;
    }

    else if (Y_Input > 0 && Y_Input < Button_Size_Y) { // Match
      Y_Input = Loop_Count;
      break;
    }

    else if (Loop_Count == Max_Touch_Object) { // Max_Touch_Object reached = Touch is DISABLED
      Error_Mode(2, "For loop Y_Input ran till end.\r\nTouch is disabled due to this.\r\nTo resolve this, either increase Max_Touch_Object or remove some buttons from the page config file.");
      return false;
    }

    Y_Input = Y_Input - Button_Size_Y - Matrix_Spacing;

  } // for - Y


  // -------------------------------------------- X - Matrix Slider --------------------------------------------
  // Has to be furst due to "X_Input = 0" it will get a false negative below
  if (Matrix_Object_List(Matrix_Slider) != ":") {
    if (Matrix_Object_List(Matrix_Slider).indexOf(":0-" + String(Y_Input) + ":") != -1) { // Full Screen Button
      X_Input = 0;
      Matrix_Calc_Pos_List = Matrix_Slider;
      Match_Found = true;
    }
  } // if (Matrix_Object_List_[1] != "")


  // -------------------------------------------- X - Normal Button --------------------------------------------
  if (Matrix_Object_List(Matrix_Button) != ":" && Match_Found == false) {
    X_Input = X_Input - Matrix_Spacing;

    for (int Loop_Count = 1; Loop_Count < Max_Touch_Object + 1; Loop_Count++) {

      if (X_Input < 0) { // No Match = return
        return false;
      }

      else if (X_Input > 0 && X_Input < Button_Size_X) { // Match

        if (Matrix_Object_List(Matrix_Button).indexOf(":" + String(Loop_Count) + "-" + String(Y_Input) + ":") == -1) { // Match Check
          return false;
        }

        X_Input = Loop_Count;
        Matrix_Calc_Pos_List = Matrix_Button;
        Match_Found = true;

        break;
      } // else if (X_Input > 0 && X_Input < Button_Size_X)

      else if (Loop_Count == Max_Touch_Object) { // Max_Touch_Object reached = Touch is DISABLED
        Error_Mode(2, "For loop X_Input ran till end.\r\nTouch is disabled due to this.\r\nTo resolve this, either increase Max_Touch_Object or remove some buttons from the page config file.");
        return false;
      }

      X_Input = X_Input - Button_Size_X - Matrix_Spacing;

    } // for X

  } // if (Matrix_Object_List(Matrix_Button)


  // ----------------------------- Match Found -----------------------------
  Matrix_Calc_Pos_X = X_Input;
  Matrix_Calc_Pos_Y = Y_Input;

  return true;

} // Matrix_Calc_Pos



// -------------------------------------------- CanNet - Calculator - Stolen --------------------------------------------
// https://github.com/benrugg/Arduino-Hex-Decimal-Conversion/blob/master/hex_dec.ino
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

    while (File_Content.indexOf("\r\n\r\n") != -1) {
      // if (File_Content.indexOf("\r\n\r\n") == -1) break; // REMOVE ME - Testing
      File_Content.replace("\r\n\r\n", "\r\n");
    }

    return File_Content;
}

String Read_Conf_File(String File_Path) { // Referance only
  return Read_Conf_File(File_Path, true);
} // Read_Conf_File


// -------------------------------------------- CanNet - Find Settings --------------------------------------------
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

String Find_Matrix_Action(int X_MPos, int Y_MPos, String Setting_Name) {

  String Action_List = Page_File_Content;

  Action_List.replace("Settings:\r\n", "");

  for (int Line_Number = 0; Line_Number < Max_Touch_Object; Line_Number++) {
    Find_Matrix_Action_Line = Action_List.substring(0, Action_List.indexOf("\r\n"));

    if (Find_Matrix_Action_Line.indexOf("X:" + String(X_MPos)) != -1) { // Match X

        if (Find_Matrix_Action_Line.indexOf("Y:" + String(Y_MPos)) != -1) { // Match Y
          return Find_Sub_Setting(Find_Matrix_Action_Line, Setting_Name);
        }

    }  // Match X

    else if (Find_Matrix_Action_Line.indexOf("N:") != -1) { // Line contains "Name" discarding line
      Line_Number--;
    }

    Action_List.replace(Find_Matrix_Action_Line + "\r\n", "");

    if (Action_List.length() == 0) {
      return "";
    }

  } // for Line_Number

  return "";

} // Find_Matrix_Action

String Find_Matrix_Action(String Setting_Name) {

  return Find_Sub_Setting(Find_Matrix_Action_Line, Setting_Name);

} // Find_Matrix_Action(String Setting_Name)


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

void Draw_Button_Matrix(String Button_Text, int X_Number, int Y_Number) {

  Matrix_Calc(X_Number, Y_Number);

	if (true) { // Edge
		lcd.setColor(Edge_Color);

		if (Draw_Edge_Size == 0); // No edge no do nothing

		else if (Draw_Edge_Size == 1)
      lcd.drawRoundRect(Matrix_Calc_Array[X_Begin],
                        Matrix_Calc_Array[Y_Begin],
                        Matrix_Calc_Array[X_End],
                        Matrix_Calc_Array[Y_End]);

		else
      lcd.fillRoundRect(Matrix_Calc_Array[X_Begin],
                        Matrix_Calc_Array[Y_Begin],
                        Matrix_Calc_Array[X_End],
                        Matrix_Calc_Array[Y_End]);

  } // Edge

	if (true) { // Button
		lcd.setColor(Button_Color);
		lcd.fillRoundRect(Matrix_Calc_Array[X_Begin] + Draw_Edge_Size,
                      Matrix_Calc_Array[Y_Begin] + Draw_Edge_Size,
                      Matrix_Calc_Array[X_End] - Draw_Edge_Size,
                      Matrix_Calc_Array[Y_End] - Draw_Edge_Size
    );
  } // Button

	if (Button_Text != "") { // Text
		lcd.setColor(Text_Color);
		lcd.setBackColor(Button_Color);

			if (Button_Center_Text == true) {
				lcd.print(
					Button_Text,
					Matrix_Calc_Array[X_Begin] + Center_Text_Calc_X(Button_Text),
					Matrix_Calc_Array[Y_Begin] + Center_Text_Calc_Y(Button_Text)
				);
			}

			else {
				lcd.print( // CHANGE ME TO SOMETHING USEFUL
					Button_Text,
					Matrix_Calc_Array[X_Begin] + 15,
					Matrix_Calc_Array[Y_Begin] + 15
				);
			}
  } // if (Button_Text != "")

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

  Matrix_Object_List_Reset();

  for (int x = 0; x < Max_Pages + 1; x++) {

    // ---------------------------------- Matrix Button ----------------------------------
    if (Page_Content.indexOf("\r\nMatrix Button = ") != -1) {

      Draw_Size_X = Button_Size_X;
      Draw_Size_Y = Button_Size_Y;

      String Button_Settings = Find_Setting(Page_Content, "Matrix Button");

      int Matrix_X = Find_Sub_Setting_Int(Button_Settings, "X");
      int Matrix_Y = Find_Sub_Setting_Int(Button_Settings, "Y");

      Matrix_Object_List(Matrix_Button, Matrix_X, Matrix_Y);

      Draw_Button_Matrix(Find_Sub_Setting(Button_Settings, "N"), Matrix_X, Matrix_Y);

      Page_Content.replace("\r\nMatrix Button = " + Button_Settings, ""); // Removed the entry that was just drawn
    } // Matrix Button


    // ---------------------------------- Matrix Slider ----------------------------------
    if (Page_Content.indexOf("\r\nMatrix Slider = ") != -1) {

      Draw_Size_Y = Button_Size_Y;

      String Slider_Settings = Find_Setting(Page_Content, "Matrix Slider");

      int Matrix_X = Find_Sub_Setting_Int(Slider_Settings, "X");
      int Matrix_Y = Find_Sub_Setting_Int(Slider_Settings, "Y");

      Matrix_Object_List(Matrix_Slider, Matrix_X, Matrix_Y);

      if (Find_Sub_Setting_Int(Slider_Settings, "X") == 0) {

        if (Full_Screen_Button_Size == 0) {
          Full_Screen_Button_Size = lcd.getDisplayXSize() - Matrix_Spacing * 2;
        }

        Draw_Size_X = Full_Screen_Button_Size;
        Matrix_X = 1;
      } // X == 0

      else { // X != 0
        Draw_Size_X = Button_Size_X;
      } // X != 0

      Draw_Button_Matrix("", Matrix_X, Matrix_Y);

      Page_Content.replace("\r\nMatrix Slider = " + Slider_Settings, ""); // Removed the entry that was just drawn
    } // Matrix Slider


    // ---------------------------------- End of loop cheks ----------------------------------
    else if (Page_Content == "Settings:\r\n") break; // No entries left in file

    else if (x == Max_Pages) {
      Page_Content.replace("Settings:\r\n", "");
      Error_Mode(2, "Draw_Page, Settings left over:\r\n" + Page_Content);
      break; // Assuming error in page file
    }

  }

} // Draw_Page

void Draw_Slider_Marker_Matrix(byte Y_Matrix_Position) {

	if (Slider_Dont_Move_Until > millis()) return;

  // --------------------------  Full Screen Slider --------------------------
  if (Matrix_Calc_Pos_X == 0) {

    int Slider_Y_Axis = Matrix_Calc_Y(Matrix_Calc_Pos_Y) + Edge_Size + Slider_Spazing;
    int Slider_Size_Y = Button_Size_Y - Edge_Size * 2 - Slider_Spazing * 2;
    int X_Position = Touch_Input_X;


    // Left of button
    if (X_Position < Matrix_Spacing + Edge_Size + 1) {
  		X_Position = Matrix_Spacing + Edge_Size + 1;
  	}

    // Right of button
  	else if (X_Position > Matrix_Spacing + Full_Screen_Button_Size - Edge_Size - Slider_Spazing - Slider_Size_X) {
  		X_Position = Matrix_Spacing + Full_Screen_Button_Size - Edge_Size - Slider_Spazing - Slider_Size_X;
  	}


  	// ************ Removing the old slider ************
  	if (Slider_Last_Position_Get() != -1) {

  		lcd.setColor(Button_Color);
  		lcd.fillRoundRect(
  									Slider_Last_Position,											// x1
  									Slider_Y_Axis,														// y1
  									Slider_Last_Position + Slider_Size_X,			// x2
  									Slider_Y_Axis + Slider_Size_Y	); 					// y2

  	} // Removing the old slider


  	// ************ Draws the new slider ************
  	lcd.setColor(Edge_Color);
  	lcd.fillRoundRect(
  								X_Position,											// x1
  								Slider_Y_Axis,									// y1
  								X_Position + Slider_Size_X,			// x2
  								Slider_Y_Axis + Slider_Size_Y );	 	// y2


    Slider_Last_Position_Set(X_Position);
    Slider_Dont_Move_Until = millis() + Slider_Dont_Move_For;

  } // if (Matrix_Calc_Pos_X == 0) - Full Screen Slider

  // --------------------------  Normal Slider --------------------------
  else {

    // ADD ME - Funcunality for this
    Error_Mode(2, "No support for none fullscreen slider aka 0-Y"); // CHANGE ME

  } // else - Normal Slider



} // Draw_Slider




// -------------------------------------------- Top_Bar --------------------------------------------
void Top_Bar() {

  // Reads the Page config file to variable
  Page_File_Content = Read_Conf_File(String(Page_File_Path) + "Page_" + Current_Page + ".txt", false);

  lcd.clrScr();

  if (Top_Bar_Present == true) {
    Draw_Top_Bar(Find_Setting(Page_File_Content, "Name"));
  }

  Draw_Page(Page_File_Content);

} // Top_Bar

void Top_Bar_Touch() {

  if (
      // !touch.dataAvailable() || // Screen not pressed
      Touch_Input_Y > Top_Bar_Size || // Input not matching top bar
      Top_Bar_Ignore_Input_Until > millis() || // Pressed to soon ignoreing input
      Top_Bar_Present == false)
    return; // No delaied output for Top_Bar_Touch

  else if (Touch_Input_Y > 0 && Touch_Input_Y < Top_Bar_Size) { // Y - Input matching top bar

    if (Top_Bar_Button_Spaceing == -1) {
      Top_Bar_Button_Spaceing = lcd.getDisplayXSize() - Top_Bar_Button_Size * 2;
    } // if (Top_Bar_Button_Spaceing == -1)

    if (Touch_Input_X > 0 && Touch_Input_X < Top_Bar_Button_Size) { // X - Page Down
      Top_Bar_Ignore_Input_Until = millis() + Top_Bar_Ignore_Input_For;

      if (Current_Page != 1) { // Ingnore input if you are at page 1
        Current_Page = Current_Page - 1;
        Top_Bar();
      } // if (Current_Page == 1)

    } // if (Touch_Input_X > 0 && Touch_Input_X < Top_Bar_Button_Size)

    else if (Touch_Input_X > Top_Bar_Button_Size + Top_Bar_Button_Spaceing && // X - Page Up
             Touch_Input_X < Top_Bar_Button_Size + Top_Bar_Button_Spaceing + Top_Bar_Button_Size ) {

      Top_Bar_Ignore_Input_Until = millis() + Top_Bar_Ignore_Input_For;

      if (Current_Page != Last_Page) { // Ingnore input if you are at the last page
        Current_Page = Current_Page + 1;
        Top_Bar();
      } // if (Current_Page != Last_Page)

    } // else if (Touch_Input_Y > 0 && Touch_Input_Y < _Top_Bar_Size)

  } // else if (Touch_Input_X > Top_Bar_Button_Size + Top_Bar_Button_Spaceing && ...

} // Top_Bar_Touch


// -------------------------------------------- Page X Touch --------------------------------------------
void Page_X_Touch() {

  if (
      Touch_Input_X == -1 || // Invalid Input - REMOVEM ME when delaied input needs to run
      Touch_Input_Y == -1 || // Invalid Input - REMOVEM ME when delaied input needs to run
      Touch_Input_Y < Top_Bar_Size + Matrix_Spacing || // Input not matching first button
      Page_Ignore_Input_Until > millis() // Pressed to soon ignoreing input
  ) return; // No delaied output for Top_Bar_Touch

  if (Matrix_Calc_Pos(Touch_Input_X, Touch_Input_Y) == true) { // Match found

    String Action = Find_Matrix_Action(Matrix_Calc_Pos_X, Matrix_Calc_Pos_Y, "A");

    if (Action == "") return; // Nothing to do

    switch (Matrix_Calc_Pos_List) {

      // ----------------------------------------------------
      case Matrix_Button:
        Serial.print("ADD ME - Queue Action: "); // rm
        Serial.println(Action); // rm

        // ADD ME - Queue Action

        Page_Ignore_Input_Until = millis() + Page_Ignore_Input_For;
        break; // Matrix_Button


      // ----------------------------------------------------
      case Matrix_Slider:

        byte Slider_Value;

        // --------------------------  Full Screen Slider --------------------------
        if (Matrix_Calc_Pos_X == 0) {

          // ADD ME - Queue Action
          // ADD ME - Queue Action
          // ADD ME - Queue Action
          // ADD ME - Queue Action


          if (Touch_Input_X <= Matrix_Spacing) { // OFF - Space left of button
            Slider_Value = 0;
          }

          else if (Touch_Input_X > Full_Screen_Button_Size + Matrix_Spacing) { // Max - Space right of button
            Slider_Value = 255;
          }

          else { // Anything else
            Slider_Value = float((Touch_Input_X - Matrix_Spacing) / (float(Full_Screen_Button_Size) / 255.00));
          }



          Draw_Slider_Marker_Matrix(Slider_Value);



        } // Full Screen Slider

        // --------------------------  Normal Slider --------------------------
        else {

          // ADD ME - Funcunality for this
          Error_Mode(2, "None fullscreen slider not supported"); // CHANGE ME

        }

        break; // Matrix_Slider


      // ----------------------------------------------------
      default:
        break;

    } // switch (Matrix_Calc_Pos_List)

  } // if (Matrix_Calc_Pos(Touch_Input_X, Touch_Input_Y) == true)

} // Page_X_Touch


// -------------------------------------------- Touch Check --------------------------------------------
void Touch_Check() {

  touch.read();

  Touch_Input_X = touch.getX();
  Touch_Input_Y = touch.getY();

  if (Flip_Touch == true) {
    if (Touch_Input_X != -1) Touch_Input_X = lcd.getDisplayXSize() - Touch_Input_X;
    if (Touch_Input_Y != -1) Touch_Input_Y = lcd.getDisplayYSize() - Touch_Input_Y;
  }


  // if (myTouch.Stabilize_Input(Touch_Input_X, Touch_Input_Y) == 1)  return; // Touch input diviated to much returning

  Top_Bar_Touch();

  if (Matrix_Object_List(Matrix_Button) != ":" || Matrix_Object_List(Matrix_Slider) != ":" ) {
    Page_X_Touch();
  }


} // Touch_Check


// -------------------------------------------- Setup --------------------------------------------
void setup() {

  // -------------------------------------------- LCD --------------------------------------------
  lcd.InitLCD();
  lcd.setFont(GroteskBold16x32);


  // -------------------------------------------- Touch --------------------------------------------
  touch.InitTouch();
  touch.setPrecision(PREC_MEDIUM);


  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);

  if (!Serial) { // rm
    for (int x = 0; x < 10; x++) {
      delay(100);
    }
  }

  if (Serial) { // rm
    delay(250);
  }


  // -------------------------------------------- SD Card --------------------------------------------
  if (!SD.begin(53)) {
    Error_Mode(1, "Initializing SD card");
  }


  // -------------------------------------------- Boot Message --------------------------------------------
  lcd.fillScr(0xF800);
  lcd.setBackColor(0x0000);
  lcd.print(String("Booting"), CENTER, lcd.getDisplayYSize() / 2 - 5);
  delay(500);


  // -------------------------------------------- Settings file import --------------------------------------------
  File_Content = Read_Conf_File(Setting_File_Path);

  if (File_Content != "") {

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

    if (File_Content.indexOf("\r\nIgnore Input For = ") != -1) {
        Page_Ignore_Input_For = Find_Setting_Int(File_Content, "Ignore Input For");
    }
  }


  // -------------------------------------------- Top Bar file import --------------------------------------------
  File_Content = Read_Conf_File(Top_Bar_File_Path);

  if (File_Content != "") {
    Top_Bar_Present = true;

    if (File_Content.indexOf("\r\nSize = ") != -1) {
      Top_Bar_Size = Find_Setting_Int(File_Content, "Size");
    }

    if (File_Content.indexOf("\r\nButton Size = ") != -1) {
      Top_Bar_Button_Size = Find_Setting_Int(File_Content, "Button Size");
    }

    if (File_Content.indexOf("\r\nIgnore Input For = ") != -1) {
      Top_Bar_Ignore_Input_For = Find_Setting_Int(File_Content, "Ignore Input For");
    }

    File_Content = "";
  }


  // -------------------------------------------- Page file import --------------------------------------------
  for (int x = 1; x < Max_Pages + 1; x++) {

    if (SD.exists(String(Page_File_Path) + "Page_" + x + ".txt"));

    else if (x == Max_Pages) {
      String Temp_String = "Page file import: for ran " + String(Max_Pages + 1) + " times.\r\nOnly " +
                            Max_Pages + " pages allowed. Isent that enought? :-)";
      Error_Mode(2, Temp_String);
    }

    else {
      Last_Page = x - 1;
      break;
    }

  } // for (int x = 1; x < Max_Pages + 1; x++)


  // -------------------------------------------- Boot Message - End --------------------------------------------
  lcd.print(String("Boot Done"), CENTER, lcd.getDisplayYSize() / 2 - 5);
  Serial.println("Boot Done");

  delay(500);

  Top_Bar();

} // setup

// -------------------------------------------- Loop --------------------------------------------
void loop() {

  // --------------------- REMOVE ME ---------------------------
  if (freeMemory_Delay_Until < millis()) { // rm

    unsigned long mesurement = freeMemory();

    if (freeMemory_Last != mesurement) {
      Serial.print("Free memory changed from: ");
      Serial.print(freeMemory_Last);
      Serial.print(" to: ");
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
