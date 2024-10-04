#ifndef DATABLOCK_H
#define DATABLOCK_H

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <Constants.h>

#include <type_traits>
#include <typeinfo>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif
#include <memory>
#include <string>
#include <cstdlib>

template <class T>
std::string
type_name()
{
    typedef typename std::remove_reference<T>::type TR;
    std::unique_ptr<char, void(*)(void*)> own
           (
#ifndef _MSC_VER
                abi::__cxa_demangle(typeid(TR).name(), nullptr,
                                           nullptr, nullptr),
#else
                nullptr,
#endif
                std::free
           );
    std::string r = own != nullptr ? own.get() : typeid(TR).name();
    if (std::is_const<TR>::value)
        r += " const";
    if (std::is_volatile<TR>::value)
        r += " volatile";
    if (std::is_lvalue_reference<T>::value)
        r += "&";
    else if (std::is_rvalue_reference<T>::value)
        r += "&&";
    return r;
}

class Datablock {
public:
    Datablock(uint16_t id, uint16_t maxSize = 4096);
    
    bool addRecord(uint16_t recordId, const std::vector<char>& recordData);
    std::vector<char> getRecord(uint16_t recordId) const;
    
    uint16_t getId() const { return header.id; }
    uint16_t getSize() const { return header.currentSize; }
    uint16_t getRecordCount() const { return header.recordCount; }
    
    const std::unordered_map<uint16_t, uint16_t>& getRecordLocations() const { return header.recordLocations; }
    void printSchema() const;
    
    std::vector<char> serialize() const;
    static Datablock deserialize(const std::vector<char>& data);

private:
    struct Header {
        uint16_t id;
        uint16_t maxSize;
        uint16_t currentSize;
        uint16_t recordCount;

        //                record_id  offset
        std::unordered_map<uint16_t, uint16_t> recordLocations;
    };

    Header header;
    std::vector<char> data;
};

#endif // DATABLOCK_H