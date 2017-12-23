#include <Arduino.h>


#include <UTFT.h>
UTFT myGLCD(CTE50, 38, 39, 40, 41);

#include <URTouch.h>
URTouch  myTouch( 6, 5, 4, 3, 2);

// extern uint8_t BigFont[];
extern uint8_t GroteskBold16x32[];

#define Default_Ignore_Input_For 2000 // Time in millisecounds CHANGE ME


unsigned long Top_Bar_Ignore_Input_Until;


#define Main_Page_Ignore_Input_For 750 // Time in millisecounds CHANGE ME
#define Main_Page_Delay_Output_For 2500
#define Main_Page_Dimmer_Maker_Size_X 5 // It adds X pixels to each side
#define Main_Page_Dimmer_Maker_Move_Ignore_For 75
unsigned long Main_Page_Ignore_Input_Until;
unsigned long Main_Page_Delay_Output_Until;
unsigned long Main_Page_Dimmer_Maker_Move_Ignore_Untill;
bool Main_Page_Delay_Output_Waiting = false;
int Main_Page_Dimmer_Last_Marker_Potition = -9999;


int Display_Center_X;
int Display_Center_Y;

int Button_Spazing = 50;

int Touch_Input_X, Touch_Input_Y;

int Light_Strenght = 180; // CNAMGE ME

#define Page_Number_Last 3
int Page_Number = 1;
String Page_Number_Text_Array[Page_Number_Last] = {"Main","Relay's","Voltmeter's"};




void Main_Page() {

  myGLCD.Set_Button_Size(200, 90);
  myGLCD.Set_Button_Edge_Size(4);
  myGLCD.Set_Button_Matrix_Spacing(50);

  myGLCD.Draw_Button_Matrix("Old Lights", 1, 1);
  myGLCD.Draw_Button_Matrix("Red Lights", 2, 1);
  myGLCD.Draw_Button_Matrix("Gen / Inv", 3, 1);

  myGLCD.Draw_Button_Matrix("Internet", 1, 2);


  // --------------- 3 X 1 ---------------
  // --------------- Dimmer - Main ---------------
  myGLCD.Set_Button_Size_2(myGLCD.getDisplayXSize() - myGLCD.Get_Button_Matrix_Spacing() * 2, myGLCD.Get_Button_Size_Y());
  myGLCD.Draw_Button_Matrix("", 1, 3, true); // _Button_Size_2


} // END MARKER - Main_Page

