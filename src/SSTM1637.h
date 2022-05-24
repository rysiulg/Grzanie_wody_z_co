
/*
  SevenSegmentTM1637 - class to control a 4 digit seven segment display with a TM1636 or TM1637 driver IC
  Created by Bram Harmsen, September 25, 2015
  Released into the public domain.
  Licence: GNU GENERAL PUBLIC LICENSE V2.0

  # Changelog

  v1.0  25-10-2015
  v1.1  04-07-2020

*/

#ifndef SevenSegmentTM1637_H
#define SevenSegmentTM1637_H

#include <Arduino.h>
//#include "SevenSegmentAsciiMap.h"


#ifndef SevenSegmentAsciiMap_H
#define SevenSegmentAsciiMap_H

#include <Arduino.h>

// ASCII MAPPINGS
#define TM1637_CHAR_SPACE       B00000000 // 32  (ASCII)
#define TM1637_CHAR_EXC         B00000110
#define TM1637_CHAR_D_QUOTE     B00100010
#define TM1637_CHAR_POUND       B01110110
#define TM1637_CHAR_DOLLAR      B01101101
#define TM1637_CHAR_PERC        B00100100
#define TM1637_CHAR_AMP         B01111111
#define TM1637_CHAR_S_QUOTE     B00100000
#define TM1637_CHAR_L_BRACKET   B00111001
#define TM1637_CHAR_R_BRACKET   B00001111
#define TM1637_CHAR_STAR        B01011100
#define TM1637_CHAR_PLUS        B01010000
#define TM1637_CHAR_COMMA       B00010000
#define TM1637_CHAR_MIN         B01000000
#define TM1637_CHAR_DOT         B00001000
#define TM1637_CHAR_F_SLASH     B00000110
#define TM1637_CHAR_0           B00111111   // 48
#define TM1637_CHAR_1           B00000110
#define TM1637_CHAR_2           B01011011
#define TM1637_CHAR_3           B01001111
#define TM1637_CHAR_4           B01100110
#define TM1637_CHAR_5           B01101101
#define TM1637_CHAR_6           B01111101
#define TM1637_CHAR_7           B00000111
#define TM1637_CHAR_8           B01111111
#define TM1637_CHAR_9           B01101111
#define TM1637_CHAR_COLON       B00110000
#define TM1637_CHAR_S_COLON     B00110000
#define TM1637_CHAR_LESS        B01011000
#define TM1637_CHAR_EQUAL       B01001000
#define TM1637_CHAR_GREAT       B01001100
#define TM1637_CHAR_QUEST       B01010011
#define TM1637_CHAR_AT          B01011111
#define TM1637_CHAR_A           B01110111 // 65  (ASCII)
#define TM1637_CHAR_B           B01111111
#define TM1637_CHAR_C           B00111001
#define TM1637_CHAR_D           TM1637_CHAR_d
#define TM1637_CHAR_E           B01111001
#define TM1637_CHAR_F           B01110001
#define TM1637_CHAR_G           B00111101
#define TM1637_CHAR_H           B01110110
#define TM1637_CHAR_I           B00000110
#define TM1637_CHAR_J           B00001110
#define TM1637_CHAR_K           B01110101
#define TM1637_CHAR_L           B00111000
#define TM1637_CHAR_M           B00010101
#define TM1637_CHAR_N           B00110111
#define TM1637_CHAR_O           B00111111
#define TM1637_CHAR_P           B01110011
#define TM1637_CHAR_Q           B01100111
#define TM1637_CHAR_R           B00110011
#define TM1637_CHAR_S           B01101101
#define TM1637_CHAR_T           TM1637_CHAR_t
#define TM1637_CHAR_U           B00111110
#define TM1637_CHAR_V           B00011100
#define TM1637_CHAR_W           B00101010
#define TM1637_CHAR_X           TM1637_CHAR_H
#define TM1637_CHAR_Y           B01101110
#define TM1637_CHAR_Z           B01011011
#define TM1637_CHAR_L_S_BRACKET B00111001 // 91 (ASCII)
#define TM1637_CHAR_B_SLASH     B00110000
#define TM1637_CHAR_R_S_BRACKET B00001111
#define TM1637_CHAR_A_CIRCUM    B00010011
#define TM1637_CHAR_UNDERSCORE  B00001000
#define TM1637_CHAR_A_GRAVE     B00010000
#define TM1637_CHAR_a           B01011111 // 97 (ASCII)
#define TM1637_CHAR_b           B01111100
#define TM1637_CHAR_c           B01011000
#define TM1637_CHAR_d           B01011110
#define TM1637_CHAR_e           B01111011
#define TM1637_CHAR_f           TM1637_CHAR_F
#define TM1637_CHAR_g           B01101111
#define TM1637_CHAR_h           B01110100
#define TM1637_CHAR_i           B00000100
#define TM1637_CHAR_j           B00001100
#define TM1637_CHAR_k           TM1637_CHAR_K
#define TM1637_CHAR_l           B00110000
#define TM1637_CHAR_m           TM1637_CHAR_M
#define TM1637_CHAR_n           B01010100
#define TM1637_CHAR_o           B01011100
#define TM1637_CHAR_p           TM1637_CHAR_P
#define TM1637_CHAR_q           TM1637_CHAR_Q
#define TM1637_CHAR_r           B01010000
#define TM1637_CHAR_s           TM1637_CHAR_S
#define TM1637_CHAR_t           B01111000
#define TM1637_CHAR_u           B00011100
#define TM1637_CHAR_v           B00011100
#define TM1637_CHAR_w           TM1637_CHAR_W
#define TM1637_CHAR_x           TM1637_CHAR_X
#define TM1637_CHAR_y           B01100110
#define TM1637_CHAR_z           TM1637_CHAR_Z
#define TM1637_CHAR_L_ACCON     B01111001 // 123 (ASCII)
#define TM1637_CHAR_BAR         B00000110
#define TM1637_CHAR_R_ACCON     B01001111
#define TM1637_CHAR_TILDE       B01000000 // 126 (ASCII)

