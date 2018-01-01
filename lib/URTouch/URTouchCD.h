// URTouchCD.h
// ----------
//
// Since there are slight deviations in all touch screens you should run a
// calibration on your display module. Run the URTouch_Calibration sketch
// that came with this library and follow the on-screen instructions to
// update this file.
//
// Remember that is you have multiple display modules they will probably
// require different calibration data so you should run the calibration
// every time you switch to another module.
// You can, of course, store calibration data for all your modules here
// and comment out the ones you dont need at the moment.
//

// These calibration settings works with my ITDB02-3.2S.
// They MIGHT work on your 320x240 display module, but you should run the
// calibration sketch anyway. If you are using a display with any other
// resolution you MUST calibrate it as these settings WILL NOT work.
// #define CAL_X 0x00378F66UL
// #define CAL_Y 0x03C34155UL
// #define CAL_S 0x000EF13FUL

// Randum numbers of the internet
// #define CAL_X 0x00234F89UL
// #define CAL_Y 0x0066CEAFUL
// #define CAL_S 0x8031F1DFUL

// Calculated LANDSCAPE
#define CAL_X 0x03E6005DUL
#define CAL_Y 0x03C280D2UL
#define CAL_S 0x8031F1DFUL

/*

Its missing the

CAL_X: 0x03F14032UL
CAL_Y: 0x03DA0089UL
CAL_S: 0x8031F1DFUL



CAL_X: 0x03E6005DUL
CAL_Y: 0x03C280D2UL
CAL_S: 0x8031F1DFUL


CAL_X: 0x03E4C05FUL
CAL_Y: 0x03C140E0UL
CAL_S: 0x8031F1DFUL

CAL_X: 0x03E7C056UL
CAL_Y: 0x03C5C0D8UL
CAL_S: 0x8031F1DFUL

CAL_X: 0x03E84052UL
CAL_Y: 0x03C6C0DFUL
CAL_S: 0x8031F1DFUL


Best so far
#define CAL_X 0x03E6005DUL
#define CAL_Y 0x03C280D2UL
#define CAL_S 0x8031F1DFUL
*/
