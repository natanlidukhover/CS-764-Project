#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "defs.h"
#include "Scan.h"

using namespace std;

std::vector<int> getParameters(int size) {
    std::vector<int> intVector;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);

    int i = 0;
    
    while (i < size/4) {
        int random_number = dis(gen);
        intVector.push_back(random_number);  // Appending a random number
        i++;
    }
    return intVector;
}

// Save vector of integers to file in binary format
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
    cout << fileSize << "bytes written to file" << filename << std::endl;
    inFile.close();
    }

// Read vector of integers from file in binary format
// compile using make
//TODO
// use Row and Table class to store integers
void readIntegersFromBinaryFile(const std::string& filename, int recordSize, int numberOfRecordsToRead) {
    ifstream inFile (filename, ios::in | ios::binary | std::ios::ate);
    std::streamsize fileSize = inFile.tellg();
    cout << "Size of input file is "<< fileSize << " bytes" << std::endl;

    //seekg used as ate mode moves it to end of file
    inFile.seekg(0, ios::beg);
    vector<int> res;
    while(numberOfRecordsToRead > 0)
    {
        unsigned short int y;
        inFile.read((char*)(&y), recordSize);
        cout << "Char " << y << " \n";
        numberOfRecordsToRead -= 1;
        res.push_back(y);
    }
    inFile.close();
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
	std::vector<int> test = getParameters(40);
    string file = "./data/testData.bin";
    saveIntegersToBinaryFile(test, file);
    readIntegersFromBinaryFile(file, 1, 6);
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