class AsciiMap {
public:
    const static uint8_t map[96];
};
// static const uint8_t asciiMap[96];

#endif



// store an ASCII Map in PROGMEM (Flash memory)
const uint8_t AsciiMap::map[] PROGMEM = {
  TM1637_CHAR_SPACE,
  TM1637_CHAR_EXC,
  TM1637_CHAR_D_QUOTE,
  TM1637_CHAR_POUND,
  TM1637_CHAR_DOLLAR,
  TM1637_CHAR_PERC,
  TM1637_CHAR_AMP,
  TM1637_CHAR_S_QUOTE,
  TM1637_CHAR_L_BRACKET,
  TM1637_CHAR_R_BRACKET,
  TM1637_CHAR_STAR,
  TM1637_CHAR_PLUS,
  TM1637_CHAR_COMMA,
  TM1637_CHAR_MIN,
  TM1637_CHAR_DOT,
  TM1637_CHAR_F_SLASH,
  TM1637_CHAR_0,          // 48 (ASCII)
  TM1637_CHAR_1,
  TM1637_CHAR_2,
  TM1637_CHAR_3,
  TM1637_CHAR_4,
  TM1637_CHAR_5,
  TM1637_CHAR_6,
  TM1637_CHAR_7,
  TM1637_CHAR_8,
  TM1637_CHAR_9,
  TM1637_CHAR_COLON,
  TM1637_CHAR_S_COLON,
  TM1637_CHAR_LESS,
  TM1637_CHAR_EQUAL,
  TM1637_CHAR_GREAT,
  TM1637_CHAR_QUEST,
  TM1637_CHAR_AT,
  TM1637_CHAR_A,          // 65 (ASCII)
  TM1637_CHAR_B,
  TM1637_CHAR_C,
  TM1637_CHAR_D,
  TM1637_CHAR_E,
  TM1637_CHAR_F,
  TM1637_CHAR_G,
  TM1637_CHAR_H,
  TM1637_CHAR_I,
  TM1637_CHAR_J,
  TM1637_CHAR_K,
  TM1637_CHAR_L,
  TM1637_CHAR_M,
  TM1637_CHAR_N,
  TM1637_CHAR_O,
  TM1637_CHAR_P,
  TM1637_CHAR_Q,
  TM1637_CHAR_R,
  TM1637_CHAR_S,
  TM1637_CHAR_T,
  TM1637_CHAR_U,
  TM1637_CHAR_V,
  TM1637_CHAR_W,
  TM1637_CHAR_X,
  TM1637_CHAR_Y,
  TM1637_CHAR_Z,
  TM1637_CHAR_L_S_BRACKET,  // 91 (ASCII)
  TM1637_CHAR_B_SLASH,
  TM1637_CHAR_R_S_BRACKET,
  TM1637_CHAR_A_CIRCUM,
  TM1637_CHAR_UNDERSCORE,
  TM1637_CHAR_A_GRAVE,       // 96 (ASCII)
  TM1637_CHAR_a,
  TM1637_CHAR_b,
  TM1637_CHAR_c,
  TM1637_CHAR_d,
  TM1637_CHAR_e,
  TM1637_CHAR_f,
  TM1637_CHAR_g,
  TM1637_CHAR_h,
  TM1637_CHAR_i,
  TM1637_CHAR_j,
  TM1637_CHAR_k,
  TM1637_CHAR_l,
  TM1637_CHAR_m,
  TM1637_CHAR_n,
  TM1637_CHAR_o,
  TM1637_CHAR_p,
  TM1637_CHAR_q,
  TM1637_CHAR_r,
  TM1637_CHAR_s,
  TM1637_CHAR_t,
  TM1637_CHAR_u,
  TM1637_CHAR_v,
  TM1637_CHAR_w,
  TM1637_CHAR_x,
  TM1637_CHAR_y,
  TM1637_CHAR_z,
  TM1637_CHAR_L_ACCON,  // 123 (ASCII)
  TM1637_CHAR_BAR,
  TM1637_CHAR_R_ACCON,
  TM1637_CHAR_TILDE     // 126 (ASCII)
};




