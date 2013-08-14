repository_lpc1769_adafruit_320x240_lcd_display_driver
======================================================


Purpose:  Port of Adafruit Device Driver for 3.2" 320x240 TFT LCD display to the 
          NXP LPC1769 microprocessor using CooCox IDE.




Original Author:      Limor Fried  (LadyAda)
LPC1769 Port Author:  James P. Lynch

Target Board:                  NXP LPC1769 LPCXpresso evaluation board
Software Development System:   CooCox Eclipse-based free IDE

Driver Components you'll need:  lcddriver.c
                                lcddriver.h
                                font.c
                                
LadyAda's test suite:           main.c 


Improvements to LadyAda's original driver:

    Improved Annotation
    Use of graphics box for character draw
    





The Adafruit 3.2" 320x240 TFT color LCD display ($40) was originally targeted to the Arduino market and LadyAda's 
driver library was specifically interfaced to an Arduino UNO or Mega board.

I recoded LadaAda's library for the NXP LPC1769 ARM-Cortex M3 microprocessor, specifically the LPCXpresso 
evaluation board.


   First, let's look at the AdaFruit TFT LCD board:

  					AdaFruit LCD Display
  					--------------------

  320x240 pixels     2.8" diagonal  8-bit parallel interface  16-bit color format (64k colors) ILITEK 9325 controller

                List of AdaFruit TFT LCD Signals

 1x20  2x10   Name        Description           Notes
 __________________________________________________________________________________________

  1     1      GND         ground
  2     2      3-5V        Vss supply           (5v or 3.3v)

  3     3      CS          chip select          (low = chip selected)
  4     4      CD          register select      (low = select index register, high = data)
  5     5      WR          write strobe         (low = write data)
  6     6      RD          read strobe          (low = read data)
  7     7      RST         reset                (low = reset ILI9328 LCD controller)
  8     8      Backlite    back light control   (high = turn on back light)

  9     9      X+          touch screen
  10    10     Y+          touch screen
  11    11     X-          touch screen
  12    12     Y-          touch screen

  13	  13   	 D0          8-bit data bus (lsb)
  14	  14     D1          8-bit data bus
  15	  15     D2          8-bit data bus
  16	  16     D3          8-bit data bus
  17	  17     D4          8-bit data bus
  18	  18     D5          8-bit data bus
  19	  19     D6          8-bit data bus
  20	  20     D7          8-bit data bus (msb)


  Selecting the Microprocessor Interface
  --------------------------------------

  The ILI9325 Controller Pin Descriptions show that Input Interface Pins IM3..IM0 select the interface desired.

  IM3   IM2   IM1   IM0
  ----------------------------------------------------------
   0     0     1     1    Select i80-system 8-bit interface

   Notes: AdaFruit schematic only shows IM0=1 and IM3=0 connections.
       		IM2=0 only used to select SPI-mode interface, assume that this is tied to ground but not shown
       		IM1=1 only used to select i80-system 8/16 bit interface, assume that this is tied to Vcc but not shown

       		bit TRI=0 and bit DFM=0 in Register R03h (Entry Mode) select "system 8-bit interface (2 transfer/pixel) 
       		65536 colors,  TRI and DFM are set in LCD_Setup() function.
       		
       		


   Writing a Single Pixel
   ----------------------

                              System 8-bit interface (2 transfers per pixel)
   |-----|-----|-----|-----|-----|-----|-----|-----|  |-----|-----|-----|-----|-----|-----|-----|-----|
   | R5  | R4  | R3  | R2  | R1  | G5  | G4  | G3  |  | G2  | G1  | G0  | B5  | B4  | B3  | B2  | B1  |
   |-----|-----|-----|-----|-----|-----|-----|-----|  |-----|-----|-----|-----|-----|-----|-----|-----|
     15    14    13    12    11    10     9     8        7     6     5     4     3     2     1     0
                    First 8-bit transfer                             Second 8-bit transfer
                    
                    
   Note:  5-bits RED    6-bits GREEN      5-bits BLUE

   For example:    #define BLACK		0x0000                  Note: AdaFruit documentation for the 2.8" TFT
                   #define BLUE		  0x001F                        LCD Display lists 262144 colors.
                   #define RED 		  0xF800
                   #define GREEN		0x07E0                        Since her pixel data transfer is only 16-bits,
                   #define CYAN		  0x07FF                        the number of possible colors is actually 65536.
                   #define MAGENTA  0xF81F
                   #define YELLOW   0xFFE0                        So what? That's still a lot of colors!
                   #define WHITE		0xFFFF


    Write Command Timing Example

    CD  -----------|                        |---------------- register select = 0 (command)
                   |                        |
                   |________________________|

    CS  ------------------|               |------------------ chip select
                          |               |
                          |_______________|

    WR ---------------------|          |--------------------- write strobe (latches data on rising edge)
                            |          |
                            |__________|
                                       :
                                       :
    D0..7 ---------------------|-------:---|----------------- write data (represents all eight data bus lines)
                               |       V   |
                               |___________|                  note: Data should be stable for 10nsec before WR rises
                                                                    Data should be stable for 15nsec after WR rises
                                      data
                                     captured


    Write Data Timing Example

    CD  ----------------------------------------------------- register select = 1 (data)

    CS  ------------------|               |------------------ chip select
                          |               |
                          |_______________|

    WR ---------------------|          |--------------------- write strobe (latches data on rising edge)
                            |          |
                            |__________|
                                       :
                                       :
    D0..7 ---------------------|-------:---|----------------- write data (represents all eight data bus lines)
                               |       V   |
                               |___________|                  note: Data should be stable for 10nsec before WR rises
                                                                    Data should be stable for 15nsec after WR rises
                                      data
                                     captured
                                      here


    Read Data Timing Example

    CD  ----------------------------------------------------- register select = 1 (data)

    CS  ------------------|                   |------------------ chip select
                          |                   |
                          |___________________|

    RD ---------------------|          |--------------------- read strobe (latches data on rising edge)
                            |          |
                            |__________|
                                       :
                                       :
    D0..7 ------------------|----------:--|-------------------- read data (represents all eight data bus lines)
                            |          V  |
                            |_____________|                  note: Data should be stable for 10nsec before WR rises
                                                                 Data should be stable for 15nsec after WR rises
                                      data
                                     captured
                                      here




  Assignment of LCD Control/Data Signals to LPCXpresso Board I/O Ports
  ----------------------------------------------------------------------

        LCD                                     LPCXpresso Board
    Pin    Description                     J6 Pin   Description
  ----------------------------------------------------------------------
    1      Ground                            1      GND    ground
    2      3.3 volts                         2      3V3    3.3 volts
    3      CS chip select                    23     P0.21  Port0 bit21
    4      CD register select  (RS)          24     P0.22  Port0 bit22
    5      WR write strobe                   15     P0.23  Port0 bit22
    6      RD read strobe                    16     P0.24  Port0 bit24
    7      RST restart                       17     P0.25  Port0 bit25
    8      Backlite on                       2      3V3    3.3 volts  (backlight wired to always be ON)
    9      X+  touch screen
    10     Y+  touch screen
    11     X-  touch screen
    12     Y-  touch screen
    13     D0  (lsb)                         15     P0.0   Port0 bit0
    14     D1                                16     P0.1   Port0 bit1
    15     D2                                17     P0.2   Port0 bit2
    16     D3                                18     P0.3   Port0 bit3
    17     D4                                19     P0.4   Port0 bit4
    18     D5                                20     P0.5   Port0 bit5
    19     D6                                21     P0.6   Port0 bit6
    20     D7  (msb)                         22     P0.7   Port0 bit7




	  GPIO Operations with NXP LPC1769 ARM-Cortex Microprocessors
    -----------------------------------------------------------

	  There are five GPIO ports (32-bits each): Port0, Port1, Port2, Port3, and Port4

	  Control is via 4 configuration registers:

			FIOxMASK =  mask register  (any bits set to 0, unmasked, can be changed)
			FIOxDIR  =  direction register (0 = bit is a digital input,  1 = bit is a digital output)
			FIOxSET  =  set bits     (1 = set this bit if it is also unmasked)
			FIOxCLR  =  clear bits   (1 = clear this bit if it is also unmasked)



   The direction (input / output) must be configured by the FIO0DIR 32-bit register  0x2009 C000
   note:  0 = input,  1 = output

            Fast GPIO Port0 Direction control register     FIO0DIR

                                         RST   RD    WR    C/D   CS
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |     |     |     |     |     |     |P0.25|P0.24|P0.23|P0.22|P0.21|     |     |     |     |     |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      31    30    29    28    27    26    25    24    23    22    21    20    19    18    17    16

                                                     D7    D6    D5    D5    D3    D2    D1    D0
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |     |  *  |  *  |  *  |     |     |     |     |P0.7 |P0.6 |P0.5 |P0.4 |P0.3 |P0.2 |P0.1 |P0.0 |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      15    14    13    12    11    10    9     8     7     6     5     4     3     2     1     0
            Note: bits 12, 13, 14 not available for Port0


   You can unmask only the above bits to be changed by the FIO0MASK 32-bit register  0x2009 C010
   note:  0 = unmasked (allow bit to be changed),  1 = masked (bit does not participate in I/O operations)

            Fast GPIO Port0 Mask control register     FIO0MASK

                                         RST   RD    WR    C/D   CS
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |  1  |  1  |  1  |  1  |  1  |  1  |  0  |  0  |  0  |  0  |  0  |  1  |  1  |  1  |  1  |  1  |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      31    30    29    28    27    26    25    24    23    22    21    20    19    18    17    16

                                                     D7    D6    D5    D5    D3    D2    D1    D0
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |  1  |  1  |  1  |  1  |  1  |  1  |  1  |  1  |  0  |  0  |  0  |  0  |  0  |  0  |  0  |  0  |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      15    14    13    12    11    10    9     8     7     6     5     4     3     2     1     0
      Note: FIO0MASK defaults to all zeros (all bits unmasked) at boot-up
            Probably best to not count on this, rather explicitly specify which bits are unmasked



   Any bit can be set by the FIO0SET 32-bit register  0x2009 C018  (bits must be unmasked via FIO0MASK register)
   note:  0 = no change,  1 = bit set

            Fast Port0 Output Set register register     FIO0SET

                                         RST   RD    WR    C/D   CS
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |     |     |     |     |     |     |P0.25|P0.24|P0.23|P0.22|P0.21|     |     |     |     |     |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      31    30    29    28    27    26    25    24    23    22    21    20    19    18    17    16

                                                     D7    D6    D5    D5    D3    D2    D1    D0
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |     |  *  |  *  |  *  |     |     |     |     |P0.7 |P0.6 |P0.5 |P0.4 |P0.3 |P0.2 |P0.1 |P0.0 |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      15    14    13    12    11    10    9     8     7     6     5     4     3     2     1     0
            Note: bits 12, 13, 14 not available for Port0


   Any bit can be cleared by the FIO0CLR 32-bit register  0x2009 C01C  (bits must be unmasked via FIO0MASK register)
   note:  0 = no change,  1 = bit cleared

            Fast Port0 Output Set register register     FIO0CLR

                                         RST   RD    WR    C/D   CS
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |     |     |     |     |     |     |P0.25|P0.24|P0.23|P0.22|P0.21|     |     |     |     |     |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      31    30    29    28    27    26    25    24    23    22    21    20    19    18    17    16

                                                     D7    D6    D5    D5    D3    D2    D1    D0
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
   |     |  *  |  *  |  *  |     |     |     |     |P0.7 |P0.6 |P0.5 |P0.4 |P0.3 |P0.2 |P0.1 |P0.0 |
   |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
      15    14    13    12    11    10    9     8     7     6     5     4     3     2     1     0
            Note: bits 12, 13, 14 not available for Port0



   Simple Example  -  pulse the Write line (LCD_WR)
   ================================================

   from lcddriver.h, we have these useful #define macros:
   	#define LCD_PORT0	0
   	#define LCD_PORT1	1
   	#define LCD_PORT2	2
   	#define LCD_PORT3	3
   	#define LCD_PORT4	4
		#define LCD_MASK	1
		#define LCD_UNMASK	0
		#define LCD_INPUT	0
		#define LCD_OUTPUT	1
		#define LCD_CS		21
		#define LCD_CD		22
		#define LCD_WR		23
		#define LCD_RD		24
		#define LCD_RST		25

	 from lpc_types.h, we have the following useful #define macros:
		#define _BIT(n)	(1<<n)
		#define _SBF(f,v) (v<<f)


	  #include "lpc_types.h"
	  #include "lcddriver.h"
	  #include "lpc17xx_gpio.h"
	  #include "lpc17xx_clkpwr.h"

    uint32_t	LastStatePort0;

    /* Enable GPIO Clock */
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);

    /* set mask for P0.23 (LCD_WR) via 32-bit Fast GPIO functions  */
    FIO_SetMask(LCD_PORT0, _BIT(LCD_WR), UNMASK);

    /* set I/O port direction via 32-bit FastGPIO functions  */
    FIO_SetDir(LCD_PORT0, _BIT(LCD_WR), LCD_OUTPUT)

    /* Output WR to high level via 32-bit FastGPIO functions  */
    FIO_SetValue(LCD_PORT0, _BIT(LCD_WR));

   /* Output WR to low level via 32-bit FastGPIO functions  */
    FIO_ClearValue(LCD_PORT0, _BIT(LCD_WR));

    /* for completeness, there's also a 32-bit FastGPIO function to read an I/O port  */
    LastStatePort0 = FIO_ReadValue(LCD_PORT0);





