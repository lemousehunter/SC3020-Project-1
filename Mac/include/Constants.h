#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <inttypes.h>
#include <iostream>

extern uint8_t RECORD_SIZE;
extern uint16_t BLOCK_SIZE;
extern uint16_t BLOCK_HEADER_SIZE;
extern uint16_t AVAILABLE_BLOCK_SIZE;
extern uint16_t MAX_RECORDS_PER_BLOCK;
extern uint16_t MAX_USED_SPACE_PER_BLOCK;
extern uint16_t MIN_FREE_SPACE_PER_BLOCK;
extern std::string DATABASE_FILENAME;
extern std::string INDEX_FILENAME;
extern uint16_t BPLUSTREE_ORDER;

#endif