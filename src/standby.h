/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef PREAMP_H_
#define PREAMP_H_

#include "pch.h"
#include "remote.h"
#include "tasks.h"
#include "dev/button.h"

#define STANDBY_IGNORE_REMOTE 10

enum StandbyMode_t {
  STANDBY_MODE_NORMAL = 0,
  STANDBY_MODE_LEARN = 1,

  // const
  STANDBY_MODE_LAST = STANDBY_MODE_LEARN
};

struct Standby {
  struct Config* config;

  enum StandbyMode_t mode;

  uint8_t buttonClicks;

  uint8_t mustSaveConfig;
  uint8_t mustIgnoreRemote;

  struct Task* timeoutTask;
  struct Task* blinkTask;
  struct Task* modeTask;
};

extern struct Standby* standby;

void Standby_init(void);
void Standby_blink(void);
void Standby_update(void);
void Standby_timeout(void);
void Standby_mode(void);
void Standby_button(enum Buttonstate_t state);
void Standby_remote(void);

void Standby_toggle(void);
void Standby_sync(void);

#endif