void Main_Page_Touch() {

  myTouch.Set_Button_Size(myGLCD.Get_Button_Size_X(), myGLCD.Get_Button_Size_Y());
  myTouch.Set_Button_Matrix_Spacing(myGLCD.Get_Button_Matrix_Spacing());


  // --------------------------------------------- Dimmer Send ---------------------------------------------
  if (Main_Page_Delay_Output_Waiting == true && Main_Page_Delay_Output_Until < millis()) {
    Serial.print("Send Output - Main Page - Dimmer: "); // REMOVE ME
    Serial.println(Light_Strenght); // REMOVE ME

    Main_Page_Delay_Output_Waiting = false;
  }

    // --------------------------------------------- 3 X 1 ---------------------------------------------
    if (Touch_Input_X > 0 && Touch_Input_X < myGLCD.getDisplayXSize()) {

      if (Touch_Input_X > 0 && Touch_Input_X < Button_Spazing) { // Uses the space before the bar to make a bigger touch spot for 0%
        Light_Strenght = 0;
        Touch_Input_X = Button_Spazing + Main_Page_Dimmer_Maker_Size_X + 2 + myGLCD.Get_Button_Edge_Size(); // To make the maker apear at the start of the dimmer
      }

      else if (Touch_Input_X > myGLCD.getDisplayXSize() - Button_Spazing && Touch_Input_X < myGLCD.getDisplayXSize()) { // Asme as above just oposite (100%)
        Light_Strenght = 255;
        Touch_Input_X = myGLCD.getDisplayXSize() - Button_Spazing - Main_Page_Dimmer_Maker_Size_X - 2 - myGLCD.Get_Button_Edge_Size(); // To make the maker apear at the end of the dimmer
      }

      else {
        Light_Strenght = float((Touch_Input_X - Button_Spazing)  / 2.74); // 700 (Length of bar) / 255 (analogWrite Max) = 2,745098039215686 = 2.4 (rounding down to make sure it can go to 100%)
      }

      Main_Page_Delay_Output_Waiting = true;
      Main_Page_Delay_Output_Until = millis() + Main_Page_Delay_Output_For;
    } // END MARKER - 3 X 1

    if (Main_Page_Dimmer_Maker_Move_Ignore_Untill < millis()) {


      // Removing the old maker
      if (Main_Page_Dimmer_Last_Marker_Potition != -9999) { // -9999 = No marker present
        myGLCD.setColor(0, 255, 0);
        myGLCD.fillRoundRect(
                              Main_Page_Dimmer_Last_Marker_Potition - Main_Page_Dimmer_Maker_Size_X,
                              (myTouch.Get_Top_Bar_Size() + Button_Spazing * 3 + myGLCD.Get_Button_Size_Y() * 2) + 2 + myGLCD.Get_Button_Edge_Size(),
                              Main_Page_Dimmer_Last_Marker_Potition + Main_Page_Dimmer_Maker_Size_X,
                              (myTouch.Get_Top_Bar_Size() + Button_Spazing * 3 + myGLCD.Get_Button_Size_Y() * 3) - 2 - myGLCD.Get_Button_Edge_Size()
                            );
      }

      // Draws the new maker
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRoundRect(
                            Touch_Input_X - Main_Page_Dimmer_Maker_Size_X,
                            (myTouch.Get_Top_Bar_Size() + Button_Spazing * 3 + myGLCD.Get_Button_Size_Y() * 2) + 2 + myGLCD.Get_Button_Edge_Size(),
                            Touch_Input_X + Main_Page_Dimmer_Maker_Size_X,
                            (myTouch.Get_Top_Bar_Size() + Button_Spazing * 3 + myGLCD.Get_Button_Size_Y() * 3) - 2 - myGLCD.Get_Button_Edge_Size()
                          );

      // Notes marker potition
      Main_Page_Dimmer_Last_Marker_Potition = Touch_Input_X;
      Main_Page_Dimmer_Maker_Move_Ignore_Untill = millis() + Main_Page_Dimmer_Maker_Move_Ignore_For;
  }





  if (Main_Page_Ignore_Input_Until > millis()) return;

  // --------------------------------------------- 1 X ? ---------------------------------------------
  else if (Touch_Input_Y > myTouch.Get_Top_Bar_Size() + Button_Spazing && Touch_Input_Y < myTouch.Get_Top_Bar_Size() + Button_Spazing + myGLCD.Get_Button_Size_Y()) {


    // --------------------------------------------- 1 X 1 ---------------------------------------------
    if (Touch_Input_X > Button_Spazing && Touch_Input_X < (Button_Spazing + myGLCD.Get_Button_Size_X())) {
      Main_Page_Ignore_Input_Until = millis() + Main_Page_Ignore_Input_For;
      Serial.println("1 X 1"); // REMOVE ME
    }

    // --------------------------------------------- 1 X 2 ---------------------------------------------
    else if (Touch_Input_X > (Button_Spazing * 2 + myGLCD.Get_Button_Size_X()) && Touch_Input_X < (Button_Spazing * 2 + myGLCD.Get_Button_Size_X() * 2)) {
      Main_Page_Ignore_Input_Until = millis() + Main_Page_Ignore_Input_For;
      Serial.println("1 X 2"); // REMOVE ME
    }

    // --------------------------------------------- 1 X 2 ---------------------------------------------
    else if (Touch_Input_X > (Button_Spazing * 3 + myGLCD.Get_Button_Size_X() * 2) && Touch_Input_X < (Button_Spazing * 3 + myGLCD.Get_Button_Size_X() * 3)) {
      Main_Page_Ignore_Input_Until = millis() + Main_Page_Ignore_Input_For;
      Serial.println("1 X 3"); // REMOVE ME
    }
  }


  // --------------------------------------------- 2 X ? ---------------------------------------------
  else if (Touch_Input_Y > myTouch.Get_Top_Bar_Size() + Button_Spazing * 2 + myGLCD.Get_Button_Size_Y() && Touch_Input_Y < myTouch.Get_Top_Bar_Size() + Button_Spazing * 2 + myGLCD.Get_Button_Size_Y() * 2) {

    // --------------------------------------------- 2 X 1 ---------------------------------------------
    if (Touch_Input_X > Button_Spazing && Touch_Input_X < (Button_Spazing + myGLCD.Get_Button_Size_X())) {
      Main_Page_Ignore_Input_Until = millis() + Main_Page_Ignore_Input_For;
      Serial.println("2 X 1"); // REMOVE ME
    }

  }



} // END MARKER - Main_Page_Touch


