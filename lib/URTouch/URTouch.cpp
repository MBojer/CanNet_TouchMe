/*
  URTouch.cpp - Arduino/chipKit library support for Color TFT LCD Touch screens
  Copyright (C)2016 Rinky-Dink Electronics, Henning Karlsen. All right reserved

  Basic functionality of this library are based on the demo-code provided by
  ITead studio.

  You can find the latest version of the library at
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the
  examples and tools supplied with the library.
*/

#include "URTouch.h"
#include "URTouchCD.h"

#if defined(__AVR__)
	#include "hardware/avr/HW_AVR.inc"
#elif defined(__PIC32MX__)
	#include "hardware/pic32/HW_PIC32.inc"
#elif defined(__arm__)
	#include "hardware/arm/HW_ARM.inc"
#endif

URTouch::URTouch(byte tclk, byte tcs, byte din, byte dout, byte irq)
{
	T_CLK	= tclk;
	T_CS	= tcs;
	T_DIN	= din;
	T_DOUT	= dout;
	T_IRQ	= irq;
}

void URTouch::InitTouch(byte orientation)
{
	orient					= orientation;
	_default_orientation	= CAL_S>>31;
	touch_x_left			= (CAL_X>>14) & 0x3FFF;
	touch_x_right			= CAL_X & 0x3FFF;
	touch_y_top				= (CAL_Y>>14) & 0x3FFF;
	touch_y_bottom			= CAL_Y & 0x3FFF;
	disp_x_size				= (CAL_S>>12) & 0x0FFF;
	disp_y_size				= CAL_S & 0x0FFF;
	prec					= 10;

	P_CLK	= portOutputRegister(digitalPinToPort(T_CLK));
	B_CLK	= digitalPinToBitMask(T_CLK);
	P_CS	= portOutputRegister(digitalPinToPort(T_CS));
	B_CS	= digitalPinToBitMask(T_CS);
	P_DIN	= portOutputRegister(digitalPinToPort(T_DIN));
	B_DIN	= digitalPinToBitMask(T_DIN);
	P_DOUT	= portInputRegister(digitalPinToPort(T_DOUT));
	B_DOUT	= digitalPinToBitMask(T_DOUT);
	P_IRQ	= portInputRegister(digitalPinToPort(T_IRQ));
	B_IRQ	= digitalPinToBitMask(T_IRQ);

	pinMode(T_CLK,  OUTPUT);
    pinMode(T_CS,   OUTPUT);
    pinMode(T_DIN,  OUTPUT);
    pinMode(T_DOUT, INPUT);
    pinMode(T_IRQ,  OUTPUT);

	sbi(P_CS, B_CS);
	sbi(P_CLK, B_CLK);
	sbi(P_DIN, B_DIN);
	sbi(P_IRQ, B_IRQ);
}

void URTouch::read()
{
	unsigned long tx=0, temp_x=0;
	unsigned long ty=0, temp_y=0;
	unsigned long minx=99999, maxx=0;
	unsigned long miny=99999, maxy=0;
	int datacount=0;

	cbi(P_CS, B_CS);

	pinMode(T_IRQ,  INPUT);
	for (int i=0; i<prec; i++)
	{
		if (!rbi(P_IRQ, B_IRQ))
		{
			touch_WriteData(0x90);
			pulse_high(P_CLK, B_CLK);
			temp_x=touch_ReadData();

			if (!rbi(P_IRQ, B_IRQ))
			{
				touch_WriteData(0xD0);
				pulse_high(P_CLK, B_CLK);
				temp_y=touch_ReadData();

				if ((temp_x>0) and (temp_x<4096) and (temp_y>0) and (temp_y<4096))
				{
					tx+=temp_x;
					ty+=temp_y;
					if (prec>5)
					{
						if (temp_x<minx)
							minx=temp_x;
						if (temp_x>maxx)
							maxx=temp_x;
						if (temp_y<miny)
							miny=temp_y;
						if (temp_y>maxy)
							maxy=temp_y;
					}
					datacount++;
				}
			}
		}
	}
	pinMode(T_IRQ,  OUTPUT);

	if (prec>5)
	{
		tx = tx-(minx+maxx);
		ty = ty-(miny+maxy);
		datacount -= 2;
	}

	sbi(P_CS, B_CS);
	if ((datacount==(prec-2)) or (datacount==PREC_LOW))
	{
		if (orient == _default_orientation)
		{
			TP_X=ty/datacount;
			TP_Y=tx/datacount;
		}
		else
		{
			TP_X=tx/datacount;
			TP_Y=ty/datacount;
		}
	}
	else
	{
		TP_X=-1;
		TP_Y=-1;
	}
}

bool URTouch::dataAvailable()
{
	bool avail;
	pinMode(T_IRQ,  INPUT);
	avail = !(rbi(P_IRQ, B_IRQ));
	pinMode(T_IRQ,  OUTPUT);
	return avail;
}