// COMPILE TIME USER CONFIG ////////////////////////////////////////////////////
#define TM1637_DEBUG                  false   // true for serial debugging
#define TM1637_BEGIN_DELAY            500     // ms
#define TM1637_PRINT_BUFFER_SIZE      128     // lower if you don't need it

// Default values //////////////////////////////////////////////////////////////
#define TM1637_DEFAULT_PRINT_DELAY    300     // 300 ms delay between characters
#define TM1637_DEFAULT_BLINK_DELAY    50      // ms
#define TM1637_DEFAULT_CLOCK_BLINK_DELAY 500  // the default delay for when using printTime
#define TM1637_DEFAULT_BLINK_REPEAT   10
#define TM1637_DEFAULT_CURSOR_POS     0       // 0-MAX-1 (e.g 3)
#define TM1637_DEFAULT_COLON_ON       false   //
#define TM1637_DEFAULT_BACKLIGHT      100     // 0..100

#define TM1637_MAX_LINES    1                 // number of display lines
#define TM1637_MAX_COLOM    4                 // number of coloms (digits)
#define TM1637_MAX_CHARS    128

// PROGRAM CONFIG (ONLY CHANGE WHEN YOU KNOW WHAT YOU RE DOING:)////////////////
#define TM1637_CLK_DELAY_US 5           // clock delay for communication
// mine works with 1us, perhaps increase if display does not function ( tested upto 1ms)


// COMMANDS ////////////////////////////////////////////////////////////////////
#define TM1637_COM_SET_DATA     B01000000 // 0x40 (1) Data set
#define TM1637_COM_SET_ADR      B11000000 // 0xC0 (2) Address command set
#define TM1637_COM_SET_DISPLAY  B10000000 // 0x80 (3) Display control command set

// Data set (1) (use logical OR to contruct complete command)
#define TM1637_SET_DATA_WRITE   B00000000 // Write data to the display register
#define TM1637_SET_DATA_READ    B00000010 // Read the key scan data
#define TM1637_SET_DATA_A_ADDR  B00000000 // Automatic address increment
#define TM1637_SET_DATA_F_ADDR  B00000100 // Fixed address
#define TM1637_SET_DATA_M_NORM  B00000000 // Normal mode
#define TM1637_SET_DATA_M_TEST  B00100000 // Test mode

// Address settings (2) (use logical OR to contruct complete command)
#define TM1637_SET_ADR_00H      B0000000  // addr 00
#define TM1637_SET_ADR_01H      B0000001  // addr 01
#define TM1637_SET_ADR_02H      B0000010  // addr 02
#define TM1637_SET_ADR_03H      B0000011  // addr 03
#define TM1637_SET_ADR_04H      B0000100  // addr 04 (only TM1637)
#define TM1637_SET_ADR_05H      B0000101  // addr 05 (only TM1637)
// The command is used to set the display register address; if the address is set to 0C4H or higher, the data is ignored, until the effective address is set; when the power is on, the default is set to 00H address.

// Display control command set (use logical OR to consruct complete command)
#define TM1637_SET_DISPLAY_1    B0000000  // Pulse width 1/16 (0.0625) (0)
#define TM1637_SET_DISPLAY_2    B0000001  // Pulse width 2/16 (0.0625) (1)
#define TM1637_SET_DISPLAY_4    B0000010  // Pulse width 4/16 (0.0625) (2)
#define TM1637_SET_DISPLAY_10   B0000011  // Pulse width 10/16 (0.0625) (3)
#define TM1637_SET_DISPLAY_11   B0000100  // Pulse width 11/16 (0.0625) (4)
#define TM1637_SET_DISPLAY_12   B0000101  // Pulse width 12/16 (0.0625) (5)
#define TM1637_SET_DISPLAY_13   B0000110  // Pulse width 13/16 (0.0625) (6)
#define TM1637_SET_DISPLAY_14   B0000111  // Pulse width 14/16 (0.0625) (7)
#define TM1637_SET_DISPLAY_OFF  B0000000  // OFF
#define TM1637_SET_DISPLAY_ON   B0001000  // ON
// there are a total of 8 brighness values, plus off

// PROTOCOL SPECIFICATION
/*
* Structure
* START COMMAND ACK STOP                  set config or display
* START ADR_CMD DATA ACK STOP             sets single digit
* START ADR_CMD DATA0 .. DATAN ACK STOP   sets multiple digits when in auto mode
*
* There are basicly three things you can do:
*   1. Set some configuration values
*     - read/write mode, auto/manual address, normal/test mode
*   2. Set a (starting) address followed by 1 or N data bytes
*   3. Set the display brightness (pwm) 0-7 and on or off
*
* From the datasheet it might seem that you always have to perform all three commands; setting configuration, setting address and data bytes and display. I'v tested this and this is not true. You can just set only one of these three. But ofcourse you have to make sure that your configuration is set properly. For example if you haven't set the configuration to automatic addresses, you can't just send out 4 data bytes, it won't work. Simlilair, if your display is off and you write some data to it, it won't display. On the other hand most default setting are what you want most of the time.

*/

class SevenSegmentTM1637 : public Print {

public:
  // LIQUID CRISTAL API ///////////////////////////////////////////////////////
  // See http://playground.arduino.cc/Code/LCDAPI for more details.

