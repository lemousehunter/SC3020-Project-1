#include <Constants.h>

extern uint8_t RECORD_SIZE = 26;
extern uint16_t BLOCK_SIZE = 4096;
extern uint16_t BLOCK_HEADER_SIZE = 614;
extern uint16_t AVAILABLE_BLOCK_SIZE = BLOCK_SIZE - BLOCK_HEADER_SIZE; // 3482
extern uint16_t MAX_RECORDS_PER_BLOCK = AVAILABLE_BLOCK_SIZE  / (RECORD_SIZE + 1); // 128
extern uint16_t MAX_USED_SPACE_PER_BLOCK = MAX_RECORDS_PER_BLOCK * (RECORD_SIZE + 1) + BLOCK_HEADER_SIZE;
extern uint16_t MIN_FREE_SPACE_PER_BLOCK = BLOCK_SIZE - MAX_USED_SPACE_PER_BLOCK;
extern std::string DATABASE_FILENAME = "data.db";
extern std::string INDEX_FILENAME = "index.dat";
extern std::string DATA_FILENAME = "games.txt";

extern uint16_t BPLUSTREE_ORDER = 100; // Increased order for better performance
extern bool isEqual(float a, float b, float epsilon) {
    return std::abs(a - b) < epsilon;
};