Compendium of Graphics Functions Supported by this Driver
=====================================================================================================================


// ***********************************************************************************************
// *  goHome - sets the horizontal and vertical GRAM address to (0, 0) and then selects
// *           the Write Data to GRAM register
// *
// *	Inputs: none
// *
// *	Returns: nothing
// ***********************************************************************************************
void goHome(void)



// ***********************************************************************************************
//    goTo - sets the horizontal and vertical GRAM address (x0,y0) and then selects
//           the Write Data to GRAM register
//
//  	Inputs: x       = starting x address (0 .. 239)
//  			y       = starting y address (0 .. 319)
//
//  	Returns: nothing
// ***********************************************************************************************
inline void goTo(int x, int y)



// ****************************************************************************************
//    drawChar - draws an ASCII character at the specified (x,y) address, color, and size
//
//      Inputs:	  x       = starting x address (0 .. 239)  upper-left corner of font box
//      		  y       = starting y address (0 .. 319)  upper-left corner of font box
//  			  c       =   ASCII character to be displayed
//  	          fcolor  =   12-bit foreground color value		rrrrrggggggbbbbb
//  	          bcolor  =   12-bit background color value		rrrrrggggggbbbbb
//  			  size    =   font pitch (0=SMALL, 1=MEDIUM, 2=LARGE)
//
//      Returns:  nothing
//
//
//    Notes:  Assume that setWriteDir(); has already been called.
//
//            Here's an example to display "E" at address (20,20)
//
//  		  drawChar(20, 20, 'E', WHITE, BLACK, SMALL);
//
//  				 (20,20)        (27,20)
//  					|             |
//  					|             |
//  			      	V             V
//  				 ---------x-------->
//  			     :  _ # # # # # # #   0x7F
//  				 :  _ _ # # _ _ _ #   0x31
//  			     :  _ _ # # _ # _ _   0x34
//  			     y  _ _ # # # # _ _   0x3C
//  			     :  _ _ # # _ # _ _   0x34
//  			     :  _ _ # # _ _ _ #   0x31
//  			     :  _ # # # # # # #   0x7F
//  			     :  _ _ _ _ _ _ _ _   0x00
//                   V
//  					^             ^
//  					|             |
//  					|             |
//  				 (20,27)       (27,27)
//
//
//  	The most efficient way to display a character is to make use of the window address "wrap-around"
//      feature of the Ilitek ILI9325 LCD controller chip.
//
//  	Assume that we position the character at (20, 20). The starting pixel is thus top - left.
//  	With the window address set commands, you can specify an 8x8 window address box for the SMALL and
//      MEDIUM characters or a 16x24 window address box for the LARGE characters.
//
//  		writeRegister(LCD_HOR_START_AD, x0);
//  		writeRegister(LCD_HOR_END_AD, x1);
//  		writeRegister(LCD_VER_START_AD, y0 + 8);
//          writeRegister(LCD_VER_END_AD, y1 + 8);
//
//  	When the algorithm completes the pixel at col 27, the column address wraps back to 20
//  	At the same time, the row address increases by one (this is done by the controller)
//
//  	The following simple loop will suffice:
//
//      	writeRegister(TFTLCD_GRAM_HOR_AD, x)
//          writeRegister(TFTLCD_GRAM_VER_AD, y)
//          writeCommand(TFTLCD_RW_GRAM);
//
//  	    for (i = 0; i <= nBytes; i++) {
//          	// copy pixel byte from font table and then decrement row
//          	PixelRow = *pChar++;
//
//          	// loop on each pixel in the byte (left to right)
//          	Mask = 0x80;
//          	for (j = 0; j < nCols; j++) {
//          		// if pixel bit set, use foreground color; else use the background color
//          		writeData_unsafe(((PixelRow & Mask) == 0) ? bColor : fColor);
//          		Mask = Mask >> 1;
//  	    	}
//          }
//
//    Once the "window" is set up and the address registers are pointing to the first
//    pixel (x, y), the algorithm is simply 64 writeData_unsafe(color) operations.
//
//    	Author:  James P Lynch    May 2, 2011
//  ***************************************************************************************
void drawChar(uint16_t x, uint16_t y, char c, uint16_t fColor, uint16_t bColor, uint8_t s)




