/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "pch.h"

enum Buttonstate_t {
  BUTTON_IDLE,
  BUTTON_PUSHED,
  BUTTON_RELEASED,
  BUTTON_HOLD
};

struct ButtonNode {
  // next entry within the list
  struct ButtonNode* next;

  // listener
  void (*listener)(enum Buttonstate_t);
};

void Button01_init(void);
void Button01_join(void (*listener)(enum Buttonstate_t));

#endif
