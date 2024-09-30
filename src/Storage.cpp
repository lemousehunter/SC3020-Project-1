#include "Storage.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>


// Constructor
Storage::Storage(const std::string& filename, size_t numRecordsPerDatablock, const std::string& datablockDir)
    : filename(filename), datablockDir(datablockDir), totalRecords(0), recordsPerDatablock(0), datablockCount(0) {
    
    recordsPerDatablock = numRecordsPerDatablock;

    // Print out the message
    std::cout << "Storage constructor called with filename: " << filename << " and datablockDir: " << datablockDir << std::endl;
    
    /*
    If the directory to store the data block does not exist:
        create a new directory
        ingestData()
    else:
        counting the exist data block existed in the directory and assigning it to 'datablockCount'
        loadMetadata() from existed blocks

    Because the data is loaded ALL in at the time so if the datablock directory does not exist means that there is no data
    or else it has been loaded before and we just load the metadata.
    */
    
    if (!std::filesystem::exists(datablockDir)) {
        std::cout << "Datablock directory does not exist. Creating directory and ingesting data." << std::endl;
        std::filesystem::create_directory(datablockDir);
        ingestData();
    } else {
        std::cout << "Datablock directory exists. Counting existing datablock files." << std::endl;
        for (const auto& entry : std::filesystem::directory_iterator(datablockDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".dat") {
                datablockCount++;
            }
        }
        std::cout << "Found " << datablockCount << " existing datablock files." << std::endl;
        
        // Load metadata from existing datablocks
        loadMetadata();
    }
}

bool compare_FG_PCT_home(const Record& record1, const Record& record2){
    return record1.fgPctHome < record2.fgPctHome;
}

void Storage::ingestData() {
    std::cout << "Starting data ingestion from file: " << filename << std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    std::string line;
    std::getline(file, line); // Skip header
    std::cout << "Skipped header line: " << line << std::endl;


    /*
    The idea of this initialization for the 'fileRecords' is to load all the data into 1 vector
    and sort it with 'FG_PCT_home' column in the file so that we can create a correct 'datablockId'
    in the correct sorted manner.
    */
    std::vector<Record> fileRecords;  // A vector of records in the original file


    std::vector<Record> blockRecords; // A vector of records in the Block

    int datablockId = 0;
    int recordId = 0;

    std::cout << "Ingesting data..." << std::endl;

    while (std::getline(file, line)) {
        std::istringstream iss(line);  // Initialize an input string stream called 'iss' that is assigned with value of 'line'
        std::string token;
        Record record;



        // TODO: These 2 line2 will affect the TUAN's CODE
        record.datablockId = datablockId;
        record.recordId = recordId;

        // Parse the record fields
        std::getline(iss, record.gameDate, '\t');
        std::getline(iss, record.teamId, '\t');
        
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


        fileRecords.push_back(record);


    // This is the previous version
    // START OLD CODE PART
    
        // blockRecords.push_back(record); // Push the record into the block
        // records.push_back(record);      // Store metadata
        // totalRecords++;
        // recordId++;

        // if (blockRecords.size() == recordsPerDatablock) {
        //     createDatablock(blockRecords, datablockId);
        //     blockRecords.clear();
        //     datablockId++;
        //     recordId = 0; // This could be remove due to redundancy ??? I think?
        //     // When we remove the this reset line it fail to correctly calculate the Average of FG3_PCT_home
        // }
    // END OLD CODE PART
    }

    /*
    Some comment on my next part of the code:
    This code still run fine UNTIL the average part it act diffectly when we try to calculate the
    average of the FG3_PCT_home it return all 0. 
    


    */

    // START TUAN'S CODE
    // Sorting out the record based on the FG_PCT_home
    std::cout << "Sorting the record based on FG_PCT_home..." << std::endl;
    sort(fileRecords.begin(), fileRecords.end(), compare_FG_PCT_home);

    std::cout << "Records after sorting:" << std::endl;
    // for (const auto& record : fileRecords) {
    //     std::cout << "RecordID: " << record.recordId << ", FG_PCT_home: " << record.fgPctHome << std::endl;
    // }
    // Pushing all sorted records into the blocks

    float min_this_block = 1000;
    for (Record sorted_record: fileRecords){

        // TODO: this one could be the potential value used for the block id
        // if (sorted_record.fgPctHome < min_this_block){
        //     min_this_block = sorted_record.fgPctHome;
        // } 

        //std::cout << sorted_record.fgPctHome << std::endl;



        // TODO: This line will overwrite the given dataBlockId given above
        sorted_record.datablockId = datablockId;
        sorted_record.recordId = recordId;


        blockRecords.push_back(sorted_record);
        records.push_back(sorted_record);
        totalRecords++;
        recordId++;

        if (blockRecords.size() == recordsPerDatablock) { // limits number of records to predefined recordsPerDatablock
            createDatablock(blockRecords, datablockId);
            blockRecords.clear();
            datablockId++;
            recordId = 0; // Why do we have to reset the recordId every time the block is created: This is for reading from the datablock files after creation
        }
    }
    // END TUAN'S CODE

    if (!blockRecords.empty()) {
        createDatablock(blockRecords, datablockId);
    }

    datablockCount = datablockId + 1;
    std::cout << "Finished data ingestion. Total records: " << totalRecords << ", Datablocks created: " << datablockCount << std::endl;
}


