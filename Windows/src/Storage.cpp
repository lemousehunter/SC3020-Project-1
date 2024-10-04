#include "Storage.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>


Storage::Storage(const std::string& filename) : filename(filename), totalRecords(0) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.good()) {
        loadDatablocks();
    }
}

bool compareRecord(const Record& a, const Record& b){
    return a.fgPctHome < b.fgPctHome;
}

std::unordered_map<uint16_t, std::vector<std::pair<uint16_t, uint16_t>>> Storage::getRecordLocationsMap() const {
    std::unordered_map<uint16_t, std::vector<std::pair<uint16_t, uint16_t>>> result;

    for (const auto& datablock : datablocks) {
        uint16_t datablockId = datablock.getId();
        std::vector<std::pair<uint16_t, uint16_t>> records;

        for (const auto& [recordId, location] : datablock.getRecordLocations()) {
            records.emplace_back(recordId, location);
        }

        result[datablockId] = std::move(records);
    }

    return result;
}


void Storage::ingestData(const std::string& inputFilename) {

    /*
    Create an instance of input file stream object (std::ifstream) named inputFile
    and we passed in the input file path to open the file
    */

    std::ifstream inputFile(inputFilename);

    if (!inputFile.is_open()) {
        throw std::runtime_error("Unable to open input file: " + inputFilename);
    }

    std::string line;
    std::getline(inputFile, line); // Skip header


    std::vector<Record> fileRecords;  // A vector of records in the original file
    std::vector<Record> records;
    uint16_t recordId = 0;

    


    /*
    For each line in the file we will parse and ingest the data in each record
    */
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

        fileRecords.push_back(record); // TUAN's added code


    // START OLD CODE 1

        // records.push_back(record);

        // if (records.size() == 100) { // Adjust this number as needed
        //     createDatablock(records);
        //     records.clear();
        // }

    // END OLD CODE 1
    }

    // Sort records based on fg_pct_home
    // std::sort(records.begin(), records.end(), compareRecord);
    std::sort(fileRecords.begin(), fileRecords.end(), compareRecord);

    // for (Record sorted_record: fileRecords){
    //     std::cout << sorted_record.fgPctHome << std::endl;
    // }
    

    for (Record sorted_record: fileRecords){

        // TODO: this one could be the potential value used for the block id
        // if (sorted_record.fgPctHome < min_this_block){
        //     min_this_block = sorted_record.fgPctHome;
        // } 

        //std::cout << sorted_record.fgPctHome << std::endl;

        records.push_back(sorted_record);

        if (records.size() == 100) { // Adjust this number as needed
            createDatablock(records);
            records.clear();
        }
    }



    if (!records.empty()) {
        createDatablock(records);
    }

    totalRecords = recordId;
    datablockCount = datablocks.size();
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

    uint16_t datablockCount = datablocks.size();
    file.write(reinterpret_cast<const char*>(&datablockCount), sizeof(uint16_t));

    for (const auto& datablock : datablocks) {
        std::vector<char> serializedDatablock = datablock.serialize();
        uint16_t size = serializedDatablock.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(uint16_t));
        file.write(serializedDatablock.data(), serializedDatablock.size());
    }
}


Datablock * Storage::getDatablock(uint16_t dataBlockId){
    return &datablocks[dataBlockId];
}

std::vector<Record> Storage::getRecordsWithBlockId(uint16_t datablockId){
    Datablock * datablock = getDatablock(datablockId);
    std::vector<Record> result_record;

    //                 recordId   offset
    std::unordered_map<uint16_t, uint16_t> datablockRecordLocations = datablock->getRecordLocations();


    std::vector<char> serializedRecord;
    //             recordId   offset
    for (std::pair<uint16_t, uint16_t> datablockRecordLocation: datablockRecordLocations){
        serializedRecord = datablock->getRecord(datablockRecordLocation.first);
        result_record.push_back(deserializeRecord(serializedRecord));
    }

    return result_record;
}