// int16_t URTouch::getX()
// {
// 	long c;
//
// 	if ((TP_X==-1) or (TP_Y==-1))
// 		return -1;
// 	if (orient == _default_orientation)
// 	{
// 		c = long(long(TP_X - touch_x_left) * (disp_x_size)) / long(touch_x_right - touch_x_left);
// 		if (c<0)
// 			c = 0;
// 		if (c>disp_x_size)
// 			c = disp_x_size;
// 	}
// 	else
// 	{
// 		if (_default_orientation == PORTRAIT)
// 			c = long(long(TP_X - touch_y_top) * (-disp_y_size)) / long(touch_y_bottom - touch_y_top) + long(disp_y_size);
// 		else
// 			c = long(long(TP_X - touch_y_top) * (disp_y_size)) / long(touch_y_bottom - touch_y_top);
// 		if (c<0)
// 			c = 0;
// 		if (c>disp_y_size)
// 			c = disp_y_size;
// 	}
// 	return _X_Flip_Output - c;
// }
//
// int16_t URTouch::getY()
// {
// 	int c;
//
// 	if ((TP_X==-1) or (TP_Y==-1))
// 		return -1;
// 	if (orient == _default_orientation)
// 	{
// 		c = long(long(TP_Y - touch_y_top) * (disp_y_size)) / long(touch_y_bottom - touch_y_top);
// 		if (c<0)
// 			c = 0;
// 		if (c>disp_y_size)
// 			c = disp_y_size;
// 	}
// 	else
// 	{
// 		if (_default_orientation == PORTRAIT)
// 			c = long(long(TP_Y - touch_x_left) * (disp_x_size)) / long(touch_x_right - touch_x_left);
// 		else
// 			c = long(long(TP_Y - touch_x_left) * (-disp_x_size)) / long(touch_x_right - touch_x_left) + long(disp_x_size);
// 		if (c<0)
// 			c = 0;
// 		if (c>disp_x_size)
// 			c = disp_x_size;
// 	}
// 	return _Y_Flip_Output - c;
// }

void URTouch::setPrecision(byte precision)
{
	switch (precision)
	{
		case PREC_LOW:
			prec=1;		// DO NOT CHANGE!
			break;
		case PREC_MEDIUM:
			prec=12;	// Iterations + 2
			break;
		case PREC_HI:
			prec=27;	// Iterations + 2
			break;
		case PREC_EXTREME:
			prec=102;	// Iterations + 2
			break;
		default:
			prec=12;	// Iterations + 2
			break;
	}
}

void URTouch::calibrateRead()
{
	unsigned long tx=0;
	unsigned long ty=0;

	cbi(P_CS, B_CS);

	touch_WriteData(0x90);
	pulse_high(P_CLK, B_CLK);
	tx=touch_ReadData();

	touch_WriteData(0xD0);
	pulse_high(P_CLK, B_CLK);
	ty=touch_ReadData();

	sbi(P_CS, B_CS);

	TP_X=ty;
	TP_Y=tx;
}

// --------------------------------------------------------------------------------------------
// -------------------------------------- CanNet --------------------------------------
// --------------------------------------------------------------------------------------------



int16_t URTouch::getX()
{
	long c;

	if ((TP_X==-1) or (TP_Y==-1))
		return -1;
	if (orient == _default_orientation)
	{
		c = long(long(TP_X - touch_x_left) * (disp_x_size)) / long(touch_x_right - touch_x_left);
		if (c<0)
			c = 0;
		if (c>disp_x_size)
			c = disp_x_size;
	}
	else
	{
		if (_default_orientation == PORTRAIT)
			c = long(long(TP_X - touch_y_top) * (-disp_y_size)) / long(touch_y_bottom - touch_y_top) + long(disp_y_size);
		else
			c = long(long(TP_X - touch_y_top) * (disp_y_size)) / long(touch_y_bottom - touch_y_top);
		if (c<0)
			c = 0;
		if (c>disp_y_size)
			c = disp_y_size;
	}
	return Flip_Output_X - c;
}

int16_t URTouch::getY()
{
	int c;

	if ((TP_X==-1) or (TP_Y==-1))
		return -1;
	if (orient == _default_orientation)
	{
		c = long(long(TP_Y - touch_y_top) * (disp_y_size)) / long(touch_y_bottom - touch_y_top);
		if (c<0)
			c = 0;
		if (c>disp_y_size)
			c = disp_y_size;
	}
	else
	{
		if (_default_orientation == PORTRAIT)
			c = long(long(TP_Y - touch_x_left) * (disp_x_size)) / long(touch_x_right - touch_x_left);
		else
			c = long(long(TP_Y - touch_x_left) * (-disp_x_size)) / long(touch_x_right - touch_x_left) + long(disp_x_size);
		if (c<0)
			c = 0;
		if (c>disp_x_size)
			c = disp_x_size;
	}
	return Flip_Output_Y - c;
}


void URTouch::Set_Stabilize_Input(int Max_Touch_Diaviation, int Max_Touch_Diaviation_Time) {

	_Max_Touch_Diaviation = Max_Touch_Diaviation;
	_Max_Touch_Diaviation_Time = Max_Touch_Diaviation_Time;

}

