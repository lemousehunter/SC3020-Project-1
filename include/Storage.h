#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Datablock.h"

struct Record {
    int gameDate;
    int teamId;
    int ptsHome;
    float fgPctHome;
    float ftPctHome;
    float fg3PctHome;
    int astHome;
    int rebHome;
    bool homeTeamWins;
    uint32_t recordId;
};

class Storage {
public:
    Storage(const std::string& filename);
    void ingestData(const std::string& inputFilename);
    Record getRecord(uint32_t recordId);
    std::vector<Record> bulkRead(const std::vector<uint32_t>& recordIds);
    void printStatistics() const;
    size_t getTotalRecords() const;
    std::vector<Record> getAllRecords() const;
    
    const std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>>& getRecordLocations() const {
        return recordLocations;
    }

private:
    std::string filename;
    std::vector<Datablock> datablocks;
    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> recordLocations; // recordId -> {datablockId, offset}
    uint32_t totalRecords;
    
    void createDatablock(const std::vector<Record>& records);
    void saveDatablocks();
    void loadDatablocks();
    std::vector<char> serializeRecord(const Record& record) const;
    Record deserializeRecord(const std::vector<char>& data) const;
};

#endif // STORAGE_H