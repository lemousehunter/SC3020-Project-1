// File: StorageManager.cpp
#include "StorageManager.h"
#include <iostream>
#include <sstream>
#include <cmath>

StorageManager::StorageManager(size_t block_size) : block_size(block_size) {
    records_per_block = block_size / sizeof(Record);
}

void StorageManager::load_data(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    
    // Skip the header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        Record record;

        std::getline(iss, record.game_date_est, '\t');
        std::getline(iss, record.team_id_home, '\t');
        iss >> record.pts_home;
        iss.ignore();
        iss >> record.fg_pct_home;
        iss.ignore();
        iss >> record.ft_pct_home;
        iss.ignore();
        iss >> record.fg3_pct_home;
        iss.ignore();
        iss >> record.ast_home;
        iss.ignore();
        iss >> record.reb_home;
        iss.ignore();
        iss >> record.home_team_wins;

        records.push_back(record);
    }
}

void StorageManager::save_to_disk(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    for (const auto& record : records) {
        file.write(reinterpret_cast<const char*>(&record), sizeof(Record));
    }
}

std::vector<Record> StorageManager::read_block(size_t block_number) {
    std::vector<Record> block;
    size_t start_index = block_number * records_per_block;
    size_t end_index = std::min(start_index + records_per_block, records.size());
    
    for (size_t i = start_index; i < end_index; ++i) {
        block.push_back(records[i]);
    }
    
    return block;
}

void StorageManager::print_statistics() {
    std::cout << "Storage Component Statistics:" << std::endl;
    std::cout << "Size of a record: " << sizeof(Record) << " bytes" << std::endl;
    std::cout << "Number of records: " << records.size() << std::endl;
    std::cout << "Records per block: " << records_per_block << std::endl;
    std::cout << "Number of blocks: " << get_num_blocks() << std::endl;
}

size_t StorageManager::get_num_blocks() const {
    return std::ceil(static_cast<float>(records.size()) / records_per_block);
}
