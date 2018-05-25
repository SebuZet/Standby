/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "button.h"
#include "const.h"
#include "tasks.h"

// current state of the button
enum Buttonstate_t button01State = BUTTON_IDLE;

// lists containing listeners
struct ButtonNode* button01Listeners = NULL;

// flag which indicates state cleanup on the next dispatch
uint8_t button01IdleNext = FALSE;

void __Button01_dispatch(void)
{
  if(button01IdleNext) {
    button01State = BUTTON_IDLE;
    button01IdleNext = FALSE;
  }

  if(button01State != BUTTON_IDLE) {
    struct ButtonNode* node = button01Listeners;
    while(node) {
      node->listener(button01State);
      node = node->next;
    }

    button01IdleNext = TRUE;
  }
}

void __Button01_poll(void)
{
  if((PINC & (1 << 1)) == 0)
    button01State = BUTTON_PUSHED;
}

void Button01_init(void)
{
  DDRC |= 1 << 1;
  PORTC |= 1 << 1;

  // create tasks
  Tasks_create(100, TASK_REPEAT, __Button01_poll, NULL);
  Tasks_create(400, TASK_REPEAT, __Button01_dispatch, NULL);
}

void Button01_join(void (*listener)(enum Buttonstate_t))
{
  struct ButtonNode* node = (struct ButtonNode*)malloc(sizeof(struct ButtonNode));

  // it will be the last entry, so no next node
  node->next = NULL;

  // node values
  node->listener = listener;

  if(button01Listeners == NULL) { // no main node yet
    button01Listeners = node;
    } else {
    struct ButtonNode* parent = button01Listeners;
    while(parent->next) {
      parent = parent->next;
    }

    parent->next = node;
  }
}
