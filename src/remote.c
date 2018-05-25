/*
 * Copyright (c) 2014-2018
 * Konrad Kusnierz <iryont@gmail.com>
 *
 * All information, intellectual and technical concepts contained herein is,
 * and remains the property of the above person. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless
 * prior written permission is obtained.
 */

#include "remote.h"
#include "tasks.h"

// remote global structure
struct Remote* remote = NULL;

// LSB <=> MSB swap lookup table
static const uint8_t swap[] = {0, 8, 4, 12, 2, 10, 6, 14 , 1, 9, 5, 13, 3, 11, 7, 15};

// LSB <=> MSB swap byte
#define SWAP_BYTE(v) ((swap[(v & 0x0F)] << 4) | swap[(v & 0xF0) >> 4])

ISR(TIMER0_COMPA_vect)
{
  remote->ticks++;
}

ISR(INT0_vect)
{
  if(remote->sampling == FALSE) {
    remote->sampling = TRUE;
    remote->samples = 0;
  }

  // increase amount of samples
  remote->samples++;

  // load and update sample
  struct RemoteSample* sample = Remote_sample(remote->samples - 1);

  if(sample) {
    sample->delay = remote->ticks;
    sample->edge = (EICRA & (1 << ISC00)) ? REMOTE_EDGE_RISING : REMOTE_EDGE_FALLING;
  }

  // change edge detection
  EICRA ^= (1 << ISC00);

  // reset ticks
  remote->ticks = 0;

  // reset watchdog
  remote->watchdog->ticks = 0;
}

// Public //

void Remote_init(void)
{
  if(!remote)
    remote = (struct Remote*)malloc(sizeof(struct Remote));

  // everything is set to 0 or FALSE
  memset(remote, 0, sizeof(struct Remote));

  // define protocols
  Remote_define(REMOTEPROTOCOL_NEC, 9000, 200, Remote_parseNEC);
  Remote_define(REMOTEPROTOCOL_PNR, 8500, 200, Remote_parsePNR);
  Remote_define(REMOTEPROTOCOL_JVC, 8400, 200, Remote_parseJVC);
  Remote_define(REMOTEPROTOCOL_SAT, 8000, 200, Remote_parseSAT);
  Remote_define(REMOTEPROTOCOL_RCA, 4000, 200, Remote_parseRCA);
  Remote_define(REMOTEPROTOCOL_SIR, 2400, 200, Remote_parseSIR);
  Remote_define(REMOTEPROTOCOL_RC6, 2666, 200, Remote_parseRC6);
  Remote_define(REMOTEPROTOCOL_RC5, 889, 100, Remote_parseRC5);
  Remote_define(REMOTEPROTOCOL_NRC, 500, 80, Remote_parseNRC);
  Remote_define(REMOTEPROTOCOL_JVC, 526, 80, Remote_parseJVC);
  Remote_define(REMOTEPROTOCOL_VCR, 320, 60, Remote_parseVCR);

  // watchdog - 20 ms
  remote->watchdog = Tasks_create(20, TASK_REPEAT, Remote_watchdog, NULL);

  // INT0 PIN as input
  DDRD &= ~(1 << 2);
  PORTD &= ~(1 << 2);

  // CTC mode, clk/64
  TCCR0A = (1 << WGM01);
  TCCR0B |= (1 << CS01) | (1 << CS00);
  TIMSK0 |= (1 << OCIE0A);

  // CTC value
  OCR0A = REMOTE_TICK_RESOLUTION * REMOTE_TICK_MILLIS / 1000ULL - 1;

  // initialize counter
  TCNT0 = 0;

  // INT0 (falling edge)
  EICRA |= (1 << ISC01);

  // turn on INT0
  EIMSK |= (1 << INT0);
}

struct Remote* Remote_get(void)
{
  return remote;
}

struct RemoteCode* Remote_pop(void)
{
  if(remote->available) {
    remote->available = FALSE;
    return &remote->lastCode;
  }

  return NULL;
}

void Remote_define(enum RemoteProtocol_t protocol, uint16_t agc, uint16_t jitter, BOOL (*parser)(void))
{
  if(remote->protocols < REMOTE_MAX_PROTOCOLS) {
    remote->parsers[remote->protocols].protocol = protocol;
    remote->parsers[remote->protocols].agc = agc;
    remote->parsers[remote->protocols].jitter = jitter;
    remote->parsers[remote->protocols].parser = parser;

    remote->protocols++;
  }
}

