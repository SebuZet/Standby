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

#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8)

enum Schedule_t {
  TASK_REPEAT = 0,
  TASK_SCHEDULE = 1
};

struct Task {
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
};

void Tasks_init(void);
void Tasks_poll(void);
uint16_t Tasks_size(void);
struct Task* Tasks_create(uint16_t millis, enum Schedule_t schedule, void (*listener)(void), void (*callback)(void));

#endif