  /* Constructor
  @param [in] pinClk      clock pin (any digital pin)
  @param [in] pinDIO      digital output pin (any digital pin)
  */
  SevenSegmentTM1637(uint8_t pinClk, uint8_t pinDIO);
  /* Initializes the display
  * Initializes the display, sets some text and blinks the display

  @param [in] cols      optional: number of coloms (digits)
  @param [in] rows      optional: number of rows
  */
  void    init(uint8_t cols = TM1637_MAX_COLOM, uint8_t rows = TM1637_MAX_LINES);
  /* Implemented for compatibility, see begin() above */
  void    begin(uint8_t cols = TM1637_MAX_COLOM, uint8_t rows = TM1637_MAX_LINES);
  // Print class inheritance ///////////////////////////////////////////////////
  // See https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/Print.h for more details
  /* This library inherent the Print class, this means that all regular print function can be used. For example:
  * printing a number:                    print(78)
  * printint a number in BIN, OCT, HEX..: print(78, BIN)
  * printing a float:                     print(2.85)
  * printing a PROGMEM c string:          print(F("Arduino"))
  *
  * Also the more low level write() function can be used. (Actually all print function eventually call one of these write methods, every class that wants to inherent from the Print class needs to implement these)
  */
  size_t write(uint8_t byte);
  size_t write(const char* str);
  size_t write(const uint8_t* buffer, size_t size);

  /* Clears the display
  * Writes zero to all digits and segments, display off.
  */
  void    clear(void);
  /* Sets the cursor position to zero
  */
  void    home(void);
  /* Sets the cursor position to a specfic position
  *
  @param [in] col         colomn (position)
  */
  void    setCursor(uint8_t row, uint8_t col);
  // Liquid cristal optional //////////////////////////////////////////////////
  /* Sets the display backlight
  * The display has 8 PWM modes and an off mode. The function accepts a value from 0 to 100, where 80-100 are the same; full brighness.

  @param [in] value       brightness value (0..80(100))
  */
  void    setBacklight(uint8_t value);
  /* Sets the display contrast (identical to brightness)
  * This function is mainly for compatibility with the LCD API
  */
  void    setContrast(uint8_t value);

  /* Turns the display ON
  * Identical to setting the brightness to the default value.
  */
  void    on(void);
  /* Turns the display ON
  * Identical to setting the brightness to zero and clearing the display.
  */
  void    off(void);

  // SevenSegmentTM1637 METHODS ///////////////////////////////////////////////
  /* Blink the last printed text
  *
  @param [in] blinkDelay    optional: blink delay in ms
  @param [in] repeats       optional: number of blink repeats
  */
  void    blink(uint8_t blinkDelay = TM1637_DEFAULT_BLINK_DELAY, uint8_t repeats = TM1637_DEFAULT_BLINK_REPEAT, uint8_t maxBacklight=100, uint8_t minBacklight=0);

  // getters and setters ///////////////////////////////////////////////////////
  /* Turn the colon on or off
  * When turing the colon on, the next displayed text/numbers will have a colon
  @param [in] setToOn       sets the colon to on or off
  */
  void    setColonOn(bool setToOn);
  /* Get the currrent colon setting
  */
  bool    getColonOn(void);
  /* Sets the delay for scrolling text
  * When printing more than four characters/ the display will scroll, this setting determines the scrolling speed in ms
  @param [in] printDelay    the print delay in ms
  */
  void    setPrintDelay(uint16_t printDelay);

  // helpers //////////////////////////////////////////////////////////////////
  /* Encodes a character to sevensegment binairy
  *
  @param [in] c             a character to encode
  */
  uint8_t encode(char c);
  /* Encodes a single digit to sevensegment binairy
  *
  @param [in] d             a digit to encode
  */
  uint8_t encode(int16_t d);
  /* Encodes a null terminated c string (char array) to sevensegment binairy
  *
  @param [out] buffer       holds the encodes char array
  @param [in] str           the c string to encode
  @param [in] bufferSize    the size/length of the buffer
  */
  size_t  encode(uint8_t* buffer, const char* str, size_t bufferSize);
  /* Encodes a byte array to sevensegment binairy
  *
  @param [out] buffer       holds the encodes char array
  @param [in] byteArr       the byte array to encode
  @param [in] bufferSize    the size/length of the buffer
  */
  size_t  encode(uint8_t* buffer, const uint8_t* byteArr, size_t arrSize);
  /* Shift an array one position to the left
  @param [out] buffer       the buffer to be shifted
  @param [in] length        the length to the buffer
  */
  void    shiftLeft(uint8_t* buffer, size_t length);