// Private //

void Remote_parse(void)
{
  uint16_t agc = Remote_delayToNextEdge(0);

  for(uint8_t i = 0; i < remote->protocols; i++) {
    struct RemoteProtocol* proto = &remote->parsers[i];

    if(Remote_check(agc, proto->agc, proto->jitter)) {
      if(proto->parser())
        return;
    }
  }

  //LCD_goto(0, 0);
  //sprintf(g_displayBuffer, "AGC: [ %d]", Remote_delayToNextEdge(0));
  //LCD_writeText(g_displayBuffer);
}

void Remote_watchdog(void)
{
  if(remote->sampling) {
    // edge detection - falling edge
    EICRA &= ~(1 << ISC00);

    // parse possible code
    Remote_parse();

    // aint sampling no more
    remote->sampling = FALSE;
  }
}

inline uint16_t Remote_swap(uint16_t value, uint8_t bits)
{
  return (SWAP_BYTE(value >> 8) | (SWAP_BYTE(value & 0xFF) << 8)) >> (16 - bits);
}

struct RemoteSample* Remote_sample(uint8_t sample)
{
  if(sample < remote->samples) {
    return &remote->buffer[sample];
  }

  return NULL;
}

enum RemoteEdge_t Remote_edge(uint8_t edge)
{
  struct RemoteSample* sample = Remote_sample(edge);

  if(sample)
    return sample->edge;

  return REMOTE_EDGE_NONE;
}

uint16_t Remote_delayToNextEdge(uint8_t edge)
{
  struct RemoteSample* sample = Remote_sample(edge + 1); 

  if(sample)
    return sample->delay * REMOTE_TICK_RESOLUTION;

  return 0;
}

uint16_t Remote_delayBetweenEdges(uint8_t edge, uint8_t limit)
{
  uint16_t delay = 0;

  for(edge = edge + 1; edge <= limit; ++edge) {
    struct RemoteSample* sample = Remote_sample(edge);

    if(sample)
      delay += sample->delay;
  }

  return delay * REMOTE_TICK_RESOLUTION;
}

BOOL Remote_check(uint16_t delay, uint16_t expected, uint16_t jitter)
{
  return abs(delay - expected) <= jitter;
}

BOOL Remote_parseNEC(void)
{
  uint16_t space = Remote_delayToNextEdge(1);

  if(!Remote_check(space, 4500, 200)) {
    if(Remote_check(space, 2250, 200)) {
      remote->lastCode = remote->previousCode;
      remote->available = TRUE;

      return TRUE;
    }

    return FALSE;
  }

  // current edge
  uint8_t edge = 2;

  // address, address inverted, command, command inverted
  uint8_t data[4] = {0, 0, 0, 0};

  // parse 32 bits
  for(uint8_t bit = 0; bit < 32; bit++) {
    edge++;

    if(Remote_check(Remote_delayToNextEdge(edge), 1690, 200))
      data[bit >> 3] |= 1 << (bit & 7);

    edge++;
  }

  if(((data[0] & 0x7F) | (data[1] & 0x7F)) != 0x7F)
    return FALSE;

  if(((data[2] & 0x7F) | (data[3] & 0x7F)) != 0x7F)
    return FALSE;

  // save current code
  remote->lastCode.command = data[2];
  remote->lastCode.address = data[0];
  remote->lastCode.toggled = FALSE;

  // previous code
  remote->previousCode = remote->lastCode;

  // code is available
  remote->available = TRUE;

  return TRUE;
}

BOOL Remote_parsePNR(void)
{
  if(!Remote_check(Remote_delayToNextEdge(1), 4250, 200)) {
    return FALSE;
  }

  // current edge
  uint8_t edge = 2;

  // address, address inverted, command, command inverted
  uint8_t data[4] = {0, 0, 0, 0};

  // parse 32 bits
  for(uint8_t bit = 0; bit < 32; bit++) {
    edge++;

    if(Remote_check(Remote_delayToNextEdge(edge), 1594, 200))
      data[bit >> 3] |= 1 << (bit & 7);

    edge++;
  }

  if(((data[0] & 0x7F) | (data[1] & 0x7F)) != 0x7F)
    return FALSE;

  if(((data[2] & 0x7F) | (data[3] & 0x7F)) != 0x7F)
    return FALSE;

  // save current code
  remote->lastCode.command = data[2];
  remote->lastCode.address = data[0];
  remote->lastCode.toggled = FALSE;

  // code is available
  // something is wrong, perhaps test codes?
  // remote->available = TRUE;
  return TRUE;
}

