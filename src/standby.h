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
#include "tasks.h"
#include "dev/button.h"

#define STANDBY_IGNORE_REMOTE 10

enum StandbyMode_t
{
	STANDBY_MODE_STARTING = 0,
	STANDBY_MODE_NORMAL,
	STANDBY_MODE_GOING_TO_STANDBY,
	STANDBY_MODE_LEARN_POWER_ON,
	STANDBY_MODE_LEARN_POWER_OFF,

	// const
	STANDBY_MODE_LAST = STANDBY_MODE_LEARN_POWER_OFF
};

struct Standby
{
	struct Config*		config;
	enum StandbyMode_t	mode;
	uint8_t				mustIgnoreRemote;
	struct Task*		blinkTask;
};

extern struct Standby* standby;

void Standby_init(void);

#endif
