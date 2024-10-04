#include "Datablock.h"
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <Storage.h>

Datablock::Datablock(uint16_t id, uint16_t maxSize)
    : header{id, maxSize, 0, 0, {}} {
    data.reserve(maxSize);
}

void Datablock::printSchema() const {
    uint8_t size;
    Record record;
    std::cout << "┌──────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│             Datablock Schema (" << BLOCK_SIZE << " bytes)            │" << std::endl;
    std::cout << "╞══════════════════════════════════════════════════════╡" << std::endl;
    std::cout << "│Header                                 " << unsigned(4 * sizeof(uint16_t) + sizeof(header.recordLocations)) << " bytes" << "       │" << std::endl;
    std::cout << "├────────────────┬─────────────────────────────────────┤" << std::endl;
    std::cout << "│id              │      " << type_name<decltype(header.id)>() << "  " << unsigned(sizeof(uint16_t)) << "  bytes" << "       │" << std::endl;
    std::cout << "│maxSize         │      " << type_name<decltype(header.maxSize)>() << "  " << unsigned(sizeof(uint16_t)) << "  bytes" << "       │" << std::endl;
    std::cout << "│currentSize     │      " << type_name<decltype(header.currentSize)>() << "  " << unsigned(sizeof(uint16_t)) << "  bytes" << "       │" << std::endl;
    std::cout << "│recordCount     │      " << type_name<decltype(header.recordCount)>() << "  " << unsigned(sizeof(uint16_t)) << "  bytes" << "       │"<< std::endl;
    std::cout << "│recordLocations │      " << "unordered map" << "   " << sizeof(header.recordLocations) << " bytes" << "       │" << std::endl;
    std::cout << "╞════════════════╧═════════════════════════════════════╡" << std::endl;
    std::cout << "│Record                                 " << sizeof(size) << "  bytes" << "       │" << std::endl;
    std::cout << "╞══════════════════════════════════════════════════════╡" << std::endl;
    std::cout << "│Record Header                          " << sizeof(size) << "  bytes" << "       │" << std::endl;
    std::cout << "├────────────────┬─────────────────────────────────────┤" << std::endl;
    std::cout << "│size            │      " << type_name<decltype(record.gameDate)>() << "             " << sizeof(size) << "  bytes" << "       │" << std::endl;
    std::cout << "╞════════════════╪═════════════════════════════════════╡" << std::endl;
    std::cout << "│gameDate        │      " << type_name<decltype(record.gameDate)>() << "             " << sizeof(record.gameDate) << "  bytes" << "       │" << std::endl;
    std::cout << "│teamId          │      " << type_name<decltype(record.teamId)>() << "             " << sizeof(record.teamId) << "  bytes" << "       │"  << std::endl;
    std::cout << "│ptsHome         │      " << type_name<decltype(record.ptsHome)>() << "   " << sizeof(record.ptsHome) << "  bytes" << "       │" << std::endl;
    std::cout << "│fgPctHome       │      " << type_name<decltype(record.fgPctHome)>() << "           " << sizeof(record.fgPctHome) << "  bytes" << "       │" << std::endl;
    std::cout << "│ftPctHome       │      " << type_name<decltype(record.ftPctHome)>() << "           " << sizeof(record.ftPctHome) << "  bytes" << "       │" << std::endl;
    std::cout << "│fg3PctHome      │      " << type_name<decltype(record.fg3PctHome)>() << "           " << sizeof(record.fg3PctHome) << "  bytes" << "       │" << std::endl;
    std::cout << "│astHome         │      " << type_name<decltype(record.astHome)>() << "   " << sizeof(record.astHome) << "  bytes" << "       │" << std::endl;
    std::cout << "│rebHome         │      " << type_name<decltype(record.rebHome)>() << "   " << sizeof(record.rebHome) << "  bytes" << "       │" << std::endl;
    std::cout << "│homeTeamWins    │      " << type_name<decltype(header.id)>() << "  " << sizeof(record.homeTeamWins) << "  bytes" << "       │" << std::endl;
    std::cout << "│recordId        │      " << type_name<decltype(header.id)>() << "  " << sizeof(record.recordId) << "  bytes" << "       │" << std::endl;
    std::cout << "╞════════════════╧═════════════════════════════════════╡" << std::endl;
    std::cout << "│                          ...                         │" << std::endl;
    std::cout << "│                          ...                         │" << std::endl;
    std::cout << "│                      Other Records                   │" << std::endl;
    std::cout << "│                          ...                         │" << std::endl;
    std::cout << "│                          ...                         │" << std::endl;
    std::cout << "└──────────────────────────────────────────────────────┘\n\n" << std::endl;

    

}