BOOL Remote_parseSAT(void)
{
  if(!Remote_check(Remote_delayToNextEdge(1), 4000, 200))
    return FALSE;

  // current edge
  uint8_t edge = 2;

  // payload
  uint8_t address = 0;
  uint8_t command = 0;

  // parse 8 bits of address
  for(uint8_t bit = 0; bit < 8; bit++) {
    edge++;

    if(Remote_check(Remote_delayToNextEdge(edge), 1474, 200))
      address |= 1 << bit;

    edge++;
  }

  // skip one edge of end pulse (8-bit address)
  edge++;

  // Space between address and command
  if(!Remote_check(Remote_delayToNextEdge(edge), 4000, 200))
    return FALSE;

  // skip one edge of start pulse (8-bit command)
  edge++;

  // parse 8 bits of command
  for(uint8_t bit = 0; bit < 8; bit++) {
    edge++;

    if(Remote_check(Remote_delayToNextEdge(edge), 1474, 200))
      command |= 1 << bit;

    edge++;
  }

  // save current code
  remote->lastCode.command = command;
  remote->lastCode.address = address;
  remote->lastCode.toggled = FALSE;

  // code is available
  remote->available = TRUE;
  return TRUE;
}

BOOL Remote_parseRCA(void)
{
  if(!Remote_check(Remote_delayToNextEdge(1), 4000, 200))
    return FALSE;

  // amount of bits to parse
  const uint8_t bits = 24;

  // current edge
  uint8_t edge = 2;

  // 4-bit address, 8-bit command, 4-bit inverted address, 8-bit inverted command
  uint32_t payload = 0;

  // parse 24 bits
  for(uint8_t bit = 0; bit < bits; bit++) {
    edge++;

    if(Remote_check(Remote_delayToNextEdge(edge), 2000, 200))
      payload |= 1 << (bits - 1 - bit);

    edge++;
  }

  uint8_t addressNormal = (payload & 0x00F00000) >> 20;
  uint8_t commandNormal = (payload & 0x000FF000) >> 12;
  uint8_t addressInvert = (payload & 0x00000F00) >> 8;
  uint8_t commandInvert = (payload & 0x000000FF);

  if(((addressNormal & 0x0F) | (addressInvert & 0x0F)) != 0x0F)
    return FALSE;

  if((commandNormal | commandInvert) != 0xFF)
    return FALSE;

  // save current code
  remote->lastCode.command = commandNormal;
  remote->lastCode.address = addressNormal;
  remote->lastCode.toggled = FALSE;

  // previous code
  remote->previousCode = remote->lastCode;

  // code is available
  remote->available = TRUE;
  return TRUE;
}

BOOL Remote_parseSIR(void)
{
  if(!Remote_check(Remote_delayToNextEdge(1), 600, 100))
    return FALSE;

  // amount of bits to parse - yet to be determined
  uint8_t bits = 0;

  // current edge
  uint8_t edge = 2;

  // 12-bit, 15-bit, and 20-bit versions of the protocol do exist
  uint32_t payload = 0;

  for(; edge < remote->samples; bits++, edge += 2) {
    if(Remote_check(Remote_delayToNextEdge(edge), 1200, 200))
      payload |= 1 << bits;
  }

  // device address
  if(bits == 12 || bits == 20) {
    remote->lastCode.address = (payload & 0x0F80) >> 7;
  } else {
    remote->lastCode.address = (payload & 0x7F80) >> 7;
  }

  // save current code
  remote->lastCode.command = payload & 0x7F;
  remote->lastCode.toggled = FALSE;

  // code is available
  remote->available = TRUE;
  return TRUE;
}

