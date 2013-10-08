

#include <stdint.h>
#include <stdlib.h>
#include "inc/tm4c123gh6pm.h"
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

const int MESSAGE_SET_SIZE = 17;
char* c[] = { "Experiment 2    .", "LCD Interface   .", "with Tiva MCU  .",
		"Use the wheel   .", "to scroll up and.", "down through the.", "menu, thanks! :D.",
		"Hello World!    .", "Hello Anthony!  .", "Hello Juan!     .",
		"AeroBal Micro 2 .", "Hakuna Matata   .", "If this doesn't .",
		"work, it was    .", "Anthony's fault .", ",if it works,    .", "Juan did it! :) ." };

void switchPressed();
void init();
void nextString(int i);
void menuUp(int i);
void menuDown(int i);
void writeLetter(char letter);
void writeString(char* string);
void LcdCommandWrite(int command);
void LcdDataWrite(char letter);
void writeDataToPort(int data);
int isPressedUp();
int isPressedDown();
void delay(uint32_t amount);
void IntFinish();
void IntMaskEnable();
void interruptInit();

int cursor = 0;
int direction = 0;
int count = 0 ;

void init(){

	nextString(cursor);
	int wasPressed = 0;

	//Init Infinite Loop.
	while(1){
		//Scroll Up.
		if(isPressedUp() && !wasPressed){

			//Move up in circular array.
			cursor-- ;
			cursor = (cursor < 0) ? MESSAGE_SET_SIZE-1 : cursor;

			menuUp(cursor);

			//Delay and set software debouncing variable.
			SysCtlDelay(300000);
			wasPressed = 1;

		}
		else if(isPressedDown() && !wasPressed){
			//Move Down in circular array.
			cursor = (cursor+1)%MESSAGE_SET_SIZE;

			menuDown(cursor);

			//Delay and set software debouncing variable.
			SysCtlDelay(300000);
			wasPressed = 1 ;
		}
		else if(!isPressedDown() && !isPressedUp() && wasPressed){
			//If none is pressed.
			wasPressed = 0;
		}
	}
}
void nextString(int i){
	writeString(c[(i%MESSAGE_SET_SIZE)]);
	SysCtlDelay(200000);
	LcdCommandWrite(0xA8);
	writeString(c[(i+1)%MESSAGE_SET_SIZE]);
	SysCtlDelay(200000);
	LcdCommandWrite(0x80);

}

//Stub for modularization. Interrupt calls. Up.
void menuUp(int i){
	nextString(i);
}

//Stub for modularization. Interrupt calls. Down.
void menuDown(int i){
	nextString(i);
}

void writeLetter(char letter){
	LcdDataWrite(letter);
}

void writeString(char* string){
	int i;
	for(i = 0; (string[i] != '.')||(string[i] != '\0') ;i++){
		 LcdDataWrite(string[i]);
	}

}

void LcdCommandWrite(int command){

	//SET RS to low
	//SET E to low
	//PA2 = RS
	//PA3 = E
	GPIO_PORTA_DATA_R &= 0xF3;

	writeDataToPort(command);

	//set E line high to begin write cycle.
	GPIO_PORTA_DATA_R |= 0x08;

	//Pause to allow LCD to accept data.
	//
	// Delay for a bit
	//
	SysCtlDelay(20000);


	//Turn low to finish write cycle.
	GPIO_PORTA_DATA_R &= 0xF7;

}

void LcdDataWrite(char letter){

	//SET RS to High
	//SET E to low
	//PA2 = RS
	//PA3 = E

	GPIO_PORTA_DATA_R |= 0x04;
	GPIO_PORTA_DATA_R &= 0xF7;

	//Write.
	writeDataToPort(letter);

	//set E line high to begin write cycle.
	GPIO_PORTA_DATA_R |= 0x08;

	//Pause to allow LCD to accept data.
	//
	// Delay for a bit
	//
	SysCtlDelay(20000);

	//Turn low to finish write cycle.
	GPIO_PORTA_DATA_R &= 0xF7;

}

void writeDataToPort(int data){

	GPIO_PORTC_DATA_R &= 0x0F;
	GPIO_PORTD_DATA_R &= 0xF0;

	GPIO_PORTC_DATA_R |= (data & 0xF0) ; //C
	GPIO_PORTD_DATA_R |= (data & 0x0F) ; //D

}

//Helper Functions
int isPressedUp(){
	return (GPIO_PORTE_DATA_R & 0x04) ;
}

int isPressedDown(){
	return (GPIO_PORTE_DATA_R & 0x02);
}

// Not Yet Implemented.
void delay(uint32_t amount){
	SysCtlDelay(amount);
}

//Change to port and pin selection.
void IntFinish(){
	HWREG(0x4002441C) = 0x06 ;
}

//Change to port and pin selection.
void IntMaskEnable(){
	HWREG(0x40024410) = 0x06; //Activating Port B
	//_asm("MOV R1, R2");
}


//Init the interrupt framework.
void interruptInit(){

	 IntMasterEnable();
	 IntEnable(INT_GPIOE);
	 IntMaskEnable();
	 HWREG(0x40024408)= 0x00;
	 HWREG(0x4002440C)=0x00;
}

void writeNumber(int number){
	int n1 = number;
	int size = 1;
	while(n1 >= 10){
		n1 /=10 ;
		size++ ;
	}

	int n = size  ;
	int digits[n];
	while(n >= 0){
		digits[n] = number % 10;
		number /= 10;
		n--;
	}

	int i = 0 ;
	while(i < size){
		writeLetter(digits[i]+30);
		i++;
	}
}

void switchPressed(){

	count ++;
	LcdCommandWrite(0x80);
	writeLetter(count+48);
	SysCtlDelay(300000);
	IntFinish();
	return;

}

int main(void){
	volatile uint32_t ui32Loop;

	// There is a control register that enables interfacing with the registers
	// that contain information about the ports.
	// Enable the GPIO ports that are used for the on-board LED.
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;

	// Using Tiva example:
	// Do a dummy read to insert a few cycles after
	// enabling the peripheral.
	ui32Loop = SYSCTL_RCGC2_R;

	//DIR - Direction.
	//Set Direction for each register port.
	GPIO_PORTA_DIR_R = 0x0C;
	GPIO_PORTE_DIR_R = 0x00;
	GPIO_PORTC_DIR_R = 0xF0;
	GPIO_PORTD_DIR_R = 0x0F;



	//DEN - Digital Enable.
	//Enable digital interface with
	//these particular register ports.
	GPIO_PORTA_DEN_R = 0x0C;
	GPIO_PORTE_DEN_R = 0xFF;
	GPIO_PORTC_DEN_R = 0xF0;
	GPIO_PORTD_DEN_R = 0x0F;

	//Write Commands to Initialize LCD.
	LcdCommandWrite(0x38);
	LcdCommandWrite(0x06);
	LcdCommandWrite(0x0E);
	LcdCommandWrite(0x01);

	cursor = 0 ;

	interruptInit();
	//init();
	while(1);

}