// ***********************************************************************************************
//   drawString - Draws a null-terminated string c starting at(x0,y0) in the color specified
//
//		Inputs: x0    = row address (0 .. 131)
// 				y0    = column address (0 .. 131)
// 				c 	  = null-terminated string
// 				color = 16-bit color value rrrrrggggggbbbbb
//
// 		Returns: nothing
//
//   Note: if you run past the edges, there will be weird wrap-around effects
//         function will overrun and fail if the string is not null-terminated
// ***********************************************************************************************
void drawString(uint16_t x, uint16_t y, char *c, uint16_t fColor, uint16_t bColor, uint8_t s)



// ***********************************************************************************************
//   drawCircle - Draws a circle outline in the specified color at center (x0,y0) with radius r
//
//		Inputs: x0    = row address (0 .. 131)
// 				y0    = column address (0 .. 131)
// 				r 	  = radius in pixels
// 				color = 16-bit color value rrrrrggggggbbbbb
//
// 		Returns: nothing
//
// Author: Jack Bresenham IBM, Winthrop University (Father of this algorithm, 1962)
//
// Note: taken verbatim Wikipedia article on Bresenham's line algorithm
//       http://www.wikipedia.org
// ***********************************************************************************************
void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)




// ***********************************************************************************************
//   fillCircle - Fills a circle in the specified color at center (x0,y0) with radius r
//
//		Inputs: x0    = row address (0 .. 131)
// 				y0    = column address (0 .. 131)
// 				r 	  = radius in pixels
// 				color = 16-bit color value rrrrrggggggbbbbb
//
// 		Returns: nothing
// ***********************************************************************************************
void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)




