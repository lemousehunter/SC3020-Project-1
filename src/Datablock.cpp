#include "Datablock.h"
#include <stdexcept>
#include <cstring>

Datablock::Datablock(uint32_t id, uint32_t maxSize)
    : id(id), maxSize(maxSize), currentSize(0), recordCount(0) {
    data.reserve(maxSize);
}

bool Datablock::addRecord(uint32_t recordId, const std::vector<char>& recordData) {
    if (currentSize + recordData.size() + sizeof(uint32_t) > maxSize) {
        return false;
    }
    
    recordLocations[recordId] = currentSize;
    
    uint32_t size = recordData.size();
    data.insert(data.end(), reinterpret_cast<char*>(&size), reinterpret_cast<char*>(&size) + sizeof(uint32_t));
    data.insert(data.end(), recordData.begin(), recordData.end());
    
    currentSize += sizeof(uint32_t) + recordData.size();
    recordCount++;
    
    return true;
}

std::vector<char> Datablock::getRecord(uint32_t recordId) const {
    auto it = recordLocations.find(recordId);
    if (it == recordLocations.end()) {
        throw std::runtime_error("Record not found");
    }
    
    uint32_t offset = it->second;
    uint32_t size;
    std::memcpy(&size, &data[offset], sizeof(uint32_t));
    
    return std::vector<char>(data.begin() + offset + sizeof(uint32_t), data.begin() + offset + sizeof(uint32_t) + size);
}

std::vector<char> Datablock::serialize() const {
    std::vector<char> result;
    result.reserve(sizeof(uint32_t) * 4 + currentSize + recordLocations.size() * sizeof(uint32_t) * 2);
    
    auto appendUint32 = [&result](uint32_t value) {
        result.insert(result.end(), reinterpret_cast<char*>(&value), reinterpret_cast<char*>(&value) + sizeof(uint32_t));
    };
    
    appendUint32(id);
    appendUint32(maxSize);
    appendUint32(currentSize);
    appendUint32(recordCount);
    
    result.insert(result.end(), data.begin(), data.end());
    
    for (const auto& pair : recordLocations) {
        appendUint32(pair.first);
        appendUint32(pair.second);
    }
    
    return result;
}

Datablock Datablock::deserialize(const std::vector<char>& serializedData) {
    if (serializedData.size() < sizeof(uint32_t) * 4) {
        throw std::runtime_error("Invalid serialized data");
    }
    
    auto readUint32 = [&serializedData](size_t& offset) {
        uint32_t value;
        std::memcpy(&value, &serializedData[offset], sizeof(uint32_t));
        offset += sizeof(uint32_t);
        return value;
    };
    
    size_t offset = 0;
    uint32_t id = readUint32(offset);
    uint32_t maxSize = readUint32(offset);
    uint32_t currentSize = readUint32(offset);
    uint32_t recordCount = readUint32(offset);
    
    Datablock datablock(id, maxSize);
    datablock.currentSize = currentSize;
    datablock.recordCount = recordCount;
    
    datablock.data.assign(serializedData.begin() + offset, serializedData.begin() + offset + currentSize);
    offset += currentSize;
    
    for (uint32_t i = 0; i < recordCount; ++i) {
        uint32_t recordId = readUint32(offset);
        uint32_t location = readUint32(offset);
        datablock.recordLocations[recordId] = location;
    }
    
    return datablock;
}