bool URTouch::Stabilize_Input(int Touch_Input_X, int Touch_Input_Y) {
	// Checks if the touch input doviats to much and returns if it does
  // Done to try to elimenate false hits
  if (_Touch_Input_Last_Input + _Max_Touch_Diaviation_Time > millis()) {

    if (
        Touch_Input_X - _Touch_Input_X_Check > _Max_Touch_Diaviation ||
				(Touch_Input_X - _Touch_Input_X_Check) * -1 > _Max_Touch_Diaviation ||

        Touch_Input_Y - _Touch_Input_Y_Check > _Max_Touch_Diaviation ||
        (Touch_Input_Y - _Touch_Input_Y_Check) * -1 > _Max_Touch_Diaviation
    ) {

			if (_Touch_Input_X_Check == 0 && _Touch_Input_Y_Check == 0 && _Touch_Input_Last_Input == 0) // If all is 0 assuming the first check have not been mad a returning false
				return false;

			Serial.println("HIT: Stabilize_Input"); // REMOVE ME
			return true;
    }


  }

  _Touch_Input_X_Check = Touch_Input_X;
  _Touch_Input_Y_Check = Touch_Input_Y;
  _Touch_Input_Last_Input = millis();
	return false;
}



byte URTouch::Get_Button_Matrix_Number(bool X_Y, int Input, bool Use_Button_Size_2) {

	if (X_Y == false) { // false = X

		if (Input - Button_Matrix_Spacing < 0) { // Input before first button
			return 0;
		}

		if (Use_Button_Size_2 == false) { // Button_Size
			Button_Size = Button_Size_X;
		} // END MARKER - if (Use_Button_Size_2 == false)

		else { // Button_Size
			Button_Size = Button_Size_2_X;
		} // END MARKER - else

	} // END MARKER - if (X_Y == false)


	else { // true = Y
		if (Input - Top_Bar_Size - Button_Matrix_Spacing < 0) { // Input before first button
			return 0;
		}

		if (Use_Button_Size_2 == false) { // Button_Size
			Button_Size = Button_Size_Y;
		} // END MARKER - if (Use_Button_Size_2 == false)

		else { // Button_Size
			Button_Size = Button_Size_2_Y;
		} // END MARKER - else

	} // END MARKER - else


	for (int x = 1; x < 100; x++) {

		int Temp_Int = Input - Button_Matrix_Spacing * x;

		if (x != 1) {
			Temp_Int = Temp_Int - Button_Size * (x - 1);
		}

		if (X_Y == true) { // true = Y

			Temp_Int = Temp_Int - Top_Bar_Size;

		} // END MARKER - if (X_Y == true)

		if (Temp_Int < 0) {
			Serial.println("MARKER"); // REMOVE ME
			Serial.println(x); // REMOVE ME
			return 0;
		}

		if (Temp_Int < Button_Size && Temp_Int > 0) {
			return x;
		}

	} // END MARKER - for "loop"

	return 0;
}  // End Marker - Get_Button_Matrix_Number

byte URTouch::Get_Button_Matrix_Number(bool X_Y, int Input) {

	return Get_Button_Matrix_Number(X_Y, Input, false);

}  // End Marker - Get_Button_Matrix_Number



byte URTouch::Get_Top_Bar_Button_Number(int Input_X, int Input_Y) {

	if (Input_Y > Top_Bar_Size) { // Input not matching top bar
		Top_Bar_Button_Pressed = 0;
	}

	else if (_Top_Bar_Ignore_Input_Untill > millis()) { // Pressed to soon ignoreing input
		Top_Bar_Button_Pressed = 0;
	}

	else if (Input_Y > 0 && Input_Y < Top_Bar_Size) { // Y - Input matching top bar


		if (Input_X > 0 && Input_X < Top_Bar_Button_Size) { // X - Page Down
			_Top_Bar_Ignore_Input_Untill = millis() + Top_Bar_Ignore_Input_For;
			Top_Bar_Button_Pressed = 1;
		}

		else if (
							Input_X > Top_Bar_Button_Size + _Top_Bar_Button_Spaceing &&
							Input_X < Top_Bar_Button_Size + _Top_Bar_Button_Spaceing + Top_Bar_Button_Size
						) { // X - Page Up

			_Top_Bar_Ignore_Input_Untill = millis() + Top_Bar_Ignore_Input_For;
			Top_Bar_Button_Pressed = 2;
		}

	} // END MARKER - else if (Input_Y > 0 && Input_Y < _Top_Bar_Size)

	return Top_Bar_Button_Pressed;

}  // End Marker - Get_Button_Matrix_Number

void URTouch::Set_Top_Bar_Button_Size(int Button_Size, int Display_Size_X) {

	Top_Bar_Button_Size = Button_Size;
	_Top_Bar_Button_Spaceing = Display_Size_X - Top_Bar_Button_Size * 2;

}  // End Marker - Set_Top_Bar_Button_Size