// ***************************************************************************************************
// *  fillScreen - fills entire screen with the specified color
// *
// *		Inputs: color = 16-bit fill color value  (rrrrrggggggbbbbb)
// *
// *	    Returns: nothing
// ********************************************************************************************
void fillScreen(uint16_t color)



// ***************************************************************************************************
//    drawRect - draws a NON-filled rectangle in the specified color from (x, y) to (x + w, y + h)
//
//  		Inputs: x     = horizontal address - top left corner  (0..239)
//  				y     = vertical address   - top left corner  (0..319)
//  				w     = width in pixels
//  				h     = height in pixels
//  				color = 16-bit fill color value  (rrrrrggggggbbbbb)
//
//  	    Returns: nothing
//
// ********************************************************************************************
void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)




// ************************************************************************************************
//    fillRect - draws a filled rectangle in the specified color from (x0,y0) to (x0 + w, y0 + h)
//
//  		Inputs: x     = horizontal address - top left corner (0..239)
//  				y     = vertical address   - top left corner (0..319)
//  				w     = width in pixels
//  				h     = height in pixels
//  				color = 16-bit fill color value  (rrrrrggggggbbbbb)
//
//  	    Returns: nothing
//
//      Notes:
//
//      The best way to fill a rectangle is to take advantage of the "wrap-around" feature
//      built into the Ilitek ILI9325 controller. By defining a drawing box, the memory can
//      be simply filled by successive memory writes until all pixels have been illuminated.
//
//      Author: James P Lynch July 7, 2007
// ************************************************************************************************
void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)




