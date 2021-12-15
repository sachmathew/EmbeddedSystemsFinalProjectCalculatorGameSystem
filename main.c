/******************************************************************************/
/*                                                                            */
/* PmodKYPD.c -- Demo for the use of the Pmod Keypad IP core                  */
/*                                                                            */
/******************************************************************************/
/* Author:   Mikel Skreen                                                     */
/* Copyright 2016, Digilent Inc.                                              */
/******************************************************************************/
/* File Description:                                                          */
/*                                                                            */
/* This demo continuously captures keypad data and prints a message to an     */
/* attached serial terminal whenever a positive edge is detected on any of    */
/* the sixteen keys. In order to receive messages, a serial terminal          */
/* application on your PC should be connected to the appropriate COM port for */
/* the micro-USB cable connection to your board's USBUART port. The terminal  */
/* should be configured with 8-bit data, no parity bit, 1 stop bit, and the   */
/* the appropriate Baud rate for your application. If you are using a Zynq    */
/* board, use a baud rate of 115200, if you are using a MicroBlaze system,    */
/* use the Baud rate specified in the AXI UARTLITE IP, typically 115200 or    */
/* 9600 Baud.                                                                 */
/*                                                                            */
/******************************************************************************/
/* Revision History:                                                          */
/*                                                                            */
/*    06/08/2016(MikelS):   Created                                           */
/*    08/17/2017(artvvb):   Validated for Vivado 2015.4                       */
/*    08/30/2017(artvvb):   Validated for Vivado 2016.4                       */
/*                          Added Multiple keypress error detection           */
/*    01/27/2018(atangzwj): Validated for Vivado 2017.4                       */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "PmodOLED.h"
#include "PmodKYPD.h"
#include "sleep.h"

void Initialize();
void CalculatorRun();
float decr(float val, int place);

PmodKYPD myKYPD;
PmodOLED myOLED;

// To change between PmodOLED and OnBoardOLED is to change Orientation
const u8 orientation = 0x0; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
const u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters

#define NUMQs	5
char questions[NUMQs][16] = {"2+2=?","56/7=?","376-12=?",".5*9=?","84+13-13=?"};
float answers[NUMQs] = {4,8,364,4.5,84};

int main(void) {
   Initialize();
   CalculatorRun();
   OLED_End(&myOLED);
   return 0;
}

// keytable is determined as follows (indices shown in Keypad position below)
// 12 13 14 15
// 8  9  10 11
// 4  5  6  7
// 0  1  2  3
#define DEFAULT_KEYTABLE "0FED789C456B123A"

void Initialize() {
   KYPD_begin(&myKYPD, XPAR_PMODKYPD_0_AXI_LITE_GPIO_BASEADDR);
   KYPD_loadKeyTable(&myKYPD, (u8*) DEFAULT_KEYTABLE);
   OLED_Begin(&myOLED, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR,XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, orientation, invert);
}

void CalculatorRun() {
   bool game = false;
   u16 keystate;
   XStatus status, last_status = KYPD_NO_KEY;
   u8 key, last_key = 'x';
   float input[2] = {0,0};
   int i = 0;
   char s[16];
   int operation = 0;
   bool point = false;
   int spaces = 1;
   srand(6);
   int current_q=rand()%NUMQs;
   int correct=0;

   Xil_Out32(myKYPD.GPIO_addr, 0xF);
   sprintf(s, "%f", input[i]);
   OLED_SetCursor(&myOLED, 0, 3);
   OLED_PutString(&myOLED, s);

   while (1) {
      // Capture state of each key
      keystate = KYPD_getKeyStates(&myKYPD);

      // Determine which single key is pressed, if any
      status = KYPD_getKeyPressed(&myKYPD, keystate, &key);
      // Print key detect if a new key is pressed or if status has changed

      if (status == KYPD_SINGLE_KEY
            && (status != last_status || key != last_key)) {
    	  OLED_SetCursor(&myOLED, 0, 1);
    	 if(!game){
    		 OLED_SetCursor(&myOLED, 0, 1);
			 switch(key){
				 case 'A': if(operation==1){
					 game=!game;input[0]=0;i=0;input[1]=0;operation=0;spaces=1;point=false;
				 }else{i=1;operation=1;spaces=1;point=false;}break; // +
				 case 'B':i=1;operation=2;spaces=1;point=false;break; // -
				 case 'C':i=1;operation=3;spaces=1;point=false;break; // *
				 case 'D':i=1;operation=4;spaces=1;point=false;break; // /
				 case 'E': switch(operation){// =
						 case 0: input[0]=0;i=0;input[1]=0;operation=0;spaces=1;point=false;break;
						 case 1: input[0]=input[0]+input[1];i=0;input[1]=0;operation=0;spaces=1;point=false;break;
						 case 2: input[0]=input[0]-input[1];i=0;input[1]=0;operation=0;spaces=1;point=false;break;
						 case 3: input[0]=input[0]*input[1];i=0;input[1]=0;operation=0;spaces=1;point=false;break;
						 case 4: input[0]=input[0]/input[1];i=0;input[1]=0;operation=0;spaces=1;point=false;break;
					 }break;
				 case 'F':point=true;break;  // .
				 default:
					 if(!point)
						input[i]=10*input[i]+key-48;
					 else
						input[i]=input[i]+decr(key-48.0,spaces++);
					 break;
			 }}
    	 else{
    		 OLED_SetCursor(&myOLED, 0, 1);
    		 switch(key){
    		 	 case 'A': if(operation==1){
					 game=!game;input[0]=0;i=0;input[1]=0;operation=0;spaces=1;point=false;
				 }else{operation=1;}break;
         	 	 case 'E': if(input[i] == answers[current_q]){
         	 		 	 correct=1;input[i]=0;spaces=1;point=false;current_q = rand()%NUMQs;
         	 	 	 }
         	 	 	 else{
         	 	 		 correct=2;input[i]=0;spaces=1;point=false;
         	 	 	 }break;
         	 	 case 'F':point=true;break;  // .
         	 	 default:
         	 		 if(!point)
         	 			input[i]=10*input[i]+key-48;
         	 	 	 else
         	 	 		input[i]=input[i]+decr(key-48.0,spaces++);
         	 		 break;
    	 }}
    	 sprintf(s, "%f", input[i]);
    	 OLED_Clear(&myOLED);
    	 if(game){
    		 OLED_SetCursor(&myOLED, 0, 1);
    	 	 OLED_PutString(&myOLED, questions[current_q]);
    	 	 if(correct==1){
    	 		OLED_SetCursor(&myOLED, 0, 0);
    	 		OLED_PutString(&myOLED, "Correct:)");
    	 	 }else if(correct==2){
    	 		OLED_SetCursor(&myOLED, 0, 0);
    	 		OLED_PutString(&myOLED, "Incorrect:(");
    	 	 }
    	 }
    	 OLED_SetCursor(&myOLED, 0, 3);
    	 OLED_PutString(&myOLED, s);
    	 last_key = key;
      } else if (status == KYPD_MULTI_KEY && status != last_status)
         xil_printf("Error: Multiple keys pressed\r\n");

      last_status = status;

      usleep(1000);
   }
}

float decr(float val, int place){
	for(int i = 0; i < place; i++)
		val=val/10;
	return val;
}
