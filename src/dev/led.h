/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef LED_H_
#define LED_H_

#include "pch.h"
#include "const.h"

enum Led_t
{
	LED_FIRST = 0,

	LED_MAIN = LED_FIRST,
	LED_POWER_ON,
	LED_POWER_OFF,

	LED_LAST = LED_POWER_OFF
};

struct LedPort
{
	volatile uint8_t *dir;
	volatile uint8_t *port;
	uint8_t pin;
};

void LED_init(void);
void LED_on(enum Led_t led);
void LED_off(enum Led_t led);
void LED_set(enum Led_t led, BOOL bOn);
void LED_toggle(enum Led_t led);
uint8_t LED_get(enum Led_t led);

#endif
