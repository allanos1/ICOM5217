/*
 * lcd.c
 *
 *  Created on: Sep 30, 2013
 *      Author: Anthony
 */

#include "lcd.h"

//////////////////////////////////////////
// API Layer 0

/* Private function.
 * Write the bytes to the designated data port
 * lines. This function does not issue a commit
 * to the controller.
 */
void lcdWriteToPort(uint32_t hex){

	//Using Upper: PINs: 7, 6, 5, 4
	HWREG(_lcdPortDataUpper) = (hex & 0xF0) ;

	//Using Lower: PINs: 3, 2, 1, 0
	HWREG(_lcdPortDataLower) = (hex & 0x0F) ;

}

/* Private function.
 * Commits the data written to the data lines
 * to the controller.
 */
void lcdWriteCommit(){
	//Set Enable (E) line to high
	//to begin data accept.
	HWREG(_lcdPortControl) |= 0x08;

	//Wait for LCD to accept data.
	SysCtlDelay(20000);

	//Set Enable (E) to low to finish
	//data accept.
	HWREG(_lcdPortControl) &= 0xF7 ;

}

/* Commits a write to the controller as a command write.
 * (Enables the command line.) Use the constant definitions
 * for commands in the library or their equivalent command
 * values, as input to this function.
 */
void lcdWriteCommand(uint32_t hexCommand){


	//PIN2 = RS (Register Select)
	//PIN3 = E  (Enable)
	//SET RS to low
	//SET E to low
	HWREG(_lcdPortControl) &= 0xF3;

	//Write the command to port.
	lcdWriteToPort(hexCommand);

	//Commit the command.
	lcdWriteCommit();

}

/* Commits a write to the controller as data write.
 * (Enables the data line). Input to this function should
 * be the ASCII equivalent of data.
 */
void lcdWriteData(uint32_t hexData){

	//PIN2 = RS (Register Select)
	//PIN3 = E  (Enable)
	//SET RS to High
	//SET E to low
	HWREG(_lcdPortControl) |= 0x04;
	HWREG(_lcdPortControl) &= 0xF7;

	//Write the data to port.
	lcdWriteToPort(hexData);

	//Commit the data.
	lcdWriteCommit();


}
//////////////////////////////////////////
// API Layer 1

/*
 * Clear the entire LCD screen.
 */
void lcdClear(){
	lcdWriteCommand(LCD_CMD_CLEAR);
}
/*
void lcdCursorUp(){
	//Requires Read. Not Implemented.
}
void lcdCursorDown(){
	//Requires Read.
}
*/
/* Hides the cursor of the LCD.
 */
void lcdCursorHide(){
	lcdWriteCommand(LCD_CMD_CURSOR_HIDE);
}
/*Relocates the cursor of the LCD to the first
 *line, first box.
 */
void lcdCursorHome(){
	lcdWriteCommand(LCD_CMD_POSITION_UP);
}
/*Relocates the cursor of the LCD to the second
 * line, the first box.
 */
void lcdCursorHomeDown(){
	lcdWriteCommand(LCD_CMD_POSITION_DOWN);
}
/*Moves the cursor one box to the left.
 */
void lcdCursorMoveLeft(){
	lcdWriteCommand(LCD_CMD_CURSOR_MOVE_LEFT);
}
/*Moves the cursor one box to the right.
 */
void lcdCursorMoveRight(){
	lcdWriteCommand(LCD_CMD_CURSOR_MOVE_RIGHT);
}
/*Blanks the display.
 */
void lcdDisplayHide(){
	lcdWriteCommand(LCD_CMD_DISPLAY_BLANK);
}
/*Restores the display from a blank status.
 */
void lcdDisplayRestore(){
	lcdWriteCommand(LCD_CMD_DISPLAY_RESTORE);
}
/*Scrolls all of the boxes one step to the left.
 */
void lcdScrollLeft(){
	lcdWriteCommand(LCD_CMD_SCROLL_LEFT);
}
/*Scrolls all of the boxes one step to the right.
 */
void lcdScrollRight(){
	lcdWriteCommand(LCD_CMD_SCROLL_RIGHT);
}


//////////////////////////////////////////
// API Layer 2

/* Inits the LCD screen by setting the control
 * ports, the upper data port, and the lower
 * data port. The pins used for each port are
 * documented below.
 *
 * 	CONTROL_PORT = 		0x00000110
 * 	UPPER_DATA_PORT = 	0x11110000
 * 	DOWN_DATA_PORT =	0x00001111
 *
 * Just set the port to use, and the function
 * will assign these pins to the library for
 * use.
 */
void lcdInit(uint32_t __lcdPortControl,
				uint32_t __lcdPortDataUpper,
				uint32_t __lcdPortDataLower){
	_lcdPortControl = __lcdPortControl;
	_lcdPortDataUpper = __lcdPortDataUpper;
	_lcdPortDataLower = __lcdPortDataLower;
	lcdWriteCommand(LCD_CMD_FSET_2L_8B);
	lcdWriteCommand(LCD_CMD_FSET_ENTRY_INCREMENT_NOSHIFT);
	lcdWriteCommand(LCD_CMD_CURSOR_UNDERLINE_STATIC);
	lcdClear();
	_lcdInited = 1 ;
}

//////////////////////////////////////////
// API Layer 3

/* Writes a letter to the LCD and the current
 * cursor.
 */
void lcdWriteLetter(char letter){
	lcdWriteData(letter);
}

/* TODO: Fix.
 * Writes a line to the LCD at current cursor.
 */
void lcdWriteLine(char* line){
	int i = 0;
	for(i=0;line[i]!='.'&& i < 16;i++){
		//TODO:
		//Fault protection: maybe not
		//needed.
		lcdWriteLetter(line[i]%256);
	}
	while(i < 16){
		lcdWriteLetter(' ');
		i++;
	}
}

/*Writes a line at the first line.
 */
void lcdWriteLineUp(char* line){
	lcdCursorHome();
	lcdWriteLine(line);
}

/*Writes a line at the second line.
 */
void lcdWriteLineDown(char * line){
	lcdCursorHomeDown();
	lcdWriteLine(line);
}

/*Writes the given 2 lines.
 */
void lcdWriteLines(char *line1, char* line2){
	lcdCursorHome();
	lcdWriteLine(line1);
	lcdCursorHomeDown();
	lcdWriteLine(line2);
}


//////////////////////////////////////////
// API Layer 4

/* Unsafe function. Provide correct control
 * sequences. Uses '.' to stop. Once it stops
 * it leaves the cursor where it is.
 *
 */
void lcdWriteString(char* line){
	int i = 0;
	for(i=0;line[i]!='.'&& i < 16;i++){
		//TODO:
		//Fault protection: maybe not
		//needed.
		lcdWriteLetter(line[i]%256);
	}
}

void lcdClearLine(uint32_t line){
	if(line == LCD_LINE_UP){
		lcdCursorHome();
		lcdWriteLine(".");
	}
	else if(line == LCD_LINE_DOWN){
		lcdCursorHomeDown();
		lcdWriteLine(".");
	}
}
