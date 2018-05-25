/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef CONST_H_
#define CONST_H_

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define BOOL int8_t
#define TRUE 1
#define FALSE 0

#define U16(x, y) (((uint16_t)x <<  8) | y)
#define U32(x, y) (((uint32_t)x << 16) | y)

#endif
