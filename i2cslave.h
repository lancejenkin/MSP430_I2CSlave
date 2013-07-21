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
 * i2cslave.h
 *
 *  Created on: 21/07/2013
 *      Author: Lance Jenkin
 */

#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_

// Defines
#define TI_USI_EXIT_LPM            1
#define TI_USI_STAY_LPM            0


// Initialise I2C module into slave mode
// Parameters
//	* own_address - The address that the device responds to
//	* start_callback - Function pointer to function called on receiving start condition
//	* rx_callback - Function pointer to function called on receiving a byte
//	* tx_callback - Function pointer to function that will set next byte to transmit
void i2c_slave_init(unsigned char own_address,
		void(*sart_callback)(void),
		int(*rx_callback)(volatile unsigned char),
		int (*tx_callback)(int*));

#endif /* I2CSLAVE_H_ */