BOOL Remote_parseRC6(void)
{
  if(Remote_edge(1) != REMOTE_EDGE_RISING)
    return FALSE;

  // current edge
  uint8_t edge = 1;

  // header
  uint8_t header = 0;

  // parse header
  for(uint8_t bit = 0; bit < 5; bit++) {
    if(Remote_check(Remote_delayToNextEdge(edge), bit == 0 ? 889 : 444, 200))
      edge++;

    edge++;

    if(Remote_edge(edge) == REMOTE_EDGE_RISING)
      header |= 1 << bit;
  }

  // swap LSB => MSB
  header = Remote_swap(header, 5);

  // check start bit
  if(((header & 0x10) >> 4) != 1)
    return FALSE;

  // RC6 mode and trailer
  uint8_t mode = (header & 0x0E) >> 1;
  BOOL trailer = (header & 0x01) == 0x01;

  switch(mode) {
    case 0: { // Philips Consumer Electronics mode
      // payload
      uint16_t payload = 0;

      // parse bits
      for(uint8_t bit = 0; bit < 16; bit++) {
        if(Remote_check(Remote_delayToNextEdge(edge), bit == 0 ? 889 : 444, 200))
          edge++;

        edge++;

        if(Remote_edge(edge) == REMOTE_EDGE_RISING)
          payload |= 1 << bit;
      }

      // swap LSB => MSB
      payload = Remote_swap(payload, 16);

      // save current code
      remote->lastCode.command = (payload & 0xFF00) >> 8;
      remote->lastCode.address = (payload & 0x00FF);
      remote->lastCode.toggled = trailer;

      // code is available
      remote->available = TRUE;

      // nothing to do
      break;
    }

    case 6: { // OEM mode
      // customer code
      uint16_t customer = 0;

      // amount depends on the first bit (at least 8, possible 16)
      uint8_t bits = 8;

      // parse customer bits
      for(uint8_t bit = 0; bit < bits; bit++) {
        if(Remote_check(Remote_delayToNextEdge(edge), bit == 0 ? 889 : 444, 200))
          edge++;

        edge++;

        if(Remote_edge(edge) == REMOTE_EDGE_RISING) {
          if(bit == 0) { // long customer code
            bits = 16;
          }

          customer |= 1 << bit;
        }
      }

      // swap LSB => MSB
      customer = Remote_swap(customer, bits);

      switch(customer) {
        case 32783: { // Microsoft Windows Media Center
          uint16_t payload = 0;

          // parse bits
          for(uint8_t bit = 0; bit < 16; bit++) {
            if(Remote_check(Remote_delayToNextEdge(edge), bit == 0 ? 889 : 444, 200))
              edge++;

            edge++;

            if(Remote_edge(edge) == REMOTE_EDGE_RISING)
              payload |= 1 << bit;
          }

          // swap LSB => MSB
          payload = Remote_swap(payload, 16);        

           // save current code
          remote->lastCode.command = (payload & 0x00FF);
          remote->lastCode.address = (payload & 0x7F00) >> 8;
          remote->lastCode.toggled = (payload & 0x8000) == 0x8000;

          // code is available
          remote->available = TRUE;

          break;
        }

        default: break;
      }

      break;
    }

    default: break;
  }

  return remote->available;
}

BOOL Remote_parseRC5(void)
{
  if(Remote_edge(0) != REMOTE_EDGE_FALLING)
    return TRUE;

  uint8_t commandExtendedBit = Remote_edge(2) == REMOTE_EDGE_RISING;

  uint8_t address = 0;
  uint8_t command = 0;
  uint8_t toggled = 0;

  // current edge
  uint8_t edge = 2;

  // parse toggle bit + address (5 bits)
  for(uint8_t bit = 0; bit < 6; bit++) {
    if(Remote_check(Remote_delayToNextEdge(edge), 889, 200))
      edge++;

    edge++;

    if(Remote_edge(edge) == REMOTE_EDGE_FALLING) {
      if(bit == 0)
        toggled |= 1;
      else
        address |= 1 << (bit - 1);
    }
  }

  // swap LSB => MSB
  address = Remote_swap(address, 5);

  // last bit of address as pre-bit to possible space (RC5x)
  uint8_t preBit = address & 1;

  // possible space of RC5x
  uint16_t space = Remote_delayToNextEdge(preBit ? edge + 1 : edge);

  // check for RC5x (empty space of 889 * 4 us)
  uint8_t extended = space >= 4 * 889 - 200;

  // jump to next bit edge if RC5x (ommit empty space between)
  if(extended) {
    if(preBit)
      edge++;

    if(Remote_check((space), 4 * 889, 200))
      edge += 2;

    if(Remote_check((space), 5 * 889, 200))
      edge += preBit ? 1 : 2;

    if(Remote_check((space), 6 * 889, 200))
      edge++;
  }

  // amount of bits to parse
  uint8_t bits = extended ? 12 : 6;

  // payload carrying command + data
  uint16_t payload = 0;

  for(uint8_t bit = 0; bit < bits; bit++) {
    if( !(extended && bit == 0) ) {
      if(Remote_check(Remote_delayToNextEdge(edge), 889, 200))
        edge++;

      edge++;
    }

    if(Remote_edge(edge) == REMOTE_EDGE_FALLING)
      payload |= 1 << bit;
  }

  // swap LSB => MSB
  payload = Remote_swap(payload, bits);

  if(extended) {
    command = (payload & 0x0FC0) >> 6;
  } else {
    command = payload & 0x3F;
  }

  if(commandExtendedBit) {
    command |= 1 << 7;
  }

  // save current code
  remote->lastCode.command = command;
  remote->lastCode.address = address;
  remote->lastCode.toggled = toggled;

  // code is available
  remote->available = TRUE;
  return remote->available;
}

