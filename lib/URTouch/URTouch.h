/*
  URTouch.h - Arduino/chipKit library support for Color TFT LCD Touch screens
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

#ifndef URTouch_h
#define URTouch_h

#define URTOUCH_VERSION	201

#if defined(__AVR__)
	#include "Arduino.h"
	#include "hardware/avr/HW_AVR_defines.h"
#elif defined(__PIC32MX__)
	#include "WProgram.h"
	#include "hardware/pic32/HW_PIC32_defines.h"
#elif defined(__arm__)
	#include "Arduino.h"
	#include "hardware/arm/HW_ARM_defines.h"
#endif

#define PORTRAIT			0
#define LANDSCAPE			1

#define PREC_LOW			1
#define PREC_MEDIUM			2
#define PREC_HI				3
#define PREC_EXTREME		4

class URTouch
{
	public:
		int16_t	TP_X ,TP_Y;

				URTouch(byte tclk, byte tcs, byte tdin, byte dout, byte irq);

		void	InitTouch(byte orientation = LANDSCAPE);
		void	read();
		bool	dataAvailable();
		int16_t	getX();
		int16_t	getY();
		void	setPrecision(byte precision);

		void	calibrateRead();

    private:
		regtype *P_CLK, *P_CS, *P_DIN, *P_DOUT, *P_IRQ;
		regsize B_CLK, B_CS, B_DIN, B_DOUT, B_IRQ;
		byte	T_CLK, T_CS, T_DIN, T_DOUT, T_IRQ;
		long	_default_orientation;
		byte	orient;
		byte	prec;
		byte	display_model;
		long	disp_x_size, disp_y_size, default_orientation;
		long	touch_x_left, touch_x_right, touch_y_top, touch_y_bottom;

		void	touch_WriteData(byte data);
		word	touch_ReadData();

		// --------------------------------------------------------------------------------------------
		// -------------------------------------- CanNet --------------------------------------
		// --------------------------------------------------------------------------------------------

		public:

				void Set_Stabilize_Input(int Max_Touch_Diaviation, int Max_Touch_Diaviation_Time);
				bool Stabilize_Input(int Touch_Input_X, int Touch_Input_Y);

				void getX_Flip_Output(int Display_Size_X);
				void getY_Flip_Output(int Display_Size_Y);

				byte Get_Button_Matrix_Number(bool X_Y, int Input, bool Use_Button_Size_2);
				byte Get_Button_Matrix_Number(bool X_Y, int Input);

				void Set_Button_Size(int Button_Size_X, int Button_Size_Y);
		    void Set_Button_Size_2(int Button_Size_X, int Button_Size_Y);

		    int Get_Button_Size_X();
		    int Get_Button_Size_Y();
		    int Get_Button_Size_2_X();
		    int Get_Button_Size_2_Y();

				void Set_Button_Matrix_Spacing(byte Spacing);
				byte Get_Button_Matrix_Spacing();

				// -------------------------- Top Bar --------------------------

				byte Get_Top_Bar_Button_Number(int Input_X, int Input_Y);
				void Set_Top_Bar_Button_Size(int Button_Size, int Display_Size_X);

				int Top_Bar_Size;
				int Top_Bar_Button_Size;
				int Top_Bar_Button_Pressed;
				int Top_Bar_Ignore_Input_For;

			private:

				int _X_Flip_Output = 0;
				int _Y_Flip_Output = 0;

				int _Button_Size;
				int _Button_Size_X;
		    int _Button_Size_Y;
		    int _Button_Size_2_X;
		    int _Button_Size_2_Y;

				byte _Button_Matrix_Number_X;
				byte _Button_Matrix_Number_Y;
				int _Button_Matrix_Size;
				int _Button_Matrix_Spacing;

				// -------------------------- Top Bar --------------------------

				int _Top_Bar_Button_Spaceing;

				unsigned long _Top_Bar_Ignore_Input_Untill;


				unsigned long _Touch_Input_Last_Input;
				int _Max_Touch_Diaviation = 250; // Max diviation from last touch input
				int _Max_Touch_Diaviation_Time = 250; // If the input is stable this is the delays between the inputs gets though
				int _Touch_Input_X_Check, _Touch_Input_Y_Check;


#if defined(ENERGIA)
		volatile uint32_t* portOutputRegister(int value);
		volatile uint32_t* portInputRegister(int value);
#endif
};

#endif