// *************************************************************************************************
//   drawFastLine - draws a vertical or horizontal line in the specified color
//
//   Inputs:	x0    = starting x address (0 .. 239)
//   			y0    = starting y address (0 .. 319)
//          	l     = length in pixels
//   			color = 16-bit color value rrrrrggggggbbbbb
//              flag  = 0=horizontal, 1=vertical
//
//   Returns: nothing
// *************************************************************************************************
void drawFastLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color, uint8_t rotflag)




// *************************************************************************************************
//   drawVerticalLine - draws a vertical line in the specified color
//
//   Inputs:	x0     = starting x address (0 .. 239)
//   			y0     = starting y address (0 .. 319)
//          	length = length in pixels
//   			color  = 16-bit color value rrrrrggggggbbbbb
//
//   Returns: nothing
// *************************************************************************************************
void drawVerticalLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color)




// *************************************************************************************************
//   drawHorizontalLine - draws a horizontal line in the specified color
//
//   Inputs:	x0     = starting x address (0 .. 239)
//   			y0     = starting y address (0 .. 319)
//          	length = length in pixels
//   			color  = 16-bit color value rrrrrggggggbbbbb
//
//   Returns: nothing
// *************************************************************************************************
void drawHorizontalLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color)




// *************************************************************************************************
//   drawLine - draws a line in the specified color from (x0,y0) to (x1,y1)
//
//   Inputs:	x0 = starting x address (0 .. 239)
//   			y0 = starting y address (0 .. 319)
//          	x1 = ending x address   (0 .. 239)
//   			y1 = ending y address   (0 .. 319)
//   			color = 16-bit color value rrrrrggggggbbbbb
//   				where:  rrrrr  = 11111 full red
//   										:
//   						         00000 red is off
//
//   						gggggg = 111111 full green
//   										:
//   								 000000 green is off
//
//   						bbbbb  = 11111 full blue
//   										:
//   								 00000 blue is off
//
//   Returns: nothing
//
//   Note: good write-up on this algorithm in Wikipedia (search for Bresenham's line algorithm)
//         see lcd.h for some sample color settings
//
//   Authors: Dr. Leonard McMillan, Associate Professor UNC
//   		  Jack Bresenham IBM, Winthrop University (Father of this algorithm, 1962)
//
//   Note: taken verbatim from Professor McMillan's presentation:
//         http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
// *************************************************************************************************
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)




