/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "relay.h"

void Relay_init(void)
{
	DDRC  |= _BV(2);
	PORTC &= ~(_BV(2));
}

void Relay_on(void)
{
	PORTC |= _BV(2);
}

void Relay_off(void)
{
	PORTC &= ~(_BV(2));
}

void Relay_toggle(void)
{
	PORTC ^= _BV(2);
}

uint8_t Relay_get(void)
{
	return (PORTC & _BV(2)) == _BV(2);
}
