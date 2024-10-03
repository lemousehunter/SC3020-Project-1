#ifndef DATABLOCK_H
#define DATABLOCK_H

#include <vector>
#include <cstdint>
#include <unordered_map>

class Datablock {
public:
    Datablock(uint32_t id, uint32_t maxSize = 4096);
    
    bool addRecord(uint32_t recordId, const std::vector<char>& recordData);
    std::vector<char> getRecord(uint32_t recordId) const;
    
    uint32_t getId() const { return id; }
    uint32_t getSize() const { return currentSize; }
    uint32_t getRecordCount() const { return recordCount; }
    
    const std::unordered_map<uint32_t, uint32_t>& getRecordLocations() const { return recordLocations; }
    
    std::vector<char> serialize() const;
    static Datablock deserialize(const std::vector<char>& data);

private:
    uint32_t id;
    uint32_t maxSize;
    uint32_t currentSize;
    uint32_t recordCount;
    std::vector<char> data;
    std::unordered_map<uint32_t, uint32_t> recordLocations;
};

#endif // DATABLOCK_H