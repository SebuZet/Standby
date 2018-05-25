/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#ifndef REMOTE_H_
#define REMOTE_H_

#include "pch.h"
#include "const.h"

#define REMOTE_TICK_MILLIS (F_CPU / 64ULL / 1000ULL)
#define REMOTE_BUFFER_SIZE 70
#define REMOTE_TICK_RESOLUTION 16
#define REMOTE_MAX_PROTOCOLS 12

enum RemoteEdge_t {
  REMOTE_EDGE_NONE = 0,
  REMOTE_EDGE_RISING,
  REMOTE_EDGE_FALLING
};

enum RemoteProtocol_t {
  REMOTEPROTOCOL_NEC = 0, // NEC Protocol
  REMOTEPROTOCOL_PNR,     // Pioneer protocol
  REMOTEPROTOCOL_SAT,     // X-Sat/Mitsubishi Protocol
  REMOTEPROTOCOL_RCA,     // RCA Protocol
  REMOTEPROTOCOL_SIR,     // Sony SIRC Protocol
  REMOTEPROTOCOL_RC6,     // Philips RC-6
  REMOTEPROTOCOL_RC5,     // Philips RC-5
  REMOTEPROTOCOL_NRC,     // Nokia NRC17 Protocol
  REMOTEPROTOCOL_VCR,     // Sharp Protocol
  REMOTEPROTOCOL_JVC      // JVC Protocol
};

struct RemoteCode {
  uint8_t command;
  uint8_t address;
  BOOL toggled;
};

struct RemoteSample {
  enum RemoteEdge_t edge;
  uint16_t delay;
};

struct RemoteProtocol {
  enum RemoteProtocol_t protocol;
  uint16_t agc;
  uint16_t jitter;
  BOOL (*parser)(void);
};

struct Remote {
  struct Task* watchdog;
  enum RemoteProtocol_t protocol;

  struct RemoteProtocol parsers[REMOTE_MAX_PROTOCOLS];
  uint8_t protocols;

  struct RemoteSample buffer[REMOTE_BUFFER_SIZE];
  uint8_t samples;
  BOOL sampling;

  struct RemoteCode lastCode;
  struct RemoteCode previousCode;
  BOOL available;

  uint16_t ticks;
};

// Public
void Remote_init(void);
struct Remote* Remote_get(void);
struct RemoteCode* Remote_pop(void);
void Remote_define(enum RemoteProtocol_t protocol, uint16_t agc, uint16_t jitter, BOOL (*parser)(void));

// Private
void Remote_parse(void);
void Remote_watchdog(void);
uint16_t Remote_swap(uint16_t value, uint8_t bits);
struct RemoteSample* Remote_sample(uint8_t sample);
enum RemoteEdge_t Remote_edge(uint8_t edge);
uint16_t Remote_delayToNextEdge(uint8_t edge);
uint16_t Remote_delayBetweenEdges(uint8_t edge, uint8_t limit);
BOOL Remote_check(uint16_t delay, uint16_t expected, uint16_t jitter);

// Parsers
BOOL Remote_parseNEC(void);
BOOL Remote_parsePNR(void);
BOOL Remote_parseSAT(void);
BOOL Remote_parseRCA(void);
BOOL Remote_parseSIR(void);
BOOL Remote_parseRC6(void);
BOOL Remote_parseRC5(void);
BOOL Remote_parseNRC(void);
BOOL Remote_parseVCR(void);
BOOL Remote_parseJVC(void);

#endif