  // SevenSegmentTM1637 low level methods (use when you know what you're doing)
  /* Prints raw (encoded) bytes to the display
  *         A
  *       ___
  *  * F |   | B
  * X     -G-
  *  * E |   | C
  *       ___
  *        D
  * Bit:      76543210
  * Segment:  XGFEDCBA
  *
  * For example to print an H, you would set bits BCEFG, this gives B01110110 in binary or 118 in decimal or 0x76 in HEX.
  * Bit 7 (X) only applies to the second digit and sets the colon
  */
  /* Print raw (binary encodes) bytes to the display
  @param [in] rawBytes      Array of raw bytes
  @param [in] length        optional: length to print to display
  @param [in] position      optional: Start position
  */
  void    printRaw(const uint8_t* rawBytes, size_t length = 4, uint8_t position = 0);
  /* Print raw (binary encodes) bytes to the display
  @param [in] rawByte       Raw byte
  @param [in] position      optional: Start position
  */
  void    printRaw(uint8_t rawByte, uint8_t position);
  /* Write command to IC TM1637
  @param [in] cmd         command to send
  @return acknowledged?   command was (successful) acknowledged
  */
  bool    command(uint8_t cmd) const;
  bool    command(const uint8_t* command, uint8_t length) const;
  /* Read bytes from IC TM1637
  * The IC also can read the state of a keypad? TODO untested
  */
  uint8_t comReadByte(void) const;
  /* Write a single command to the display
  @param [in] cmd         command to send
  */
  void    comWriteByte(uint8_t command) const;
  /* Send start signal
  * Send the start signal for serial communication
  */
  void    comStart(void) const;
  /* Send stop signal
  * Send the stop signal for serial communication
  */
  void    comStop(void) const;
  /* Get command acknowledged
  * Get acknowledge signal (command was succesful received)
  */
  bool    comAck(void) const;

  /* Static version of low level function
  * If using more than one display, this saves some space since these methods will be shared among all instances/objects of the class
  */
  static bool    command(uint8_t pinClk, uint8_t pinDIO, uint8_t cmd);
  static bool    command(uint8_t pinClk, uint8_t pinDIO, const uint8_t* command, uint8_t length);
  static void    comStart(uint8_t pinClk, uint8_t pinDIO);
  static void    comWriteByte(uint8_t pinClk, uint8_t pinDIO, uint8_t command);
  static bool    comAck(uint8_t pinClk, uint8_t pinDIO);
  static void    comStop(uint8_t pinClk, uint8_t pinDIO);
protected:
  const uint8_t   _pinClk;            // clock pin
  const uint8_t   _pinDIO;            // digital out pin
  uint8_t         _numCols;           // number of columns
  uint8_t         _numRows;           // number of rows

  uint8_t   _backLightValue;          // brightness of the display (0..100)
  uint8_t   _cursorPos;               // current cursor position
  uint16_t  _printDelay;              // print delay in ms (multiple chars)
  uint8_t   _colonOn;                 // colon bit if set
  uint8_t   _rawBuffer[TM1637_MAX_COLOM];// hold the last chars printed to display
};


#define TM1637_COLON_BIT        B10000000

// debug macros for debugging
#if TM1637_DEBUG
    #define TM1637_DEBUG_BEGIN(x)      Serial.begin(x)
    #define TM1637_DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
    #define TM1637_DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
    #define TM1637_DEBUG_WRITE(x)      Serial.write(x)
    #define TM1637_DEBUG_MESSAGE(...)    \
      Serial.print(millis());   \
      Serial.print(F("\t"));    \
      Serial.print(__VA_ARGS__);
    #define TM1637_DEBUG_MESSAGELN(...)  \
      TM1637_DEBUG_MESSAGE(__VA_ARGS__)  \
      Serial.println();
#else
    #define TM1637_DEBUG_BEGIN(x)
    #define TM1637_DEBUG_PRINT(...)
    #define TM1637_DEBUG_PRINTLN(...)
    #define TM1637_DEBUG_WRITE(x)
    #define TM1637_DEBUG_MESSAGE(x)
    #define TM1637_DEBUG_MESSAGELN(x)
#endif

// arduino:standard variant direct port access macros for more speed ( communication is ~us)
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8A__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega168PA__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
  #define portOfPin(P) \
    ( ((P) >= 0 && (P) < 8)? &PORTD:( ((P) > 7 && (P) < 14) ? &PORTB: &PORTC ) )
  #define ddrOfPin(P) \
    ( ((P) >= 0 && (P) < 8)? &DDRD: ( ((P) > 7 && (P) < 14) ? &DDRB: &DDRC ) )
  #define pinOfPin(P) \
    ( ((P) >= 0 && (P) < 8)? &PIND: ( ((P) > 7 && (P) < 14) ? &PINB: &PINC ) )

  #define pinIndex(P)         ( (uint8_t)( P > 13 ? P-14: P&7 ) )
  #define pinMask(P)          ( (uint8_t)( 1 << pinIndex(P) ) )

  #define pinAsInput(P)       *(ddrOfPin(P) )     &= ~pinMask(P)
  #define pinAsInputPullUp(P) *(ddrOfPin(P) )     &= ~pinMask(P);digitalHigh(P)
  #define pinAsOutput(P)      *(ddrOfPin(P) )     |= pinMask(P)
  #define digitalLow(P)       *(portOfPin(P))     &= ~pinMask(P)
  #define digitalHigh(P)      *(portOfPin(P))     |= pinMask(P)
  #define isHigh(P)           ( ( *( pinOfPin(P) )  & pinMask(P) ) >  0 )
  #define isLow(P)            ( ( *( pinOfPin(P) )  & pinMask(P) ) == 0 )
  #define digitalState(P)     ((uint8_t)isHigh(P))
