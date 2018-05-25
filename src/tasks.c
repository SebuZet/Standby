/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "tasks.h"

// list containing tasks
struct Task* tasks = NULL;
struct Task* current = NULL;

ISR(TIMER1_COMPA_vect)
{
  struct Task* node = tasks;
  while(node) {
    if(!node->running && ++node->ticks >= node->millis) {
      // clear total amount of ticks
      node->ticks = 0;

      // lock the task for execution
      node->running = TRUE;
    }

    node = node->next;
  }
}

void Tasks_init(void)
{
  OCR1AH = (uint8_t)(CTC_MATCH_OVERFLOW >> 8);
  OCR1AL = (uint8_t)(CTC_MATCH_OVERFLOW);

  // CTC mode, Clock/8
  TCCR1B |= (1 << WGM12) | (1 << CS11);

  // initialize counter
  TCNT1 = 0;

  // enable the compare match interrupt
  TIMSK1 |= (1 << OCIE1A);
}

void Tasks_poll(void)
{
  struct Task* node = tasks;
  struct Task* last = node;

  while(node) {
    if(node->running) {
      if(node->listener) {
        current = node;
        node->listener();
        current = NULL;
      }

      // scheduled task is one time task
      if(node->schedule == TASK_SCHEDULE) {
        if(node->callback) {
          node->callback();
        }

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          if(tasks == node) {
            tasks = node->next;
            } else {
            last->next = node->next;
          }
        }

        // next node
        struct Task* next = node->next;
        free(node);
        node = next;
        continue;
      }

      // the task has been executed
      node->running = FALSE;
    }

    last = node;
    node = node->next;
  }
}

uint16_t Tasks_size(void)
{
  // initialize variable to 0
  uint16_t n = 0;

  struct Task* node = tasks;  
  while(node) {
    n++;
    node = node->next;
  }

  return n;
}

struct Task* Tasks_create(uint16_t millis, enum Schedule_t schedule, void (*listener)(void), void (*callback)(void))
{
  struct Task* node = (struct Task*)malloc(sizeof(struct Task));

  // it will be the last entry, so no next node
  node->next = NULL;

  // node values
  node->millis = millis;
  node->schedule = schedule;
  node->listener = listener;
  node->callback = callback;

  // tmp variable as comparator
  node->ticks = 0;
  node->running = FALSE;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if(current && current->callback) {
      node->callback = current->callback;
      current->callback = NULL;
    }

    if(tasks == NULL) { // no main node yet
      tasks = node;
    } else {
      struct Task* parent = tasks;
      while(parent->next) {
        parent = parent->next;
      }

      parent->next = node;
    }
  }

  return node;
}
