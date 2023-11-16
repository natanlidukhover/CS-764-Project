#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "defs.h"
#include "Scan.h"

using namespace std;
/**
 * Generates size integers of 1 byte each randomly
 * 
 * @param size Size of the file to be generated in bytes. Eg: To generate a file of 50KB will be 50*1024 as the size parameter
*/
std::vector<int> getParameters(int size) {
    std::vector<int> intVector;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);

    int i = 0;
    
    while (i < size) {
        int random_number = dis(gen);
        intVector.push_back(random_number);  // Appending a random number
        i++;
    }
    return intVector;
}

/**
 * Save vector of integers to file in binary format
 * 
 * @param numbers Vector of numbers to be written to file. The numbers are single digit numbers in the range of 1 to 10
 * @param filename The binary filename where the above vector numbers will be written
*/
void saveIntegersToBinaryFile(const std::vector<int>& numbers, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::ate);
    if (!outFile) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }
    for(int x: numbers)
    {
        //outFile.write(reinterpret_cast<char*>(&x),sizeof(int));
        outFile.write((char*)&x,1);
    }
    outFile.close();
    std::ifstream inFile(filename, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = inFile.tellg();
    cout << fileSize << " bytes written to file" << filename << std::endl;
    inFile.close();
    }

// Read vector of integers from file in binary format
//TODO
// use Row and Table class to store integers
/**
 * Read vector of integers from file in binary format
 * 
 * @param filename The binary filename from where the we will read integers
 * @param recordSize The number of bytes we want to consume during one read
 * @param numberOfRecordsToRead Number of records of size recordSize to be read from the binary file
 *
*/
vector<int> readIntegersFromBinaryFile(const std::string& filename, int recordSize, int numberOfRecordsToRead) {
    vector<int> res;
    ifstream inFile (filename, ios::in | ios::binary | std::ios::ate);
    std::streamsize fileSize = inFile.tellg();
    cout << "Size of input file is "<< fileSize << " bytes" << std::endl;

    //seekg used as ate mode moves it to end of file
    inFile.seekg(0, ios::beg);
    while(numberOfRecordsToRead > 0)
    {
        unsigned short int y = 0;
        inFile.read((char*)(&y), recordSize);
        numberOfRecordsToRead -= 1;
        res.push_back(y);
    }
    inFile.close();
    return res;
}

ScanPlan::ScanPlan (RowCount const count) : _count (count)
{
	TRACE (true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (true);
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
	TRACE (true);
	return new ScanIterator (this);
} // ScanPlan::init

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (true);
    // numbers to generate
    int countOfNumbers = 10;
	std::vector<int> test = getParameters(countOfNumbers);
    string file = "./data/testData.bin";
    saveIntegersToBinaryFile(test, file);
    vector<int> numbers = readIntegersFromBinaryFile(file, 1, countOfNumbers/5);
    for(auto i : numbers) cout << i << " ";
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next ()
{
	TRACE (true);

	if (_count >= _plan->_count)
		return false;

	++ _count;
	return true;
} // ScanIterator::next
