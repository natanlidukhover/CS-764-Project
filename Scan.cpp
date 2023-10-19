#include "Scan.h"
#include "defs.h"
#include <vector>
#include<iostream>
#include<fstream>
using namespace std;
std::vector<int> getParameters(int size) {
    std::vector<int> test;
    int i = 0;
    
    while (i < size/4) {
        test.push_back(static_cast<int>(Random(0, 9)));  // Appending a random number
        i++;
    }
    
    return test;
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
        cout << x << " ";
        //outFile.write(reinterpret_cast<char*>(&x),sizeof(int));
        outFile.write((char*)&x,1);
    }
    outFile.close();
    std::ifstream inFile(filename, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = inFile.tellg();
    cout << "Test file size" << fileSize << "\n";
    inFile.close();
    }

// Read vector of integers from file in binary format
// compile using make
//TODO
// Size of file using sstats in C
// Figuring out the binary thing using that commit 
// use Row and Table class to store integers
void readIntegersFromBinaryFile(const std::string& filename, int recordSize, int numberOfRecordsToRead) {
    ifstream inFile (filename, ios::in | ios::binary | std::ios::ate);
    std::streamsize fileSize = inFile.tellg();
    cout << "Size of the file is "<< fileSize<<" bytes" << "\n";

    //seekg used as ate mode moves it to end of file
    inFile.seekg(0, ios::beg);
    while(numberOfRecordsToRead > 0)
    {
        unsigned short int y;
        inFile.read((char*)(&y), recordSize);
        cout << "Char " << y << " \n";
        numberOfRecordsToRead -= 1;
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
