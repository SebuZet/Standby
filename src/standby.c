/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "standby.h"
#include "config.h"
#include "dev/led.h"
#include "dev/relay.h"

// standby structure with all the neccessary information
struct Standby* standby = NULL;

void Standby_init(void)
{
  // create standby structure
  standby = (struct Standby*)malloc(sizeof(struct Standby));

  // zero-fill
  memset(standby, 0, sizeof(struct Standby));

  // load configuration
  standby->config = Config_get();

  // normal mode
  standby->mode = STANDBY_MODE_NORMAL;

  // update LEDs
  Standby_sync();

  // button
  Button01_join(Standby_button);

  // private tasks
  Tasks_create(100, TASK_REPEAT, Standby_remote, NULL);
  Tasks_create(4000, TASK_REPEAT, Standby_update, NULL);

  // public tasks
  standby->timeoutTask = Tasks_create(10000, TASK_REPEAT, Standby_timeout, NULL);
  standby->blinkTask = Tasks_create(500, TASK_REPEAT, Standby_blink, NULL);
  standby->modeTask = Tasks_create(1500, TASK_REPEAT, Standby_mode, NULL);
}

void Standby_blink(void)
{
  if(standby->mode == STANDBY_MODE_LEARN)
    LED_toggle(LED_MAIN);
  else
    LED_off(LED_MAIN);
}

void Standby_update(void)
{
  if(standby->mustSaveConfig) {
    Config_save();
    standby->mustSaveConfig = FALSE;
  }
}

void Standby_timeout(void)
{
  if(standby->mode != STANDBY_MODE_NORMAL)
    standby->mode = STANDBY_MODE_NORMAL;
}

void Standby_mode(void)
{
  if(standby->buttonClicks) {
    switch(standby->buttonClicks) {
      case 1: {
        Standby_toggle();
        break;
      }

      case 2: {
        // change to learn mode
        standby->mode = STANDBY_MODE_LEARN;

        // reset timeout task
        standby->timeoutTask->ticks = 0;

        break;
      }

      default: break;
    }

    // reset it
    standby->buttonClicks = 0;
  }
}

void Standby_button(enum Buttonstate_t state)
{
  // increase amount of clicks
  standby->buttonClicks++;

  // reset button option task
  standby->modeTask->ticks = 0;
}

void Standby_remote(void)
{
  struct RemoteCode* remoteCode = Remote_pop();

  if(standby->mustIgnoreRemote != STANDBY_IGNORE_REMOTE)
    standby->mustIgnoreRemote++;

  if(remoteCode) {
    uint16_t command = U16(remoteCode->address, remoteCode->command);

    switch(standby->mode) {
      case STANDBY_MODE_LEARN: {
        // save this remote code
        standby->config->remoteCode = command;

        // switch back to normal operation
        standby->mode = STANDBY_MODE_NORMAL;

        // save config
        standby->mustSaveConfig = TRUE;

        break;
      }

      case STANDBY_MODE_NORMAL: {
        if(command == standby->config->remoteCode) {
          if(standby->mustIgnoreRemote != STANDBY_IGNORE_REMOTE)
            return;

          // toggle relay
          Standby_toggle();

          // ignore
          standby->mustIgnoreRemote = 0;

          // blink
          LED_on(LED_MAIN);

          // reset blink timer
          standby->blinkTask->ticks = 0;
        }

        break;
      }

      default: break;
    }
  }
}

void Standby_toggle(void)
{
  // toggle relay
  Relay_toggle();

  // update LEDs
  Standby_sync();
}

void Standby_sync(void)
{
  if(Relay_get()) {
    LED_on(LED_AUX1);
    LED_off(LED_AUX2);
  } else {
    LED_off(LED_AUX1);
    LED_on(LED_AUX2);
  }
}
