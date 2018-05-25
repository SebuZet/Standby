/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "config.h"

// config object
struct Config* config = NULL;

void Config_init(void)
{
  if(config == NULL) {
    config = (struct Config*)malloc(sizeof(struct Config));
  }  

  // wait until EEPROM is ready
  eeprom_busy_wait();

  // load config from EEPROM
  eeprom_read_block(config, (void*)CONFIG_LOCATION, sizeof(struct Config));

  // check magic value
  if(config->magic != CONFIG_MAGIC)
    Config_reset();
}

struct Config* Config_get(void)
{
  return config;
}

void Config_save(void)
{
  // wait until EEPROM is ready
  eeprom_busy_wait();

  // save config to EEPROM
  eeprom_write_block(config, (void*)CONFIG_LOCATION, sizeof(struct Config));
}

void Config_reset(void)
{
  memset(config, 0, sizeof(struct Config));

  // magic value
  config->magic = CONFIG_MAGIC;
}
