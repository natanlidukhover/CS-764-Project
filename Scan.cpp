#include "Scan.h"
//#include "defs.cpp"
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
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }
    
    for (int number : numbers) {
        outFile << number << std::endl;  // Write binary string to file
    }
    outFile.close();
}

// Read vector of integers from file in binary format
void readIntegersFromBinaryFile(const std::string& filename, int recordSize, int numberOfRecordsToRead) {
    ifstream inFile (filename, ios::in | ios::binary);
    inFile.seekg(0, ios::end);
    int file_size = inFile.tellg();
    cout<<"Size of the file is"<<" "<< file_size<<" "<<"bytes" << "\n";
    inFile.seekg(0);
    
    while(numberOfRecordsToRead > 0)
    {
        char x[recordSize];
        inFile.read(x, recordSize);
        for(auto c: x) cout << c ;
        cout << "\n";
        char space;
        inFile.read(&space, 1);
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
	//std::vector<int> test = getParameters(40);
    //saveIntegersToBinaryFile(test, "testData.bin");
    readIntegersFromBinaryFile("./data/testData.bin", 7, 4);
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
