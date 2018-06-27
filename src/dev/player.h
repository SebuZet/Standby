/*
 * Copyright (c) 2014-2018
 * Sebastian Zaorski <sebastian.zaorski@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef SBC_PLAYER_H_
#define SBC_PLAYER_H_

#include "pch.h"
#include "const.h"

void Player_Init(void);
void Player_NotifySystemShutDown(BOOL bShutDown);
BOOL Player_isConnected(void);
BOOL Player_isShutdownRequested(void);

#endif
