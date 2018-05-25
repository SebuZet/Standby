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
#include "remote.h"
#include "tasks.h"
#include "standby.h"
#include "dev/button.h"
#include "dev/relay.h"
#include "dev/led.h"

int main(void)
{
  DDRB = DDRC = DDRD = 0xFF;
  PORTB = PORTC = PORTD = 0;

  Tasks_init();
  Config_init();
  Remote_init();
  Button01_init();
  Relay_init();
  LED_init();

  // enable all interrupts
  sei();

  // initialize
  Tasks_create(0, TASK_SCHEDULE, Standby_init, NULL);

  while(TRUE)
    Tasks_poll();
}
