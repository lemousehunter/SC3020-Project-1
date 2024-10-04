#include "Datablock.h"
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <Storage.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Unicode box-drawing characters
const char* const TOP_LEFT = "\u250C";
const char* const TOP_RIGHT = "\u2510";
const char* const BOTTOM_LEFT = "\u2514";
const char* const BOTTOM_RIGHT = "\u2518";
const char* const HORIZONTAL = "-"; //"\u2010";
const char* const VERTICAL = "\u2502";
const char* const T_DOWN = "\u252C";
const char* const T_UP = "\u2534";
const char* const T_RIGHT = "\u251C";
const char* const T_LEFT = "\u2524";
const char* const CROSS = "\u253C";

Datablock::Datablock(uint16_t id, uint16_t maxSize)
    : header{id, maxSize, 0, 0, {}} {
    data.reserve(maxSize);
}

void Datablock::printSchema() const {
#ifdef _WIN32
    // Set console output to UTF-8
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::ostringstream oss;
    uint8_t size;
    Record record;

    auto print_line = [&](const std::string& left, const std::string& middle, const std::string& right) {
        oss << left << std::string(62, *HORIZONTAL) << right << '\n';
    };

    auto print_row = [&](const std::string& label, const std::string& value, size_t bytes) {
        oss << VERTICAL << ' ' << std::left << std::setw(14) << label
            << VERTICAL << ' ' << std::left << std::setw(31) << value
            << std::right << std::setw(5) << bytes << "  bytes  " << VERTICAL << '\n';
    };

    auto print_row_long = [&](const std::string& label, const std::string& value, size_t bytes) {
        oss << VERTICAL << ' ' << std::left << std::setw(14) << label
            << VERTICAL << ' ' << std::left << std::setw(31) << value
            << std::right << std::setw(5) << bytes << " bytes  " << VERTICAL << '\n';
    };

    auto print_row_long_short = [&](const std::string& label, const std::string& value, size_t bytes) {
        oss << VERTICAL << ' ' << std::left << std::setw(14) << label
            << VERTICAL << ' ' << std::left << std::setw(31) << ' ' << value
            << std::right << std::setw(5) << bytes << " bytes  " << VERTICAL << '\n';
    };

    print_line(TOP_LEFT, T_DOWN, TOP_RIGHT);
    oss << VERTICAL << "                 Datablock Schema (" << BLOCK_SIZE << " bytes)                " << VERTICAL << '\n';
    print_line(T_RIGHT, CROSS, T_LEFT);
    print_row_long_short("Header   ", " ", 4 * sizeof(uint16_t) + sizeof(header.recordLocations));
    print_line(T_RIGHT, CROSS, T_LEFT);
    print_row("id   ", type_name<decltype(header.id)>(), sizeof(uint16_t));
    print_row("maxSize  ", type_name<decltype(header.maxSize)>(), sizeof(uint16_t));
    print_row("currentSize  ", type_name<decltype(header.currentSize)>(), sizeof(uint16_t));
    print_row("recordCount  ", type_name<decltype(header.recordCount)>(), sizeof(uint16_t));
    print_row_long("recordLocations", "unordered map", sizeof(header.recordLocations));
    print_line(T_RIGHT, CROSS, T_LEFT);
    print_row("Record  ", "", sizeof(size));
    print_line(T_RIGHT, CROSS, T_LEFT);
    print_row("Record Header ", "", sizeof(size));
    print_line(T_RIGHT, CROSS, T_LEFT);
    print_row("size  ", type_name<decltype(record.gameDate)>(), sizeof(size));
    print_line(T_RIGHT, CROSS, T_LEFT);
    print_row("gameDate  ", type_name<decltype(record.gameDate)>(), sizeof(record.gameDate));
    print_row("teamId  ", type_name<decltype(record.teamId)>(), sizeof(record.teamId));
    print_row("ptsHome  ", type_name<decltype(record.ptsHome)>(), sizeof(record.ptsHome));
    print_row("fgPctHome  ", type_name<decltype(record.fgPctHome)>(), sizeof(record.fgPctHome));
    print_row("ftPctHome  ", type_name<decltype(record.ftPctHome)>(), sizeof(record.ftPctHome));
    print_row("fg3PctHome  ", type_name<decltype(record.fg3PctHome)>(), sizeof(record.fg3PctHome));
    print_row("astHome  ", type_name<decltype(record.astHome)>(), sizeof(record.astHome));
    print_row("rebHome  ", type_name<decltype(record.rebHome)>(), sizeof(record.rebHome));
    print_row("homeTeamWins  ", type_name<decltype(record.homeTeamWins)>(), sizeof(record.homeTeamWins));
    print_row("recordId  ", type_name<decltype(record.recordId)>(), sizeof(record.recordId));
    print_line(T_RIGHT, CROSS, T_LEFT);
    oss << VERTICAL << "                            ...                               " << VERTICAL << '\n';
    oss << VERTICAL << "                            ...                               " << VERTICAL << '\n';
    oss << VERTICAL << "                        Other Records                         " << VERTICAL << '\n';
    oss << VERTICAL << "                            ...                               " << VERTICAL << '\n';
    oss << VERTICAL << "                            ...                               " << VERTICAL << '\n';
    print_line(BOTTOM_LEFT, T_UP, BOTTOM_RIGHT);

    std::cout << oss.str() << std::endl;

#ifdef _WIN32
    // Reset console output to default
    SetConsoleOutputCP(GetACP());
#endif
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