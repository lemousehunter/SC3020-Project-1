#include <Constants.h>

extern uint8_t RECORD_SIZE = 26;
extern uint16_t BLOCK_SIZE = 4096;
extern uint8_t BLOCK_HEADER_SIZE = 48;
extern uint16_t AVAILABLE_BLOCK_SIZE = BLOCK_SIZE - BLOCK_HEADER_SIZE;
extern uint16_t MAX_RECORDS_PER_BLOCK = AVAILABLE_BLOCK_SIZE  / (RECORD_SIZE + 1);