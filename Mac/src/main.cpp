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

void getAverage(SearchResult& result) {
    if (!result.found_records.empty()) {
        float totalFG3PctHome = 0.0f;
        for (const auto& record : result.found_records) {
            totalFG3PctHome += record.fg3PctHome;
        }
        result.avgFG3PctHome = totalFG3PctHome / result.found_records.size();
    }
}

SearchResult linearSearch(Storage& storage, float lower, float upper) {
    SearchResult result;

    // Get the map of datablockID: [(recordID, recordLocation), (recordID, recordLocation)...] for the whole database
    auto recordLocationsMap = storage.getRecordLocationsMap();

    // size of map (=number of keys in map) = number of datablocks accessed
    result.dataBlocksAccessed = recordLocationsMap.size();

    std::vector<Record> resulting_records;

    // Iterate over the map
    for (const auto& [datablockID, records] : recordLocationsMap) {
        std::vector<uint16_t> recordIds;
        // For each pair of recordID and recordLocation
        for (const auto& [recordId, recordLocation] : records) {
            // Save the recordID into list of record ids for current datablock
            recordIds.push_back(recordId);
        }
        
        // bulk read all recordIDs for current datablock
        auto ingested_records = storage.bulkRead(recordIds);
        
        // iterate over ingested records from bulkRead
        for (const auto& record : ingested_records) {
            if (record.fgPctHome >= lower && record.fgPctHome <= upper) {
                resulting_records.push_back(record);
            }
        }
    }

    result.numberOfResults = resulting_records.size();
    result.found_records = resulting_records;

    return result;
}

int main() {
    try {
        // Task 1: Storage component
        Storage storage(DATABASE_FILENAME);

        // Check if the database file exists
        if (!std::filesystem::exists(DATABASE_FILENAME)) {
            std::cout << "Database file not found. Ingesting data..." << std::endl;
            storage.ingestData("games.txt");
        } else {
            std::cout << "Database file found. Loading existing data..." << std::endl;
        }

        // Task 2: B+ tree indexing
        BPlusTree bTree(BPLUSTREE_ORDER, INDEX_FILENAME);

        if (std::filesystem::exists(INDEX_FILENAME)) {
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
        getAverage(result);

        // Linear search
        start = std::chrono::high_resolution_clock::now();
        SearchResult linearResult = linearSearch(storage, lower, upper);
        getAverage(linearResult);
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