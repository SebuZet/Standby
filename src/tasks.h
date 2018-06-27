/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef TASKS_H_
#define TASKS_H_

#include "pch.h"
#include "const.h"

enum Schedule_t
{
	TASK_REPEAT = 0,
	TASK_SCHEDULE = 1
};

struct Task
{
	// list containing all tasks
	struct Task* next;

	// node values
	uint16_t millis;
	enum Schedule_t schedule;
	
	void (*listener)(void);
	void (*callback)(void);

	// ticks related values
	uint16_t ticks;
	BOOL running;
	BOOL enabled;
};

void Tasks_tick_ms(void);

void Tasks_poll(void);
uint16_t Tasks_count(void);

struct Task* Tasks_create(uint16_t millis, enum Schedule_t schedule, void (*listener)(void), void (*callback)(void));

BOOL Tasks_disable(struct Task* pTask);
BOOL Tasks_enable(struct Task* pTask);
BOOL Tasks_restart(struct Task* pTask);

#endif
