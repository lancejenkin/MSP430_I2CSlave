//Copyright (c) 2013, Lance Jenkin
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of Lance Jenkin nor the
//      names of its contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL LANCE JENKIN BE LIABLE FOR ANY
//DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <msp430.h> 
#include "i2cslave.h"

/*
 * main.c
 * Exampe on how to use i2slave library
 */

int ptr_rx = 0x1000;
int ptr_tx = 0x1000;

void start_callback(){
	// Toggle LED
	//P1OUT ^= 0x1;
}

int rx_callback(unsigned char rx_data){
	// Received data byte is stored in flash
  FCTL3 = FWKEY;
  FCTL1 = FWKEY+ WRT;                       // Enable flash write
  *(unsigned char*)ptr_rx = rx_data;          // Write data to flash
  ptr_rx++;                                 // Increment Rx pointer
  FCTL1 = FWKEY;                            // Disable flash write
  FCTL3 = FWKEY + LOCK;
  return TI_USI_STAY_LPM ;                  // stay in LPM
}

int tx_callback(int* tx_data_ptr){
	// Data byte to be transmitted is passed through reference to the library
	*(unsigned char*)tx_data_ptr = *(unsigned char*)ptr_tx;
	ptr_tx++;                                 // Increment tx pointer

	return TI_USI_STAY_LPM ;                  // stay in LPM
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	    if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF){
	    	while(1);                               // If calibration constants erased
	                                                // do not load, trap CPU!!
	    }
		BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
		DCOCTL = CALDCO_1MHZ;
		P1DIR |= 0x01;                            // Enable P1.0 as output
		P1OUT = 0;
		FCTL1 = FWKEY + ERASE;                    // Enable flash erase
		FCTL2 = FWKEY + FSSEL_2 + FN0;            // Flash timing setup
		FCTL3 = FWKEY;                            // Disable lock
		*(unsigned int *)0x1000 = 0;              // Dummy write to erase flash
		FCTL1 = FWKEY;
		FCTL3 = FWKEY+LOCK;                       // Diasble flash write

		unsigned char i2c_address = 0x5C;
	    i2c_slave_init(i2c_address,start_callback, rx_callback, tx_callback);

	       while(1)
	        {
	          __disable_interrupt();
	          __bis_SR_register(LPM4_bits + GIE);     // enter LPM, enable interrupts
	          __no_operation();
	        }
		return 0;
}
