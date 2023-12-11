#include "Scan.h"

#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "defs.h"

using namespace std;

#define cout outTrace

ScanPlan::ScanPlan(RowCount const count) : _count(count) {
}  // ScanPlan::ScanPlan

ScanPlan::ScanPlan(RowCount const count, size_t blockSize) : _count(count), blockSize(blockSize) {
}  // ScanPlan::ScanPlan

ScanPlan::~ScanPlan() {
}  // ScanPlan::~ScanPlan

Iterator* ScanPlan::init() const {
    return new ScanIterator(this);
}  // ScanPlan::init

ScanIterator::ScanIterator(ScanPlan const* const plan) : _plan(plan), _count(plan->_count), blockSize(plan->blockSize) {
    this->file = "./input/testData.bin";
    cout << "Random data will be written to the file " << this->file << endl;
}  // ScanIterator::ScanIterator

ScanIterator::~ScanIterator() {
    traceprintf("produced %lu of %lu rows\n",
                (unsigned long) (_count),
                (unsigned long) (_plan->_count));
}  // ScanIterator::~ScanIterator

bool ScanIterator::next() {
    if (_count >= _plan->_count)
        return false;

    ++_count;
    return true;
}  // ScanIterator::next

/**
 * Generates size integers of 1 byte each randomly
 *
 * @param size Size of the file to be generated in bytes. Eg: To generate a file of 50KB will be 50*1024 as the size parameter
 */
std::vector<int> ScanIterator::getParameters(int numberOfIntegers) {
    std::vector<int> intVector;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);

    // Generate random numbers until the vector reaches the specified total number of integers
    for (int i = 0; i < numberOfIntegers; ++i) {
        int random_number = dis(gen);
        intVector.push_back(random_number);  // Appending a random number
    }
    return intVector;
}

/**
 * Save vector of integers to file in binary format
 *
 * @param numbers Vector of numbers to be written to file. The numbers are single digit numbers in the range of 1 to 10
 * @param filename The binary filename where the above vector numbers will be written
 */
void ScanIterator::saveIntegersToBinaryFile(const std::vector<int>& numbers, const std::string& filename, bool isAppendOnly) {
    // Open the file in appendOnly mode or truncate mode based on the flag
    std::ofstream outFile;
    if (isAppendOnly)
        outFile.open(filename, std::ios::binary | std::ios::app);
    else
        outFile.open(filename, std::ios::binary | std::ios::ate);

    if (!outFile) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }
    for (int x : numbers) {
        outFile.write((char*) &x, 1);
    }
    outFile.close();
    std::ifstream inFile(filename, std::ios::binary | std::ios::ate);
    inFile.close();
}

/**
 * Read vector of integers from file in binary format
 *
 * @param filename The binary filename from where the we will read integers
 * @param recordSize The number of bytes we want to consume during one read
 * @param numberOfRecordsToRead Number of records of size recordSize to be read from the binary file
 *
 */
vector<int> ScanIterator::readIntegersFromBinaryFile(const std::string& filename, int recordSize, int numberOfRecordsToRead) {
    vector<int> res;
    ifstream inFile(filename, ios::in | ios::binary | std::ios::ate);
    // seekg used as std::ios::ate mode moves it to end of file
    inFile.seekg(0, ios::beg);
    while (numberOfRecordsToRead > 0) {
        unsigned short int y = 0;
        inFile.read((char*) (&y), recordSize);
        numberOfRecordsToRead -= 1;
        res.push_back(y);
    }
    inFile.close();
    return res;
}

vector<int> ScanIterator::run() {
    TRACE(true);
    size_t countOfBytes = this->_count;
    // Generate chunkSize bytes of random data at a time and write it to file
    size_t blockSize = this->blockSize;
    size_t blocks = countOfBytes / blockSize;
    for (size_t i = 0; i < blocks; i++) {
        std::vector<int> currChunk = this->getParameters(blockSize);
        // Open the file in truncate mode for the first chunk and then open it in append mode
        // Hence we pass a flag isAppendOnly whose vaue is i which will be false when it is the first run (i=0)
        this->saveIntegersToBinaryFile(currChunk, this->file, i);
    }
    size_t remaining = countOfBytes % blockSize;
    if (remaining != 0) {
        std::vector<int> remainingChunk = this->getParameters(remaining);
        this->saveIntegersToBinaryFile(remainingChunk, this->file, blocks > 0);
    }

    cout << countOfBytes << " bytes of random data generated and stored in HDD" << endl;
    vector<int> numbers = readIntegersFromBinaryFile(this->file, 1, countOfBytes);
    return numbers;
}  // ScanIterator::next