#else
  #define pinAsOutput(P)      pinMode(P, OUTPUT)
  #define pinAsInput(P)       pinMode(P, INPUT)
  #define pinAsInputPullUp(P) pinMode(P, INPUT_PULLUP)
  #define digitalLow(P)       digitalWrite(P, LOW)
  #define digitalHigh(P)      digitalWrite(P, HIGH)
  #define isHigh(P)           (digitalRead(P) == 1)
  #define isLow(P)            (digitalRead(P) == 0)
  #define digitalState(P)     digitalRead(P)
#endif

#endif





SevenSegmentTM1637::SevenSegmentTM1637(uint8_t pinClk, uint8_t pinDIO) :
  _pinClk(pinClk),
  _pinDIO(pinDIO)
{
  // setup pins
  pinAsOutput(_pinClk);
  pinAsOutput(_pinDIO);
  digitalHigh(_pinClk);
  digitalHigh(_pinDIO);

  // setup defaults
  setCursor(0, TM1637_DEFAULT_CURSOR_POS);
  setPrintDelay(TM1637_DEFAULT_PRINT_DELAY);
  setColonOn(TM1637_DEFAULT_COLON_ON);
  setBacklight(TM1637_DEFAULT_BACKLIGHT);

  // write command SET_DATA   (Command1) Defaults
  command(
    TM1637_COM_SET_DATA |
    TM1637_SET_DATA_WRITE |
    TM1637_SET_DATA_A_ADDR |
    TM1637_SET_DATA_M_NORM
  );



};

// Print API ///////////////////////////////////////////////////////////////////
// single byte
size_t  SevenSegmentTM1637::write(uint8_t byte) {
  TM1637_DEBUG_PRINT(F("write byte:\t")); TM1637_DEBUG_PRINTLN((char)byte);

  size_t n = 0;
  if ( _cursorPos == _numCols ) {
    shiftLeft(_rawBuffer, _numCols);
    _rawBuffer[_cursorPos] = encode( (char)byte );
    // buffer, length, position
    printRaw( _rawBuffer, _numCols, 0);
    ++n;
  };

  if (_cursorPos < _numCols) {
    _rawBuffer[_cursorPos] = encode( (char)byte );
    // buffer, length, position
    printRaw( _rawBuffer, _cursorPos+1, 0);
    setCursor(1, _cursorPos + 1);
    ++n;
  }
  return n;
}

// null terminated char array
size_t  SevenSegmentTM1637::write(const char* str) {
  TM1637_DEBUG_PRINT(F("write char*:\t")); TM1637_DEBUG_PRINTLN(str);
  uint8_t encodedBytes[4];

  encode(encodedBytes, str, 4);
  uint8_t i =4;
  while( str[i] != '\0' ) {
    printRaw(encodedBytes);
    shiftLeft(encodedBytes, 4);
    encodedBytes[3] = encode( str[i] );
    i++;
    if ( i == TM1637_MAX_CHARS) {
      break;
    }
  }
  return i;
};

// byte array with length
size_t  SevenSegmentTM1637::write(const uint8_t* buffer, size_t size) {
  TM1637_DEBUG_PRINT(F("write uint8_t*:\t("));
  for(size_t i=0; i < size; i++) {
    TM1637_DEBUG_PRINT((char)buffer[i]);
    TM1637_DEBUG_PRINT(i == size -1?F(""):F(", "));
  }
  TM1637_DEBUG_PRINT(F(") "));
  TM1637_DEBUG_PRINT(size);

  uint8_t encodedBytes[TM1637_MAX_CHARS];

  if ( size > TM1637_MAX_CHARS) {
    size = TM1637_MAX_CHARS;
  }
  size_t length = encode(encodedBytes, buffer, size);
  TM1637_DEBUG_PRINT(F(" (")); TM1637_DEBUG_PRINT(length); TM1637_DEBUG_PRINT(F(", "));
  TM1637_DEBUG_PRINT(_cursorPos); TM1637_DEBUG_PRINTLN(F(")"));
  printRaw(encodedBytes, length, _cursorPos);
  return length;
};

// Liquid cristal API
void SevenSegmentTM1637::begin(uint8_t cols, uint8_t rows) {
  _numCols = cols;
  _numRows = rows;
  clear();
};

void SevenSegmentTM1637::init(uint8_t cols, uint8_t rows) {
  begin(cols, rows);
}

void SevenSegmentTM1637::clear(void) {
  uint8_t rawBytes[4] = {0,0,0,0};
  printRaw(rawBytes);
  home();
};

void SevenSegmentTM1637::home(void) {
  setCursor(0, 0);
};

void SevenSegmentTM1637::setCursor(uint8_t row, uint8_t col) {
  col = (col > TM1637_MAX_COLOM -1)?TM1637_MAX_COLOM-1:col;
  _cursorPos = col;

};

