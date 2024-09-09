// File: main.cpp
#include <iostream>
#include <chrono>
#include <iomanip>
#include "StorageManager.h"
#include "BPlusTree.h"

int main() {
    const size_t BLOCK_SIZE = 4096; // Assuming a 4KB block size
    StorageManager storage_manager(BLOCK_SIZE);

    // Task 1: Load data and print storage statistics
    std::cout << "Task 1: Loading data and printing storage statistics\n";
    std::cout << "------------------------------------------------\n";
    storage_manager.load_data("games.txt");
    storage_manager.print_statistics();
    std::cout << std::endl;

    // Task 2: Build B+ tree
    std::cout << "Task 2: Building B+ tree\n";
    std::cout << "------------------------\n";
    BPlusTree bplus_tree(5); // Assuming order 5 for the B+ tree
    std::vector<Record> all_records;
    size_t num_blocks = storage_manager.get_num_blocks();
    
    for (size_t i = 0; i < num_blocks; ++i) {
        std::vector<Record> block = storage_manager.read_block(i);
        all_records.insert(all_records.end(), block.begin(), block.end());
    }

    for (auto& record : all_records) {
        bplus_tree.insert(record.fg_pct_home, &record);
    }

    bplus_tree.print_statistics();
    std::cout << std::endl;

    // Task 3: Range search
    std::cout << "Task 3: Performing range search\n";
    std::cout << "--------------------------------\n";
    float start = 0.5;
    float end = 0.8;
    
    std::cout << "Searching for records with FG_PCT_home between " << start << " and " << end << std::endl;

    auto search_start = std::chrono::high_resolution_clock::now();
    std::vector<Record*> result = bplus_tree.range_search(start, end);
    auto search_end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Number of records found: " << result.size() << std::endl;

    float sum_fg3_pct = 0;
    for (const auto& record : result) {
        sum_fg3_pct += record->fg3_pct_home;
    }
    float avg_fg3_pct = sum_fg3_pct / result.size();

    std::cout << "Average FG3_PCT_home: " << std::fixed << std::setprecision(4) << avg_fg3_pct << std::endl;
    
    auto search_duration = std::chrono::duration_cast<std::chrono::microseconds>(search_end - search_start);
    std::cout << "B+ tree search time: " << search_duration.count() << " microseconds" << std::endl;

    // Brute-force linear scan
    std::cout << "\nPerforming brute-force linear scan for comparison:\n";
    auto linear_start = std::chrono::high_resolution_clock::now();
    std::vector<Record*> linear_result;
    for (auto& record : all_records) {
        if (record.fg_pct_home >= start && record.fg_pct_home <= end) {
            linear_result.push_back(&record);
        }
    }
    auto linear_end = std::chrono::high_resolution_clock::now();
    
    auto linear_duration = std::chrono::duration_cast<std::chrono::microseconds>(linear_end - linear_start);
    std::cout << "Linear scan time: " << linear_duration.count() << " microseconds" << std::endl;
    std::cout << "Number of data blocks accessed by linear scan: " << num_blocks << std::endl;

    // Verify results
    std::cout << "\nVerifying results:\n";
    if (result.size() == linear_result.size()) {
        std::cout << "B+ tree search and linear scan found the same number of records." << std::endl;
    } else {
        std::cout << "Warning: B+ tree search and linear scan found different numbers of records!" << std::endl;
        std::cout << "B+ tree: " << result.size() << " records, Linear scan: " << linear_result.size() << " records" << std::endl;
    }

    // Calculate speedup
    double speedup = static_cast<double>(linear_duration.count()) / search_duration.count();
    std::cout << "\nSpeedup of B+ tree over linear scan: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;

    return 0;
}
