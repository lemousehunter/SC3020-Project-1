#include "Storage.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

Storage::Storage(const std::string& filename) : filename(filename), totalRecords(0) {
    std::ifstream file(filename, std::ios::binary);
    if (file.good()) {
        loadDatablocks();
    }
}

void Storage::ingestData(const std::string& inputFilename) {
    std::ifstream inputFile(inputFilename);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Unable to open input file: " + inputFilename);
    }

    std::string line;
    std::getline(inputFile, line); // Skip header

    std::vector<Record> records;
    uint32_t recordId = 0;

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        std::string token;
        Record record;

        record.recordId = recordId++;

        std::getline(iss, token, '\t');
        token.erase(std::remove(token.begin(), token.end(), '/'), token.end());
        record.gameDate = std::stoi(token);

        std::getline(iss, token, '\t');
        record.teamId = std::stoi(token);

        std::getline(iss, token, '\t');
        record.ptsHome = token.empty() ? 0 : std::stoi(token);

        std::getline(iss, token, '\t');
        record.fgPctHome = token.empty() ? 0.0f : std::stof(token);

        std::getline(iss, token, '\t');
        record.ftPctHome = token.empty() ? 0.0f : std::stof(token);

        std::getline(iss, token, '\t');
        record.fg3PctHome = token.empty() ? 0.0f : std::stof(token);

        std::getline(iss, token, '\t');
        record.astHome = token.empty() ? 0 : std::stoi(token);

        std::getline(iss, token, '\t');
        record.rebHome = token.empty() ? 0 : std::stoi(token);

        std::getline(iss, token, '\t');
        record.homeTeamWins = token == "1";

        records.push_back(record);

        if (records.size() == 100) { // Adjust this number as needed
            createDatablock(records);
            records.clear();
        }
    }

    // Sort records based on fg_pct_home
    std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
        return a.fgPctHome < b.fgPctHome;
    });



    if (!records.empty()) {
        createDatablock(records);
    }

    totalRecords = recordId;
    saveDatablocks();
}

void Storage::createDatablock(const std::vector<Record>& records) {
    Datablock datablock(datablocks.size());
    
    for (const auto& record : records) {
        std::vector<char> serializedRecord = serializeRecord(record);
        if (!datablock.addRecord(record.recordId, serializedRecord)) {
            datablocks.push_back(datablock);
            datablock = Datablock(datablocks.size());
            if (!datablock.addRecord(record.recordId, serializedRecord)) {
                throw std::runtime_error("Record too large for datablock");
            }
        }
        recordLocations[record.recordId] = {datablock.getId(), datablock.getRecordLocations().at(record.recordId)};
    }
    
    datablocks.push_back(datablock);
}

void Storage::saveDatablocks() {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }

    uint32_t datablockCount = datablocks.size();
    file.write(reinterpret_cast<const char*>(&datablockCount), sizeof(uint32_t));

    for (const auto& datablock : datablocks) {
        std::vector<char> serializedDatablock = datablock.serialize();
        uint32_t size = serializedDatablock.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
        file.write(serializedDatablock.data(), serializedDatablock.size());
    }
}

void Storage::loadDatablocks() {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for reading: " + filename);
    }

    uint32_t datablockCount;
    file.read(reinterpret_cast<char*>(&datablockCount), sizeof(uint32_t));

    datablocks.clear();
    recordLocations.clear();
    totalRecords = 0;

    for (uint32_t i = 0; i < datablockCount; ++i) {
        uint32_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));

        std::vector<char> serializedDatablock(size);
        file.read(serializedDatablock.data(), size);

        Datablock datablock = Datablock::deserialize(serializedDatablock);
        datablocks.push_back(datablock);

        for (const auto& pair : datablock.getRecordLocations()) {
            recordLocations[pair.first] = {datablock.getId(), pair.second};
            totalRecords = std::max(totalRecords, pair.first + 1);
        }
    }
}

Record Storage::getRecord(uint32_t recordId) {
    auto it = recordLocations.find(recordId);
    if (it == recordLocations.end()) {
        throw std::runtime_error("Record not found");
    }

    uint32_t datablockId = it->second.first;
    uint32_t offset = it->second.second;

    std::vector<char> serializedRecord = datablocks[datablockId].getRecord(recordId);
    return deserializeRecord(serializedRecord);
}

std::vector<Record> Storage::bulkRead(const std::vector<uint32_t>& recordIds) {
    std::vector<Record> result;
    result.reserve(recordIds.size());

    for (uint32_t recordId : recordIds) {
        result.push_back(getRecord(recordId));
    }

    return result;
}

void Storage::printStatistics() const {
    const size_t BLOCK_SIZE = 4096;
    size_t recordSize = sizeof(Record);
    size_t maxRecordsPerBlock = BLOCK_SIZE / recordSize;
    std::cout << "----------------- Storage Statistics -----------------" << std::endl;
    std::cout << "Total number of records: " << totalRecords << std::endl;
    std::cout << "Number of datablocks: " << datablocks.size() << std::endl;
    std::cout << "Size of record: " << sizeof(Record) << " bytes" << std::endl;
    std::cout << "Max Number of Records per Datablock: " << maxRecordsPerBlock << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
}

size_t Storage::getTotalRecords() const {
    return totalRecords;
}

std::vector<Record> Storage::getAllRecords() const {
    std::vector<Record> allRecords;
    allRecords.reserve(totalRecords);

    for (const auto& datablock : datablocks) {
        for (const auto& pair : datablock.getRecordLocations()) {
            std::vector<char> serializedRecord = datablock.getRecord(pair.first);
            allRecords.push_back(deserializeRecord(serializedRecord));
        }
    }

    return allRecords;
}

std::vector<char> Storage::serializeRecord(const Record& record) const {
    std::vector<char> result(sizeof(Record));
    std::memcpy(result.data(), &record, sizeof(Record));
    return result;
}

Record Storage::deserializeRecord(const std::vector<char>& data) const {
    if (data.size() != sizeof(Record)) {
        throw std::runtime_error("Invalid record size");
    }
    Record record;
    std::memcpy(&record, data.data(), sizeof(Record));
    return record;
}