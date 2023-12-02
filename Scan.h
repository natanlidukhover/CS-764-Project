#include "Iterator.h"
#include <vector>
#include <iostream>
#include <string.h>
using namespace std;
class ScanPlan : public Plan
{
	friend class ScanIterator;
public:
	ScanPlan (RowCount const count);
	~ScanPlan ();
	Iterator * init () const;
	//cannot add below line due to cyclic dependency
	//ScanIterator * init () const;
private:
	RowCount const _count;
}; // class ScanPlan

class ScanIterator : public Iterator
{
public:
	ScanIterator (ScanPlan const * const plan);
	~ScanIterator ();
	string file;
	bool next ();
	vector<int> run();
	std::vector<int> getParameters(int size);
	void saveIntegersToBinaryFile(const std::vector<int>& numbers, const string& filename) ;
	vector<int> readIntegersFromBinaryFile(const std::string& filename, int recordSize, int numberOfRecordsToRead) ;
private:
	ScanPlan const * const _plan;
	RowCount _count;
}; // class ScanIterator
