#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Datablock.h"

#pragma pack(push, 1);
struct Record {
    int gameDate;             // 4 bytes
    int teamId;               // 4 bytes
    uint8_t ptsHome;          // (36 ~ 168): 1 byte
    float fgPctHome;          // 4 bytes
    float ftPctHome;          // 4 bytes
    float fg3PctHome;         // 4 bytes
    uint8_t astHome;          // (6 ~ 50)  : 1 byte
    uint8_t rebHome;          // (15 ~ 72) : 1 byte
    bool homeTeamWins;        // 1 byte
    uint16_t recordId;        // 2 bytes
};

class Storage {
public:
    Storage(const std::string& filename);
    void ingestData(const std::string& inputFilename);
    Record getRecord(uint16_t recordId);
    std::vector<Record> bulkRead(const std::vector<uint16_t>& recordIds);
    void printStatistics() const;
    size_t getTotalRecords() const;
    std::vector<Record> getAllRecords() const;
    uint16_t getDatablockCount() {return datablockCount;}
    
    const std::unordered_map<uint16_t, std::pair<uint16_t, uint16_t>>& getRecordLocations() const {
        return recordLocations;
    }

    std::unordered_map<uint16_t, std::vector<std::pair<uint16_t, uint16_t>>> getRecordLocationsMap() const;

    Datablock * getDatablock(uint16_t dataBlockId);
    std::vector<Record> getRecordsWithBlockId(uint16_t datablockId);

private:
    std::string filename;
    std::vector<Datablock> datablocks;

    //                 recordId          dataBlockId recordId
    std::unordered_map<uint16_t, std::pair<uint16_t, uint16_t>> recordLocations; // recordId -> {datablockId, offset}
    uint16_t totalRecords;

    uint16_t datablockCount;
    
    void createDatablock(const std::vector<Record>& records);
    void saveDatablocks();
    void loadDatablocks();
    std::vector<char> serializeRecord(const Record& record) const;
    Record deserializeRecord(const std::vector<char>& data) const;
};

#endif // STORAGE_H