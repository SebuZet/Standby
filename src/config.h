/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "pch.h"
#include "remote.h"

#define CONFIG_MAGIC 0x64FD
#define CONFIG_LOCATION 0x0A

struct Config {
  // remote code
  uint16_t remoteCode;

  // magic value
  uint16_t magic;
} __attribute__((packed));

void Config_init(void);
struct Config* Config_get(void);
void Config_save(void);
void Config_reset(void);

#endif
