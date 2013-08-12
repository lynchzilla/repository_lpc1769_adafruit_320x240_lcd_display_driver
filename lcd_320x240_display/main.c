// ----------------------------------------------------------------------------------------------------
//        CooCox CoOS TFT LCD Display Example  (LPC1769 LPCXpresso evaluation board))
//
// Purpose: Operate the Adafruit 2.8" 18-bit color TFT LCD with touchscreen breakout board - ILI9325
//
//			Software driver for NXP LPC1769 LPCXpresso board adapted from Adafruit Arduino Library.
//
//			The following files comprise the driver:  lcddriver.c
//                                                    lcddriver.h
//                                                    font.c
//
//			The main() function initializes the LPCXpresso board, initializes the CoOS real
//          time kernel, creates a single task0, and starts the CoOS kernel.
//
//			task0 runs as an endless loop running LadyAda's suite of graphics tests.
//
//          Note: this driver and sample application does not address the touch screen system
//
// Programmer:  James P Lynch  (LadyAda obviously for the Arduino version).
// ----------------------------------------------------------------------------------------------------


#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc_types.h"
#include "CoOs.h"
#include "lcddriver.h"


#define STACK_SIZE_DEFAULT	512
#define PORT0				0
#define INPUT				0
#define OUTPUT				1
#define LED_PINS			(uint32_t)1<<22

// Set up a 512 byte stack for task1
OS_STK task1_stk[STACK_SIZE_DEFAULT];

// function prototypes
void testfillcircles(uint8_t radius, uint16_t color);
void testdrawcircles(uint8_t radius, uint16_t color);
void testfillrects(uint16_t color1, uint16_t color2);
void testdrawrects(uint16_t color);
void testfastlines(uint16_t color1, uint16_t color2);
void testlines(uint16_t color);
void loop(void);
void testBars();

uint16_t	DeviceCode;		// identifies the lcd controller chip


// -------------------------------------------------------------------
//  initializeBoard() - Sets up the GPIO port 0 to control LED2
//
//                      Turns on the peripheral clock for GPIO Port0
//                      Sets bit 22 of GPIO Port0 to be an output
// -------------------------------------------------------------------
void initializeBoard(){

    // enable GPIO Clock
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);

    // LED2 on PORT0.22 defined as Output
    FIO_SetDir(PORT0, _BIT(LED_PINS), OUTPUT);

}


// ------------------------------------------------------------------------
//  task1( ) - Blinks LED2 at a two second rate.
//
//             Most CoOS tasks are coded as endless loops.
//
//             This endless loop sets and clears GPIO port0, bit22.
//
//			   A one second CoTickDelay() function after each LED2
//             state change gives the two second repetition rate.
//
//             Note: The current default CoOS tick rate is 10 milliseconds.
//                   Thus 100 * 10msec = 1000msec or 1.0 seconds.
// ------------------------------------------------------------------------
void task1 (void* pdata){

	// initialize the AdaFruit 2.8" TFT LCD Display
	lcdInitDisplay();

	// vLcdTask endless loop
	for (;;) {

		// read the device code
		DeviceCode = readRegister(0x00);

		// LadyAda's Graphics Test  (with minor modifications)
		lcdInitDisplay();

		fillScreen(BLACK);

		drawChar(10, 100, 'A', RED, BLACK, SMALL);
		drawChar(30, 100, 'A', RED, BLACK, MEDIUM);
		drawChar(60, 100, 'A', RED, BLACK, LARGE);

		drawString(10, 200, "hello world!", CYAN, BLACK, SMALL);
		drawString(10, 240, "hello world!", YELLOW, BLACK, MEDIUM);
		drawString(10, 280, "hello world!", BLACK, WHITE, LARGE);
		CoTickDelay(200);

		testlines(CYAN);
		CoTickDelay(200);
		testfastlines(RED, BLUE);
		CoTickDelay(200);

		testdrawrects(GREEN);
		CoTickDelay(200);

		testfillrects(YELLOW, MAGENTA);
		CoTickDelay(200);

		fillScreen(BLACK);
		testfillcircles(10, MAGENTA);
		testdrawcircles(10, WHITE);
		CoTickDelay(200);

		fillScreen(BLACK);
		loop();
		CoTickDelay(200);

		fillScreen(BLACK);
		testBars();
		CoTickDelay(200);
	}
}


void testfillcircles(uint8_t radius, uint16_t color)
{
	uint16_t	x,y;

	for (x = radius; x < TFTLCD_WIDTH; x += radius * 2) {
		for (y = radius; y < TFTLCD_HEIGHT; y += radius * 2) {
			fillCircle(x, y, radius, color);
		}
	}
}


void testdrawcircles(uint8_t radius, uint16_t color)
{
	uint16_t	x,y;

	for (x = 0; x < TFTLCD_WIDTH + radius; x += radius * 2) {
		for (y = 0; y < TFTLCD_HEIGHT + radius; y += radius * 2) {
			drawCircle(x, y, radius, color);
		}
	}
}


