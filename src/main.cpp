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

void averageResults(SearchResult &result) {
    float totalFG3PctHome;
    totalFG3PctHome = std::accumulate(result.found_records.begin(), result.found_records.end(), 0.0f,
        [](float sum, const Record& record) {
            return sum + record.fg3PctHome;
        });
        
    if (result.numberOfResults > 0) {
        result.avgFG3PctHome = totalFG3PctHome / result.numberOfResults;
    } else {
        result.avgFG3PctHome = 0;
    }
}

void linearIndexer(std::unordered_map<int, std::vector<std::streampos>> &linearIndex, Storage &storage) {
    size_t totalDatablocks = storage.getDatablockCount();
    for (size_t datablockId = 0; datablockId < totalDatablocks; ++datablockId) {
        std::string datablockFilename = storage.getDatablockFilename(datablockId);
        std::ifstream datablockFile(datablockFilename, std::ios::binary);
        if (!datablockFile.is_open()) {
            throw std::runtime_error("Unable to open datablock file: " + datablockFilename);
        }

        // Compute file offsets for all lines
        std::vector<std::streampos> lineOffsets;
        lineOffsets.push_back(datablockFile.tellg());  // Start of file

        Record record;
        while (datablockFile.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
            lineOffsets.push_back(datablockFile.tellg());
        }

        linearIndex[datablockId] = lineOffsets;
    }
}


// Function to perform linear search
SearchResult linearSearch(const Storage& storage, float lower, float upper, std::unordered_map<int, std::vector<std::streampos>> &linearIndex) {
    SearchResult result = {0, 0, 0.0f, 0};
    std::set<int> accessedDatablocks;
    size_t totalDatablocks = storage.getDatablockCount();

    for (size_t datablockId = 0; datablockId < totalDatablocks; ++datablockId) {
        std::string datablockFilename = storage.getDatablockFilename(datablockId);
        std::ifstream datablockFile(datablockFilename, std::ios::binary);

        if (!datablockFile.is_open()) {
            throw std::runtime_error("Unable to open datablock file: " + datablockFilename);
        }

        // Reset file position to the beginning
        datablockFile.clear();
        datablockFile.seekg(0, std::ios::beg);

        std::vector<std::streampos> lineOffsets = linearIndex[datablockId];

        // Read each line based on file offset
        for (size_t i = 0; i < lineOffsets.size() - 1; ++i) {
            Record record;
            datablockFile.seekg(lineOffsets[i]);
            datablockFile.read(reinterpret_cast<char*>(&record), sizeof(Record));

            if (record.fgPctHome >= lower && record.fgPctHome <= upper) {
                result.numberOfResults++;

                result.found_records.push_back(record);
                
                if (accessedDatablocks.find(datablockId) == accessedDatablocks.end()) {
                    accessedDatablocks.insert(datablockId);
                }
            }
        }

        result.dataBlocksAccessed++;
        datablockFile.close();
    }


    return result;
}

int main() {
    try {
        // Task 1: Storage component
        size_t num_of_records_per_datablock = 100;
        Storage storage("games.txt", num_of_records_per_datablock, "datablocks");

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
        
        // Calculate Average for B+ Tree Search Results
        averageResults(result);

        // Linear Index
        std::unordered_map<int, std::vector<std::streampos>> linearIndex;
        linearIndexer(linearIndex, storage);

        // Linear search
        start = std::chrono::high_resolution_clock::now();
        auto linearResult = linearSearch(storage, lower, upper, linearIndex);
        end = std::chrono::high_resolution_clock::now();
        auto linearDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Calculate Average for Linear Search Results
        averageResults(linearResult);

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