BOOL Remote_parseNRC(void)
{
  if(!Remote_check(Remote_delayToNextEdge(1), 2500, 200) && !Remote_check(Remote_delayToNextEdge(1), 3500, 200))
    return FALSE;

  // current edge, skip start bit - it's already verified by checking the space between pre-pulse and next edge
  uint8_t edge = 3;

  // payload
  uint16_t payload = 0;

  // parse 16-bits of command + address / subcode
  for(uint8_t bit = 0; bit < 16; bit++) {
    if(Remote_check(Remote_delayToNextEdge(edge), 500, 100))
      edge++;

    edge++;

    if(Remote_edge(edge) == REMOTE_EDGE_RISING)
      payload |= 1 << bit;
  }

  uint8_t command = (payload & 0x00FF);
  uint8_t address = (payload & 0xFF00) >> 8;

  BOOL init = command == 0xFE && address == 0xFF;
  BOOL initialized = remote->previousCode.command == 0xFE && remote->previousCode.address == 0xFF;

  if(init) {
    remote->previousCode.command = command;
    remote->previousCode.address = address;    
  } else if(initialized) {
    remote->lastCode.command = command;
    remote->lastCode.address = address;
    remote->lastCode.toggled = FALSE;

    // code is available
    remote->available = TRUE;
  }

  return init || initialized;
}

BOOL Remote_parseVCR(void)
{
  // payload
  uint16_t payload = 0;

  // current edge
  uint8_t edge = 0;

  // parse 15 bits - 5 bits address + 8 bits command + 2 control bits
  for(uint8_t bit = 0; bit < 15; bit++) {
    edge++;

    if(Remote_check(Remote_delayToNextEdge(edge), 1680, 200))
      payload |= 1 << bit;

    edge++;
  }

  // save current code
  remote->lastCode.command = (payload & 0x1FE0) >> 5;
  remote->lastCode.address = (payload & 0x001F);
  remote->lastCode.toggled = FALSE;

  if((remote->previousCode.command | remote->lastCode.command) == 0xFF) {
    if(payload & 0x4000) { // check bit is 1 if it's inverted message
      remote->lastCode = remote->previousCode;
    }

    // code is available
    remote->available = TRUE;
  }

  // save as previous code
  remote->previousCode = remote->lastCode;
  return TRUE;
}

BOOL Remote_parseJVC(void)
{
  // Pre-pulse
  BOOL pulse = Remote_check(Remote_delayToNextEdge(0), 8400, 200);

  // current edge
  uint8_t edge = pulse ? 2 : 0;

  // payload
  uint16_t payload = 0;

  // parse 16-bits of address + command
  for(uint8_t bit = 0; bit < 16; bit++) {
    edge++;

    if(Remote_check(Remote_delayToNextEdge(edge), 1574, 200))
      payload |= 1 << bit;

    edge++;
  }

  // save current code
  remote->lastCode.address = (payload & 0x00FF);
  remote->lastCode.command = (payload & 0xFF00) >> 8;
  remote->lastCode.toggled = FALSE;

  // code is available
  remote->available = TRUE;
  return TRUE;
}