void Relay_Page() {

  myGLCD.Set_Button_Size(200, 90);
  myGLCD.Set_Button_Edge_Size(4);
  myGLCD.Set_Button_Matrix_Spacing(50);

  myGLCD.Draw_Button_Matrix("Old Lights", 1, 1);
  myGLCD.Draw_Button_Matrix("Main Lights", 2, 1);
  myGLCD.Draw_Button_Matrix("Red Lights", 3, 1);

  myGLCD.Draw_Button_Matrix("USB Charger", 1, 2);
  myGLCD.Draw_Button_Matrix("Bilge pump", 2, 2);
  myGLCD.Draw_Button_Matrix("Gen / Inv", 3, 2);

  myGLCD.Draw_Button_Matrix("Back Lights", 1, 3);
  myGLCD.Draw_Button_Matrix("Internet", 2, 3);

} // End Marker - Main_Controles

void Relay_Page_Touch() {

} // END MARKER - Relay_Page_Touch


void Voltmeter_Page() {

  myGLCD.Set_Button_Size(200, 90);

  myGLCD.Draw_Button_Matrix("Voltmeter 1", 1, 1);

} // END MARKER - Main_Page_Touch

void Voltmeter_Page_Touch() {

} // END MARKER - Main_Page_Touch


void VLC_Interface() {

  // USE draw.pixl to make play buttons and so on


  #ifndef CENTER_Y
  #define CENTER_Y myGLCD.getDisplayYSize() / 2
  #endif

  // int xStart;
  // int yStart;


  //
  //
  // // --------------- Previous ---------------
  // xStart = 50;
  // yStart = CENTER_Y - myGLCD.Button_Size_Y() / 2;
  //
  // myGLCD.setColor(0, 50, 60);
  // myGLCD.fillRoundRect (xStart, yStart, xStart + myGLCD.Button_Size_X(), yStart + myGLCD.Button_Size_Y());
  //
  // myGLCD.setColor(255, 255, 255);
  // myGLCD.drawRoundRect (xStart, yStart, xStart + myGLCD.Button_Size_X(), yStart + myGLCD.Button_Size_Y());
  //
  // myGLCD.setBackColor(0, 50, 60);
  // myGLCD.print(String("Previous"), xStart + 30, yStart + 15);
  //
  //
  //
  //
  //
  //
  //
  //

  //
  //  // --------------- Play ---------------
  //  xStart = CENTER;
  //  yStart = CENTER - 15;
  //
  //
  //  myGLCD.setColor(0, 50, 60);
  //  myGLCD.fillRoundRect (xStart, yStart, xStart + myGLCD.Button_Size_X(), yStart + myGLCD.Button_Size_Y());
  //
  //  myGLCD.setColor(255, 255, 255);
  //  myGLCD.drawRoundRect (xStart, yStart, xStart + myGLCD.Button_Size_X(), yStart + myGLCD.Button_Size_Y());
  //
  //  myGLCD.setBackColor(0, 50, 60);
  //  myGLCD.print("Play", xStart + 30, yStart + 15);
  //
  //
  //


}