void Storage::loadDatablocks() {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for reading: " + filename);
    }

    uint16_t datablockCount;
    file.read(reinterpret_cast<char*>(&datablockCount), sizeof(uint16_t));

    datablocks.clear();
    recordLocations.clear();
    totalRecords = 0;

    for (uint16_t i = 0; i < datablockCount; ++i) {
        uint16_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(uint16_t));

        std::vector<char> serializedDatablock(size);
        file.read(serializedDatablock.data(), size);

        Datablock datablock = Datablock::deserialize(serializedDatablock);
        datablocks.push_back(datablock);

        for (const auto& pair : datablock.getRecordLocations()) {
            recordLocations[pair.first] = {datablock.getId(), pair.second};
            totalRecords = std::max(totalRecords, static_cast<uint16_t>(pair.first + 1));
        }
    }

    datablockCount = datablocks.size();
}

Record Storage::getRecord(uint16_t recordId) {


    // Find the recordID
    auto it = recordLocations.find(recordId);

    if (it == recordLocations.end()) {
        throw std::runtime_error("Record not found");
    }

    uint16_t datablockId = it->second.first;
    uint16_t offset = it->second.second;

    std::vector<char> serializedRecord = datablocks[datablockId].getRecord(recordId);
    return deserializeRecord(serializedRecord);
}

std::vector<Record> Storage::bulkRead(const std::vector<uint16_t>& recordIds) {
    std::vector<Record> result;
    result.reserve(recordIds.size());

    for (uint16_t recordId : recordIds) {
        result.push_back(getRecord(recordId));
    }

    return result;
}

void Storage::printStatistics() const {
    Datablock temp_datablock = Datablock(1);
    temp_datablock.printSchema();

    std::cout << "----------------- Storage Statistics -----------------" << std::endl;
    std::cout << "Total number of records: " << totalRecords << std::endl;
    std::cout << "Number of datablocks: " << datablocks.size() << std::endl;
    std::cout << "Size of record: " << unsigned(RECORD_SIZE) << " bytes" << std::endl;
    std::cout << "Size of record (in memory): " << sizeof(Record) << " bytes" << std::endl;
    std::cout << "Size of datablock: " << unsigned(BLOCK_SIZE) << " bytes" << std::endl;
    std::cout << "Max Number of Records per Datablock: " << unsigned(MAX_RECORDS_PER_BLOCK) << std::endl;
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
    std::vector<char> result(RECORD_SIZE);
    size_t offset = 0;

    auto writeData = [&](const void* data, size_t size) {
        memcpy(result.data() + offset, data, size);
        offset += size;
    };

    writeData(&record.gameDate, sizeof(record.gameDate));
    writeData(&record.teamId, sizeof(record.teamId));
    writeData(&record.ptsHome, sizeof(record.ptsHome));
    writeData(&record.fgPctHome, sizeof(record.fgPctHome));
    writeData(&record.ftPctHome, sizeof(record.ftPctHome));
    writeData(&record.fg3PctHome, sizeof(record.fg3PctHome));
    writeData(&record.astHome, sizeof(record.astHome));
    writeData(&record.rebHome, sizeof(record.rebHome));
    writeData(&record.homeTeamWins, sizeof(record.homeTeamWins));
    writeData(&record.recordId, sizeof(record.recordId));

    return result;
}

Record Storage::deserializeRecord(const std::vector<char>& data) const {
    if (data.size() != RECORD_SIZE) {
        throw std::runtime_error("Invalid record size");
    }

    Record record;
    size_t offset = 0;

    auto readData = [&](void* dest, size_t size) {
        memcpy(dest, data.data() + offset, size);
        offset += size;
    };

    readData(&record.gameDate, sizeof(record.gameDate));
    readData(&record.teamId, sizeof(record.teamId));
    readData(&record.ptsHome, sizeof(record.ptsHome));
    readData(&record.fgPctHome, sizeof(record.fgPctHome));
    readData(&record.ftPctHome, sizeof(record.ftPctHome));
    readData(&record.fg3PctHome, sizeof(record.fg3PctHome));
    readData(&record.astHome, sizeof(record.astHome));
    readData(&record.rebHome, sizeof(record.rebHome));
    readData(&record.homeTeamWins, sizeof(record.homeTeamWins));
    readData(&record.recordId, sizeof(record.recordId));

    return record;
}