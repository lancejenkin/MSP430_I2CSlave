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
/*
 * i2cslave.c
 *
 *  Created on: 21/07/2013
 *      Author: Lance Jenkin
 */

#include <msp430.h>
#include "i2cslave.h"

// Address of device
unsigned char i2c_address ;

// State of the I2C State machine
unsigned char i2c_state;
int res; // Result from call back function

// Call back functions
// function called when start condition is received
void (*m_start_callback)(void);
// function called when byte received
int (*m_rx_callback)(unsigned char);
// function called when byte is to be transmitted
int (*m_tx_callback)(int*);

// Initialise I2C module into slave mode
void i2c_slave_init(unsigned char own_address,
		void(*start_callback)(void),
		int(*rx_callback)(volatile unsigned char),
		int (*tx_callback)(int*)){
	// Set address
	i2c_address = own_address;

	// Set SDA and SCLK pins
	P1OUT |= 0xC0;

	// Enable internal pullup resistors
	P1REN |= 0xC0;

	// Set I2C to slave mode
	USICTL0 = USIPE6 + USIPE7 + USISWRST;

	// Enable I2C mode
	USICTL1 = USIIE + USII2C + USISTTIE;

	// Set Clock polarity
	USICKCTL = USICKPL;

	// Disable USIIFGCC Clear Ctrl
	// ASM SAYS USICNT = USIIFGCC;
	USICNT |= USIIFGCC;

	// Release USI for operation
	USICTL0 &= ~USISWRST;

	// Clear flag
	USICTL1 &= ~(USIIFG + USISTTIFG);

	// Initialise Sate Machine
	i2c_state = 0x00;

	// Set the callback functions
	m_start_callback = start_callback;
	m_rx_callback = rx_callback;
	m_tx_callback = tx_callback;
}

// I2C Interrupt Vector
// Uses a state machine, adapted from ASM code available at
// http://www.ti.com/general/docs/litabsmultiplefilelist.tsp?literatureNumber=slaa368a
// Rx bytes from master: State 2->4->6->8
// Tx bytes to Master: State 2->4->10->12->14
#pragma vector=USI_VECTOR
__interrupt void USI_ISR (void){

	unsigned char byte; // Byte to send / received byte

	// Check if start condition received
	if(USICTL1 & USISTTIFG){
		// got start condition
		m_start_callback();
		i2c_state = 2;
	}

	switch(i2c_state){
	case 0: // IDLE State - shouldn't be here!
		__no_operation();
		USICTL1 &= ~USIIFG;
		break;
	case 2:  // Receive slave address
		USICNT &= 0xE0;
		USICNT += 8;	// we going to receive 8 bits

		USICTL1 &= ~USISTTIFG; // Clear the start condition flag
		i2c_state = 4; // next state; (N)ack the slave address
		USICTL1 &= ~USIIFG; // Clear the interupt flag

		break;
	case 4: // (N)Ack Slave address
		// Check if salve address is our address
		if ( ((USISRL >> 1) & 0x7F) == i2c_address ){
			// Its us!
			// Do we go into read or write mode
			if (!(USISRL & 1)){
				// Receive Mode
				USICTL0 |= USIOE;
				USISRL = 0; // Acknowledge the salve address
				i2c_state = 6; // Receive a byte
			}else{
				// Transmit mode
				USICTL0 |= USIOE; // SDA = output
				USISRL = 0;  // ACK slave address
				i2c_state = 10; // TX a byte
			}
		}else{
			// Not us - send NACK
			USISRL = 0xFF; // Send NACK
			i2c_state = 16; // prepare restart of state machine
		}

		USICNT |= 1;
		USICTL1 &= ~USIIFG;
		break;
	case 6: // Receive a byte
		USICTL0 &= ~USIOE; // Set SDA to input
		USICNT |= 8; // Set bit counter to 8
		i2c_state = 8; // Go to next state, tx (N)ack
		USICTL1 &= ~USIIFG;
		break;
	case 8: //Check Data and Tx Ack ; move received data to appln. level
		byte = USISRL;
		USICTL0 |= USIOE; // SDA = output
		USISRL = 0; // Send ACK
		USICNT |= 1; // Bit counter = 1
		i2c_state = 6; // Receive the byte
		res = m_rx_callback(byte);
		if(res == TI_USI_EXIT_LPM){
			// Wake up
			_bic_SR_register_on_exit(CPUOFF);
		}
		USICTL1 &= ~USIIFG;
		break;
	case 10: // Transmit a byte
state10:
		USICTL0 |= USIOE;  // Set output enabled
		res = m_tx_callback((int*)(&byte));
		USISRL = byte; // Move the byte to the SRL
		USICNT |= 8; // Set bit counter to 8, send the byte
		i2c_state = 12; // RX (N)ack
		USICTL1 &= ~USIIFG;
		break;
	case 12: //  get (N)ACK from Master
		if(res == TI_USI_EXIT_LPM){
			// Wake up
			_bic_SR_register_on_exit(CPUOFF);
		}
		USICTL0 &= ~USIOE; // SDA = input
		USICNT |= 1; // Bit counter to 1
		i2c_state = 14; // next state to test (N)ack
		USICTL1 &= ~USIIFG;
		break;
	case 14: // Test for (N)ack
		if((USISRL & 0x01)){
			// NACK rx
			i2c_state = 0; // reset state machine
			USICTL1 &= ~USIIFG;
		}else{
			i2c_state = 10;
			goto state10;
		}
		break;
	case 16: // Reset state machine on NACK
		USICTL0 &= ~USIOE;
		i2c_state = 0;
		USICTL1 &= ~USIIFG;
		break;
	}
}
