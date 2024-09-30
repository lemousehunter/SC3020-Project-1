#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

struct Record {
    // Record attributes
    std::string gameDate;
    std::string teamId;
    int ptsHome;
    float fgPctHome;
    float ftPctHome;
    float fg3PctHome;
    int astHome;
    int rebHome;
    bool homeTeamWins;
    int datablockId;
    int recordId;

    // Record Offset from the file
    // This will allow to access to the record at O(1) time when the record block is loaded
    std::streampos fileOffset;
};

class Storage {
public:
    // Constructor
    Storage(const std::string& filename, size_t recordsPerDatablock, const std::string& datablockDir = "datablocks");


    // Methods
    void    ingestData();
    Record  getRecord(int datablockId, std::streampos fileOffset);
    void    printStatistics() const;
    size_t  getTotalRecords() const;

    std::vector<Record> getAllRecords() const;
    
    size_t  getDatablockCount() const;

    std::string getDatablockFilename(int datablockId) const;
    void getVectorByRecords(int dataBlockId, std::vector<std::streampos> fileOffsets, std::vector<Record> &resultRecords);

private:
    std::string filename;
    std::string datablockDir;
    size_t      totalRecords;
    size_t      recordsPerDatablock;
    std::vector<Record> records;  // This will store metadata for all records
    size_t      datablockCount;

    void createDatablock(const std::vector<Record>& blockRecords, int datablockId);
    void loadMetadata();
    
};

#endif // STORAGE_H