void SevenSegmentTM1637::setBacklight(uint8_t value) {
  value = (value    > 100 )?100:value;  // 0..100 brightness
  // Store the backlight value
  _backLightValue = value;
  // scale backlight value to 0..8
  value /= 10;                          // 0..10
  value = (value > 8   )?  8:value;     // only 8 levels and off
  uint8_t cmd = TM1637_COM_SET_DISPLAY;;
  switch ( value ) {
    case 0:
      cmd |= TM1637_SET_DISPLAY_OFF;
      break;
    case 1:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_1;
      break;
    case 2:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_2;
      break;
    case 3:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_4;
      break;
    case 4:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_10;
      break;
    case 5:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_11;
      break;
    case 6:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_12;
      break;
    case 7:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_13;
      break;
    case 8:
      cmd |= TM1637_SET_DISPLAY_ON | TM1637_SET_DISPLAY_14;
      break;
    };
#if TM1637_DEBUG
    bool ack = command(cmd);
    TM1637_DEBUG_PRINT(F("SET_DISPLAY:\t")); TM1637_DEBUG_PRINTLN((
      cmd
    ), BIN);
    TM1637_DEBUG_PRINT(F("Acknowledged:\t")); TM1637_DEBUG_PRINTLN(ack);
#else
    command(cmd);
#endif
};

void SevenSegmentTM1637::setContrast(uint8_t value) {
  setBacklight(value);
}

void SevenSegmentTM1637::on(void) {
  setBacklight(TM1637_DEFAULT_BACKLIGHT);
};

void SevenSegmentTM1637::off(void) {
  setBacklight(0);
  clear();
};

// SevenSegmentTM1637 public methods
void  SevenSegmentTM1637::blink(uint8_t blinkDelay, uint8_t repeats, uint8_t maxBacklight, uint8_t minBacklight) {
  for (uint8_t i=0; i < repeats; i++) {
    setBacklight(minBacklight);     // turn backlight off
    delay(blinkDelay);
    setBacklight(maxBacklight);     // turn backlight on
    delay(blinkDelay);
  }
  // restore backlight
  setBacklight(_backLightValue);
}

void  SevenSegmentTM1637::setPrintDelay(uint16_t printDelay) {
  _printDelay = printDelay;
};

bool  SevenSegmentTM1637::getColonOn(void) {
  return (_colonOn);
};

void  SevenSegmentTM1637::setColonOn(bool setToOn) {
    _colonOn = setToOn;
}
void  SevenSegmentTM1637::printRaw(uint8_t rawByte, uint8_t position) {
  uint8_t cmd[2];
  cmd[0] = TM1637_COM_SET_ADR | position;
  cmd[1] = rawByte;
  if (position == 1) { cmd[1]|=(_colonOn)?TM1637_COLON_BIT:0; };
  command(cmd, 2);
};

void  SevenSegmentTM1637::printRaw(const uint8_t* rawBytes, size_t length, uint8_t position) {
  // if fits on display
  if ( (length + position) <= _numCols) {
    uint8_t cmd[5] = {0, };
    cmd[0] = TM1637_COM_SET_ADR | (position & B111);  // sets address
    memcpy(&cmd[1], rawBytes, length);       // copy bytes

    // do we have to print a colon?
    if ( position < 2 ) { // printing after position 2 has never a colon
      if ( position == 0 && length >= 2) {
        // second index is the colon
        cmd[2] |= (_colonOn)?TM1637_COLON_BIT:0;
      } else {
        // first index is the colon
        cmd[1] |= (_colonOn)?TM1637_COLON_BIT:0;
      }
    }
    // TM1637_DEBUG_PRINT(F("ADDR :\t")); TM1637_DEBUG_PRINTLN(cmd[0],BIN);
    // TM1637_DEBUG_PRINT(F("DATA0:\t")); TM1637_DEBUG_PRINTLN(cmd[1],BIN);
    command(cmd, length+1);                           // send to display
  }
  // does not fit on display, need to print with delay
  else {
    // First print 1-4 characters
    uint8_t numtoPrint = _numCols - position;
    printRaw(rawBytes, numtoPrint, position);
    delay(_printDelay);

    // keep printing 4 characters till done
    uint8_t remaining = length - numtoPrint + 3;
    uint8_t i         = 1;
    while( remaining >= _numCols) {
      printRaw(&rawBytes[i], _numCols, 0);
      delay(_printDelay);
      remaining--;
      i++;
    };
  }

};

// Helpers
uint8_t SevenSegmentTM1637::encode(char c) {
  if ( c < ' ') { // 32 (ASCII)
    return 0;
  }
  return pgm_read_byte_near(AsciiMap::map + c - ' ');
};

uint8_t SevenSegmentTM1637::encode(int16_t d) {
  // can only encode single digit
  if ( d > 9 || d < 0) {
    return 0;
  };
  return pgm_read_byte_near(AsciiMap::map + d + '0' - ' ');
};

size_t  SevenSegmentTM1637::encode(uint8_t* buffer, const char* str, size_t bufferSize) {
  size_t i;

  for (i=0; i < bufferSize; i++) {
    if (str[i] == '\0' ) {
      return i;
    }
    buffer[i] = encode( str[i] );
  };
  return i;
}

