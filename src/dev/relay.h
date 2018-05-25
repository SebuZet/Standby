/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef RELAY_H_
#define RELAY_H_

#include "pch.h"
#include "const.h"

void Relay_init(void);
void Relay_on(void);
void Relay_off(void);
void Relay_toggle(void);
uint8_t Relay_get(void);

#endif