void Storage::createDatablock(const std::vector<Record>& blockRecords, int datablockId) {
    
    std::string datablockFilename = getDatablockFilename(datablockId);

    /*
    std::ofstream (output file stream)
    This line basically create an 'ofstream' object named 'datablockFile'
    */
    std::ofstream datablockFile(datablockFilename, std::ios::binary);
    
    if (!datablockFile.is_open()) {
        throw std::runtime_error("Unable to create datablock file: " + datablockFilename);
    }

    for (const auto& record : blockRecords) {
        std::streampos fileOffset = datablockFile.tellp();


        //std::cout << "Writing record: " << record.recordId << ", File offset: " << fileOffset << std::endl;
        records[record.datablockId * recordsPerDatablock + record.recordId].fileOffset = fileOffset;

        datablockFile.write(reinterpret_cast<const char*>(&record), sizeof(Record));
    }

    datablockFile.close();
    std::cout << "Created datablock file: " << datablockFilename << " with " << blockRecords.size() << " records." << std::endl;
}

void Storage::loadMetadata() {
    std::cout << "Loading metadata from existing datablocks..." << std::endl;
    records.clear();
    totalRecords = 0;

    for (int i = 0; i < datablockCount; ++i) {
        std::string datablockFilename = getDatablockFilename(i);
        std::ifstream datablockFile(datablockFilename, std::ios::binary);
        
        if (!datablockFile.is_open()) {
            throw std::runtime_error("Unable to open datablock file: " + datablockFilename);
        }

        Record record;
        while (datablockFile.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
            record.datablockId = i;
            record.recordId = totalRecords % recordsPerDatablock;
            record.fileOffset = datablockFile.tellg() - std::streampos(sizeof(Record));
            records.push_back(record);
            totalRecords++;
        }

        datablockFile.close();
    }

    std::cout << "Finished loading metadata. Total records: " << totalRecords << std::endl;
}

size_t Storage::getDatablockCount() const {
    return datablockCount;
}

std::string Storage::getDatablockFilename(int datablockId) const {
    return datablockDir + "/datablock_" + std::to_string(datablockId) + ".dat";
}


Record Storage::getRecord(int datablockId, std::streampos fileOffset) {
    std::string datablockFilename = datablockDir + "/datablock_" + std::to_string(datablockId) + ".dat";
    std::ifstream datablockFile(datablockFilename, std::ios::binary);
    
    if (!datablockFile.is_open()) {
        throw std::runtime_error("Unable to open datablock file: " + datablockFilename);
    }

    datablockFile.seekg(fileOffset);
    Record record;
    datablockFile.read(reinterpret_cast<char*>(&record), sizeof(Record));

    return record;
}



void Storage::printStatistics() const {
    std::cout << "----------------- Storage Statistics -----------------" << std::endl;
    std::cout << "Total number of records: " << totalRecords << std::endl;
    std::cout << "Number of datablocks: " << (totalRecords + recordsPerDatablock - 1) / recordsPerDatablock << std::endl;
    std::cout << "Records per datablock (pre-defined): " << recordsPerDatablock << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
}

size_t Storage::getTotalRecords() const {
    return totalRecords;
}

std::vector<Record> Storage::getAllRecords() const {
    return records;
}

void Storage::getVectorByRecords(int dataBlockId, std::vector<std::streampos> fileOffsets, std::vector<Record> &resultRecords){
    
    Record record;
    
    std::string datablockFilename = datablockDir + "/datablock_" + std::to_string(dataBlockId) + ".dat";
    std::ifstream datablockFile(datablockFilename, std::ios::binary);

    if (!datablockFile.is_open()) {
        throw std::runtime_error("Unable to open datablock file: " + datablockFilename);
    }

    for (std::streampos fileOffset: fileOffsets){
        datablockFile.seekg(fileOffset);
        datablockFile.read(reinterpret_cast<char*>(&record), sizeof(Record));
        resultRecords.push_back(record);
    }
}