// ********************************************************************************
//   drawPixel -  draws a single pixel at the specified (x,y) address and color.
//
//   Parameters:  x:      horizontal address in pixels (0..239)
//                y:      vertical address in pixels (0..319)
//                color:  color value (16-bits)  RRRRRGGGGGGBBBBB
//
//   Returns:     nothing
// ********************************************************************************
void drawPixel(uint16_t x, uint16_t y, uint16_t color)




// *************************************************************************************************************
//   lcdInitDisplay -  initializes the ILI9325 LCD controller
//
//  					1. turns on PortA and PortC clocks
//  					2. sets PortA bits 0..7 as digital outputs (data bus)
//  					3. sets PortB bits 0..5 as digital outputs (control lines)
//  					4. idles all control lines
//                      5. resets the ILI9326 LCD controller
//  					6. sets up all required ILI9325 registers
//  					7. turns on the display
//
//   Parameters:  none
//
//   Returns:     nothing
// *************************************************************************************************************
void lcdInitDisplay(void)




// ****************************************************************************
//   reset -  resets the ILI9325 LCD controller
//
//   Parameters:  none
//
//   Returns:     nothing
// ****************************************************************************
void reset(void)




// *********************************************************************************************
//   setRotation -  Sets the GRAM access writing direction
//
//   Parameters:  dir: access writing direction (0..3).
//
//   Returns:     Nothing
//
//   Note:  default direction is 3 where (0,0) is top left and
//          pixel writing is from left to right then down.
//
//
//    (239,319)|----------------------|(000,319)   (000,000)|----------------------|(000,000)
//             |E@<-----------------@ |                     | @   @              @E|
//           Y |         ::           |                     | ^   ^              ^ |
//           | |         ::           |                     | |   |    ..        | |
//           | | @<-----------------@ |                     | |   |    ..        | |
//           V |                      |                     | |   |              | |
//             | @<-----------------@B|                     |B@   @              @ |
//    (239,000)|----------------------|(000,000)   (000,000)|----------------------|(000,000)
//
//                      dir = 0                                       dir = 1
//
//
//                                                 ( x,  y )        x ---->
//             |----------------------|            (000,000)|----------------------|(000,319)
//             | @              @   @B|                     |B@----------------->@ |
//             | |              |   | |                   y |                      |
//             | |      ..      |   | |                   | | @----------------->@ |
//             | |      ..      |   | |                   | |          :           |
//             | V              V   V |                   v |          :           |
//             |E@              @   @ |                     | @----------------->@E|
//             |----------------------|            (239,000)|----------------------|(239,319)
//                    dir = 2                                   dir = 3 (DEFAULT)
//
// *********************************************************************************************
void setRotation(uint8_t dir)




// ****************************************************************************
//   setRotation -  returns the GRAM access writing direction
//
//   Parameters:  none
//
//   Returns:     access writing direction (0..3).
// ****************************************************************************
uint8_t getRotation(void)




// ****************************************************************************
//   lcdDelay -  Inserts a delay time. (warning: this hog-ties the CPU)
//
//   Parameters:  nCount: specifies the delay time in msec.
//
//   Returns:     Nothing
// ****************************************************************************
void lcdDelay(uint32_t nCount)



Cheers,

Jim Lynch


