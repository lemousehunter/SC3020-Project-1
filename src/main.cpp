#include <iostream>
#include <chrono>
#include <iomanip>
#include <set>
#include <vector>
#include <filesystem>
#include <fstream>
#include "Storage.h"
#include "BPlusTree.h"
#include <numeric>

void linearSearch(Storage& storage, float lower, float upper, SearchResult& result) {
    const auto& recordLocations = storage.getRecordLocations();
    std::unordered_map<uint32_t, std::vector<uint32_t>> datablockRecordIds;

    for (const auto& pair : recordLocations) {
        uint32_t recordId = pair.first;
        uint32_t datablockId = pair.second.first;
        
        Record record = storage.getRecord(recordId);
        if (record.fgPctHome >= lower && record.fgPctHome <= upper) {
            datablockRecordIds[datablockId].push_back(recordId);
            result.numberOfResults++;
        }
    }

    result.dataBlocksAccessed = datablockRecordIds.size();
    
    std::vector<Record> resulting_records;
    for (const auto& pair : datablockRecordIds) {
        auto records = storage.bulkRead(pair.second);
        resulting_records.insert(resulting_records.end(), records.begin(), records.end());
    }

    result.found_records = resulting_records;

    if (!resulting_records.empty()) {
        float totalFG3PctHome = 0.0f;
        for (const auto& record : resulting_records) {
            totalFG3PctHome += record.fg3PctHome;
        }
        result.avgFG3PctHome = totalFG3PctHome / resulting_records.size();
    }
}

int main() {
    try {
        // Task 1: Storage component
        Storage storage("database.dat");

        // Check if the database file exists
        if (!std::filesystem::exists("database.dat")) {
            std::cout << "Database file not found. Ingesting data..." << std::endl;
            storage.ingestData("games.txt");
        } else {
            std::cout << "Database file found. Loading existing data..." << std::endl;
        }

        // Task 2: B+ tree indexing
        int order = 100; // Increased order for better performance
        BPlusTree bTree(order, "index.dat");

        if (std::filesystem::exists("index.dat")) {
            std::cout << "Loading B+ tree from index file..." << std::endl;
            bTree.loadFromFile();
            bTree.verifyTree();
        } else {
            std::cout << "Building B+ tree from storage..." << std::endl;
            bTree.buildFromStorage(storage);
            bTree.verifyTree();
        }

        // Task 3: Search and comparison
        float lower = 0.5f;
        float upper = 0.8f;
        
        std::cout << "\nPerforming range search for " << lower << " <= FG_PCT_home <= " << upper << std::endl;
        
        // B+ Tree search
        auto start = std::chrono::high_resolution_clock::now();
        auto result = bTree.rangeSearch(lower, upper, storage);
        auto end = std::chrono::high_resolution_clock::now();
        auto bpTreeDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Linear search
        start = std::chrono::high_resolution_clock::now();
        SearchResult linearResult = {0, 0, 0.0f, 0};
        linearSearch(storage, lower, upper, linearResult);
        end = std::chrono::high_resolution_clock::now();
        auto linearDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Task 1: Print Storage Statistics
        std::cout << "\n\n======================= Task 1 ====================== " << std::endl;
        storage.printStatistics();

        // Task 2: Print B+ Tree Statistics
        std::cout << "\n======================= Task 2 ======================= " << std::endl;
        bTree.printStatistics();

        // Task 3 Print Results B+ Tree then Linear Search
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "\n======================= Task 3 ======================= " << std::endl;
        std::cout << "--------------- B+ Tree Search Results ---------------" << std::endl;
        std::cout << "Number of index nodes accessed (internal, non-leaf node): " << result.indexNodesAccessed << std::endl;
        std::cout << "Number of data blocks accessed: " << result.dataBlocksAccessed << std::endl;
        std::cout << "Number of results: " << result.numberOfResults << std::endl;
        std::cout << "Average FG3_PCT_home: " << result.avgFG3PctHome << std::endl;
        std::cout << "Running time: " << bpTreeDuration.count() << " microseconds" << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;

        std::cout << "\n---------------- Linear Search Results ---------------" << std::endl;
        std::cout << "Number of data blocks accessed: " << linearResult.dataBlocksAccessed << std::endl;
        std::cout << "Number of results: " << linearResult.numberOfResults << std::endl;
        std::cout << "Average FG3_PCT_home: " << linearResult.avgFG3PctHome << std::endl;
        std::cout << "Running time: " << linearDuration.count() << " microseconds" << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;
        std::cout << "======================================================\n\n" << std::endl;

        // Compare results
        if (result.numberOfResults != linearResult.numberOfResults) {
            std::cout << "\nWarning: Discrepancy in number of results!" << std::endl;
            std::cout << "B+ Tree: " << result.numberOfResults << ", Linear: " << linearResult.numberOfResults << std::endl;
        } else {
            std::cout << "\nNumber of results match between B+ Tree and Linear search." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
    }

    return 0;
}