bool Datablock::addRecord(uint16_t recordId, const std::vector<char>& recordData) {
    if (header.currentSize + RECORD_SIZE + sizeof(uint8_t) > header.maxSize) {
        return false;
    }
    
    header.recordLocations[recordId] = header.currentSize;
    
    uint8_t size = RECORD_SIZE;
    data.insert(data.end(), reinterpret_cast<char*>(&size), reinterpret_cast<char*>(&size) + sizeof(uint8_t));
    data.insert(data.end(), recordData.begin(), recordData.end());
    
    header.currentSize += sizeof(uint8_t) + RECORD_SIZE;
    header.recordCount++;
    
    return true;
}


std::vector<char> Datablock::getRecord(uint16_t recordId) const {
    auto it = header.recordLocations.find(recordId);
    if (it == header.recordLocations.end()) {
        throw std::runtime_error("Record not found");
    }
    
    uint16_t offset = it->second;
    uint8_t size;
    std::memcpy(&size, &data[offset], sizeof(uint8_t));
    
    return std::vector<char>(data.begin() + offset + sizeof(uint8_t), data.begin() + offset + sizeof(uint8_t) + size);
}

std::vector<char> Datablock::serialize() const {
    std::vector<char> result;
    result.reserve(sizeof(Header) + header.currentSize);
    
    auto appendUint16 = [&result](uint16_t value) {
        result.insert(result.end(), reinterpret_cast<char*>(&value), reinterpret_cast<char*>(&value) + sizeof(uint16_t));
    };
    
    appendUint16(header.id);
    appendUint16(header.maxSize);
    appendUint16(header.currentSize);
    appendUint16(header.recordCount);
    
    appendUint16(header.recordLocations.size());
    for (const auto& pair : header.recordLocations) {
        appendUint16(pair.first);
        appendUint16(pair.second);
    }
    
    result.insert(result.end(), data.begin(), data.end());
    
    return result;
}

Datablock Datablock::deserialize(const std::vector<char>& serializedData) {
    if (serializedData.size() < sizeof(uint16_t) * 5) {
        throw std::runtime_error("Invalid serialized data");
    }
    
    auto readUint16 = [&serializedData](size_t& offset) {
        uint16_t value;
        std::memcpy(&value, &serializedData[offset], sizeof(uint16_t));
        offset += sizeof(uint16_t);
        return value;
    };
    
    size_t offset = 0;
    uint16_t id = readUint16(offset);
    uint16_t maxSize = readUint16(offset);
    uint16_t currentSize = readUint16(offset);
    uint16_t recordCount = readUint16(offset);
    
    Datablock datablock(id, maxSize);
    datablock.header.currentSize = currentSize;
    datablock.header.recordCount = recordCount;
    
    uint16_t mapSize = readUint16(offset);
    for (uint16_t i = 0; i < mapSize; ++i) {
        uint16_t recordId = readUint16(offset);
        uint16_t location = readUint16(offset);
        datablock.header.recordLocations[recordId] = location;
    }
    
    datablock.data.assign(serializedData.begin() + offset, serializedData.end());
    
    return datablock;
}