size_t  SevenSegmentTM1637::encode(uint8_t* buffer, const uint8_t* byteArr, size_t bufferSize) {
  size_t i;

  for (i=0; i < bufferSize; i++) {
    buffer[i] = encode( (char)byteArr[i] );
  };
  return i;
}

void    SevenSegmentTM1637::shiftLeft(uint8_t* buffer, size_t length) {
  for (uint8_t i=0; i < length ; i++) {
    buffer[i] = buffer[i+1];
  }
}

// SevenSegmentTM1637 LOW LEVEL
bool    SevenSegmentTM1637::command(uint8_t cmd) const{
  return command(_pinClk, _pinDIO, cmd);
};

bool    SevenSegmentTM1637::command(uint8_t pinClk, uint8_t pinDIO, uint8_t cmd) {
  comStart(pinClk, pinDIO);
  comWriteByte(pinClk, pinDIO,cmd);
  bool acknowledged = comAck(pinClk, pinDIO);
  comStop(pinClk, pinDIO);
  return acknowledged;
}

bool    SevenSegmentTM1637::command(const uint8_t* commands, uint8_t length) const {
  return command(_pinClk, _pinDIO, commands, length);
};

bool    SevenSegmentTM1637::command(uint8_t pinClk, uint8_t pinDIO, const uint8_t* commands, uint8_t length) {
  bool acknowledged = true;
  comStart(pinClk, pinDIO);
  for (uint8_t i=0; i < length;i++) {
    comWriteByte(pinClk, pinDIO, commands[i]);
    acknowledged &= comAck(pinClk, pinDIO);
  };
  comStop(pinClk, pinDIO);
  return acknowledged;
}

uint8_t SevenSegmentTM1637::comReadByte(void) const {
  uint8_t readKey = 0;

  comStart();
  comWriteByte(TM1637_COM_SET_DATA | TM1637_SET_DATA_READ);
  comAck();

  pinAsInput(_pinDIO);
  digitalHigh(_pinDIO);
  delayMicroseconds(5);

  for ( uint8_t i=0; i < 8; i++) {

    readKey >>= 1;
    digitalLow(_pinClk);
    delayMicroseconds(30);

    digitalHigh(_pinClk);

    if ( isHigh(_pinDIO) ) {
      readKey = readKey | B10000000;
    };

    delayMicroseconds(30);


  };
  pinAsOutput(_pinDIO);
  comAck();
  comStop();
  return readKey;
};

void    SevenSegmentTM1637::comWriteByte(uint8_t command) const{
  comWriteByte(_pinClk, _pinDIO, command);
};

void    SevenSegmentTM1637::comWriteByte(uint8_t pinClk, uint8_t pinDIO, uint8_t command) {
  // CLK in bits
  for ( uint8_t i=0; i < 8; i++) {
    digitalLow(pinClk);   // CLK LOW

    if ( command & B1) {
      digitalHigh(pinDIO);// DIO HIGH
    } else {
      digitalLow(pinDIO); // DIO LOW
    }
    delayMicroseconds(TM1637_CLK_DELAY_US);

    command >>= 1;

    digitalHigh(pinClk);   // CLK HIGH
    delayMicroseconds(TM1637_CLK_DELAY_US);
  };
}

void    SevenSegmentTM1637::comStart(void) const {
  comStart(_pinClk, _pinDIO);
};

void    SevenSegmentTM1637::comStart(uint8_t pinClk, uint8_t pinDIO) {
  digitalHigh(pinDIO);   // DIO HIGH
  digitalHigh(pinClk);   // CLK HIGH
  delayMicroseconds(TM1637_CLK_DELAY_US);

  digitalLow(pinDIO);    // DIO  LOW
}

void    SevenSegmentTM1637::comStop(void) const {
  comStop(_pinClk, _pinDIO);
};

void    SevenSegmentTM1637::comStop(uint8_t pinClk, uint8_t pinDIO) {
  digitalLow(pinClk);   // CLK LOW
  delayMicroseconds(TM1637_CLK_DELAY_US);

  digitalLow(pinDIO);    // DIO LOW
  delayMicroseconds(TM1637_CLK_DELAY_US);

  digitalHigh(pinClk);   // CLK HIGH
  delayMicroseconds(TM1637_CLK_DELAY_US);

  digitalHigh(pinDIO);   // DIO HIGH
}

bool    SevenSegmentTM1637::comAck(void) const {
  return comAck(_pinClk, _pinDIO);
};

bool    SevenSegmentTM1637::comAck(uint8_t pinClk, uint8_t pinDIO) {
  bool acknowledged = false;

  digitalLow(pinClk);          // CLK  LOW
  pinAsInputPullUp(pinDIO);    // DIO INPUT PULLUP (state==HIGH)
  delayMicroseconds(TM1637_CLK_DELAY_US);

  acknowledged = isLow(pinDIO);// Ack should pull the pin low again

  digitalHigh(pinClk);         // CLK HIGH
  delayMicroseconds(TM1637_CLK_DELAY_US);

  digitalLow(pinClk);          // CLK  LOW
  pinAsOutput(pinDIO);

  return acknowledged;
}