void VLC_Interface_Touch() {

}


void Top_Bar() {

  myGLCD.clrScr();

  myGLCD.Draw_Top_Bar();

       if (Page_Number == 1) Main_Page();
  else if (Page_Number == 2) Relay_Page();
  else if (Page_Number == 3) Voltmeter_Page();


} // END MARKER - Top_Bar

void Top_Bar_Touch() {

  if (!myTouch.dataAvailable() || Touch_Input_Y == -1) return;

  if (myTouch.Get_Top_Bar_Button_Number(Touch_Input_X, Touch_Input_Y) != 0) {

    if (myTouch.Get_Top_Bar_Button_Number() == 1) {
      myGLCD.Set_Top_Bar_Page_Number_Down();
      Serial.print("Top Bar - Page Down to: "); // REMOVE ME
    }

    else if (myTouch.Get_Top_Bar_Button_Number() == 2) {
      myGLCD.Set_Top_Bar_Page_Number_Up();
      Serial.print("Top Bar - Page Up to: "); // REMOVE ME
    }

    Serial.println(myGLCD.Get_Top_Bar_Page_Number()); // REMOVE ME
    Top_Bar();
  }

} // END MARKER - Top_Bar_Touch


void setup() {

  delay(2000);

  Serial.begin(115200);

  while (!Serial) {
    delay(100);
  }

  Serial.println("Booting");

  myGLCD.InitLCD();
  myGLCD.setFont(GroteskBold16x32);

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

  Display_Center_X = myGLCD.getDisplayXSize() / 2;
  Display_Center_Y = myGLCD.getDisplayYSize() / 2;

  myGLCD.fillScr(0xF800);
  myGLCD.setBackColor(0x0000);
  myGLCD.print(String("Booting"), CENTER, Display_Center_Y - 5);
  delay(500);

  // -------------------------- Touch --------------------------

  myTouch.Set_Stabilize_Input(250, 250);

  myTouch.getX_Flip_Output(800);
  myTouch.getY_Flip_Output(480);


  myGLCD.Center_Text(true);

  /*
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

  // -------------------------- Top Bar --------------------------

  myGLCD.Set_Top_Bar_Text("Main;Relay's;Voltmeter's;");

  myGLCD.Set_Top_Bar_Size(50);

  myGLCD.Set_Top_Bar_Color(0x001F);
  myGLCD.Set_Top_Bar_Edge_Color(0x9CF3);
  myGLCD.Set_Top_Bar_Text_Color(0xEF5D);


  myTouch.Set_Top_Bar_Size(myGLCD.Get_Top_Bar_Size());

  myTouch.Set_Top_Bar_Button_Size(125, myGLCD.getDisplayXSize());

  myTouch.Set_Top_Bar_Ignore_Input_For(1500);



  // -------------------------- Buttons --------------------------

  myGLCD.Set_Button_Color(0x001F);
  myGLCD.Set_Button_Back_Color(0x001F);

  myGLCD.Set_Button_Edge_Color(0x9CF3);
  myGLCD.Set_Button_Text_Color(0xEF5D);



  myGLCD.print(String("Boot Done"), CENTER, Display_Center_Y - 5);
  Serial.println("Boot Done");

  delay(500);

  Top_Bar();

}


void Touch_Check() {

  myTouch.read();

  Touch_Input_X = myTouch.getX();
  Touch_Input_Y = myTouch.getY();

  // if (myTouch.Stabilize_Input(Touch_Input_X, Touch_Input_Y) == 1)  return; // Touch input diviated to much returning

  Top_Bar_Touch();

  // --------------------------------------------- 1 - Main Page ---------------------------------------------
  if (Page_Number == 1) {
  // Main_Page_Touch();
  }

  // --------------------------------------------- 2 - Relay Page ---------------------------------------------
  else if (Page_Number == 2) { // CHANGE BELOW
  }
} // End Marker - Touch_Check



void loop() {

  Touch_Check();

} // END MARKER - Loop
