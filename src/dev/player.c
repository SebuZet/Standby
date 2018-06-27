/*
 * Copyright (c) 2014-2018
 * Sebastian Zaorski <sebastian.zaorski@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "player.h"

/*
 * Communication with SBC (Single board computer) based player to customize Standby behavior
 * Standby is using two PINS (PD3 and PD4) for communication with SBC
 * Pin PD3 is configured as input. After starting connected SBC this pin should be set to LOW by SBC.
 * Pin PD4 is used as output, LOW level is set buy default. 
 * 
 * When user wants to power off the system (using button or RCU) then there are two possible scenarios:
 * - If SBC is not connected (PD3 input pin reads HIGH state) then system will be powered off immediately
 * - If SBC is connected (PD3 input pin reads LOW state) then PD4 pin will be set to HIGH level and system will be powered off after 20s
 */

void Player_Init(void)
{
	// pin for checking player presence
	DDRD  &= ~_BV(3);		// configure PD3 as input
	PORTD |= _BV (3);		// enable pull up resistor
	
	// configure PD0 as input (UART RX), when RPi is on, then UART TX is set to HIGH. When shutdown procedure is finished then UART TX pin is set to LOW
	// We can use this for checking if RPi is already shutdown
	// DDRD  &= ~_BV(0);	

	// pin for shutdown notification
	DDRD  |= _BV(4);
	PORTD &= ~(_BV(4));
	
	// pins for shutdown request
	DDRD  &= ~_BV(5);
	PORTD |= _BV (5);
	DDRD  &= ~_BV(6);
	PORTD |= _BV (6);
}

void Player_NotifySystemShutDown(BOOL bShutDown)
{
	if (bShutDown)
	{
		PORTD |= _BV (4);	// set HIGH level on D4
	}
	else
	{
		PORTD &= ~(_BV(4));	// set LOW level on D4
	}
}

BOOL Player_isConnected(void)
{
	return (PIND & _BV(3)) == 0; // check if D3 is in LOW level
}

BOOL Player_isShutdownRequested(void)
{
	return ((PIND & _BV(5)) == 0) && ((PIND & _BV(6)) == _BV(6)); // check if D5 is in LOW and D6 in HIGH level
}
