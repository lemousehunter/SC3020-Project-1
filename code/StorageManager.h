// File: StorageManager.h
#pragma once
#include <vector>
#include <fstream>
#include "Record.h"

class StorageManager {
private:
    std::vector<Record> records;
    size_t block_size;
    size_t records_per_block;

public:
    StorageManager(size_t block_size);
    void load_data(const std::string& filename);
    void save_to_disk(const std::string& filename);
    std::vector<Record> read_block(size_t block_number);
    void print_statistics();
    size_t get_num_blocks() const;
};