void testfillrects(uint16_t color1, uint16_t color2)
{
	uint16_t	x;

	fillScreen(BLACK);

	for (x = TFTLCD_WIDTH - 1; x > 6; x -= 6) {
		fillRect(TFTLCD_WIDTH / 2 - x / 2, TFTLCD_HEIGHT / 2 - x / 2 , x, x, color1);
		drawRect(TFTLCD_WIDTH / 2 - x / 2, TFTLCD_HEIGHT / 2 - x / 2 , x, x, color2);
	}
}


void testdrawrects(uint16_t color)
{
	uint16_t	x;

	fillScreen(BLACK);

	for (x = 0; x < TFTLCD_WIDTH; x += 6) {
		drawRect(TFTLCD_WIDTH / 2 - x / 2, TFTLCD_HEIGHT / 2 - x / 2 , x, x, color);
	}
}


void testfastlines(uint16_t color1, uint16_t color2)
{
	uint16_t	x,y;

	fillScreen(BLACK);

	for (y = 0; y < TFTLCD_HEIGHT; y += 5) {
		drawHorizontalLine(0, y, TFTLCD_WIDTH, color1);
   }

	for (x = 0; x < TFTLCD_WIDTH; x += 5) {
		drawVerticalLine(x, 0, TFTLCD_HEIGHT, color2);
   }
}


void testlines(uint16_t color)
{
	uint16_t	x,y;

	fillScreen(BLACK);

	for (x = 0; x < TFTLCD_WIDTH; x += 6) {
		drawLine(0, 0, x, TFTLCD_HEIGHT - 1, color);
	}

	for (y = 0; y < TFTLCD_HEIGHT; y += 6) {
		drawLine(0, 0, TFTLCD_WIDTH - 1, y, color);
	}

	fillScreen(BLACK);

	for (x = 0; x < TFTLCD_WIDTH; x += 6) {
		drawLine(TFTLCD_WIDTH - 1, 0, x, TFTLCD_HEIGHT - 1, color);
	}

	for (y = 0; y < TFTLCD_HEIGHT; y += 6) {
		drawLine(TFTLCD_WIDTH - 1, 0, 0, y, color);
	}

	fillScreen(BLACK);

	for (x = 0; x < TFTLCD_WIDTH; x += 6) {
		drawLine(0, TFTLCD_HEIGHT - 1, x, 0, color);
	}

	for (y = 0; y < TFTLCD_HEIGHT; y += 6) {
		drawLine(0, TFTLCD_HEIGHT - 1, TFTLCD_WIDTH - 1, y, color);
	}

	fillScreen(BLACK);

	for (x = 0; x < TFTLCD_WIDTH; x += 6) {
		drawLine(TFTLCD_HEIGHT - 1, TFTLCD_HEIGHT - 1, x, 0, color);
	}

	for (y = 0; y < TFTLCD_HEIGHT; y += 6) {
		drawLine(TFTLCD_WIDTH - 1, TFTLCD_HEIGHT - 1, 0, y, color);
	}
}


void loop(void) {
	for (rotation = 0; rotation < 4; rotation++) {
		setRotation(rotation);

		goHome();
		testBars();
		CoTickDelay(200);

		fillScreen(BLACK);

		drawPixel(TFTLCD_WIDTH / 2, TFTLCD_HEIGHT / 2, RED);
		CoTickDelay(200);

		drawLine(0, 0, TFTLCD_WIDTH - 1, TFTLCD_HEIGHT, BLUE);
		CoTickDelay(200);
	}
}


void testBars() {
	uint16_t	i,j;

	for(i = 0; i < TFTLCD_HEIGHT; i++)
		{

		for(j=0; j < TFTLCD_WIDTH; j++)
			{
			if (i>279)		writeData(WHITE);
			else if(i>239)	writeData(BLUE);
			else if(i>199)	writeData(GREEN);
			else if(i>159)	writeData(CYAN);
			else if(i>119)	writeData(RED);
			else if(i>79)	writeData(MAGENTA);
			else if(i>39)	writeData(YELLOW);
			else			writeData(BLACK);
		}
	}
}



// ------------------------------------------------------------------------
//  main( ) - Main program creates task0 and then starts CoOS kernel.
//
// ------------------------------------------------------------------------
int main(void)
{
	// initialize the LPCXpresso board by:
	// 		turning on clock signal for the GPIO peripheral.
	// 		setting GPIO port 0, bit22 for output (to drive LED2)
	initializeBoard();

	// initialize the CoOS real time kernel
	CoInitOS();

	// create task1
	CoCreateTask(task1,0,0,&task1_stk[STACK_SIZE_DEFAULT-1],STACK_SIZE_DEFAULT);

	// start the CoOS real time kernel
	CoStartOS();

	// you should never get here!
    while(1);
}
