/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "led.h"

const struct LedPort ledPorts[LED_LAST + 1] =
{
	{&DDRC, &PORTC, 3}, // LED_MAIN
	{&DDRB, &PORTB, 5}, // LED_POWER_ON
	{&DDRC, &PORTC, 0}  // LED_POWER_OFF
};

void LED_init(void)
{
	for (uint8_t i = LED_FIRST; i <= LED_LAST; ++i)
	{
		*ledPorts[i].dir |= _BV(ledPorts[i].pin);
	}
}

void LED_on(enum Led_t led)
{
	*ledPorts[led].port |= _BV(ledPorts[led].pin);
}

void LED_off(enum Led_t led)
{
	*ledPorts[led].port &= ~(_BV(ledPorts[led].pin));
}

void LED_set(enum Led_t led, BOOL bOn)
{
	if (bOn)
	{
		LED_on(led);	
	}
	else
	{
		LED_off(led);
	}
}

void LED_toggle(enum Led_t led)
{
	*ledPorts[led].port ^= _BV(ledPorts[led].pin);
}

uint8_t LED_get(enum Led_t led)
{
	return (*ledPorts[led].port & _BV(ledPorts[led].pin)) == _BV(ledPorts[led].pin);
}
