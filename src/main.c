/*
* Copyright (c) 2014-2018
* Konrad Kusnierz <iryont@gmail.com>
*
* All information, intellectual and technical concepts contained herein is,
* and remains the property of the above person. Dissemination of this
* information or reproduction of this material is strictly forbidden unless
* prior written permission is obtained.
*/

#include "config.h"
#include "tasks.h"
#include "standby.h"
#include "dev/button.h"
#include "dev/relay.h"
#include "dev/led.h"
#include "dev/player.h"
#include "irmp/irmp.h"

void init_interrupt(void);

int main(void)
{
	DDRB = DDRC = DDRD = 0xFF;
	PORTB = PORTC = PORTD = 0;

	Config_init();
	Button01_init();
	Relay_init();
	LED_init();
	Player_Init();
	irmp_init();

	init_interrupt();

	// enable all interrupts
	sei();

	// initialize
	Tasks_create(0, TASK_SCHEDULE, Standby_init, NULL);

	while (TRUE)
	{
		Tasks_poll();
	}
}

void init_interrupt(void)
{
	OCR1A   =  (F_CPU / F_INTERRUPTS) - 1;                                  // compare value: 1/15000 of CPU frequency
	TCCR1B  = (1 << WGM12) | (1 << CS10);                                   // switch CTC Mode on, set prescaler to 1
	TCNT1	= 0;
	TIMSK1 |= (1 << OCIE1A);
}

static const unsigned int TicksInMs = F_INTERRUPTS / 1000;
static uint_fast8_t TicksCounter = 0;

ISR(TIMER1_COMPA_vect)
{
	irmp_ISR();
	
	if (++TicksCounter >= TicksInMs)
	{
		Tasks_tick_ms();
		TicksCounter = 0;
	}
}
