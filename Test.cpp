#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "Table.h"
#include "Dram.h"
#include "defs.h"
#include <iostream>
#include "Ssd.h"
using namespace std;
int main(int argc, char* argv[]) {
    TRACE(true);
	size_t data_size = 150;//50MB=50 * 1024 * 1024; //in bytes
	size_t row_size = 50;
	size_t rows = (data_size/row_size) + ((data_size % row_size) == 0 ? 0 : 1) ;
	ScanIterator * const sc_it = new ScanIterator(new ScanPlan (data_size));
	vector<int> numbers = sc_it->run();
	
	Table tmp(rows, 3, 17);
	int k = 0;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < 50; j++) {
			tmp[i][j] = numbers[k++];
			cout << (int)tmp[i][j];
			cout << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	int sort_col = 0;
	generateRuns(tmp, 0, rows - 1, sort_col);
	verifySortedRuns(tmp, 0, rows - 1, sort_col);

	cout << "Sorted table" << "\n";
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < 50; j++) {
			cout << (int)tmp[i][j] << " ";
		}
		cout << "\n";
	}
	//for hdd pagesize is given by bandwidth * latency = 100*0.01
	Ssd * const hdd = new Ssd("./data/testData.bin",(size_t)10*1024*1024*1024, 1, data_size);
	size_t offset = data_size + 1;
	for(size_t i = 0; i < rows; i++)
	{
		hdd->writeData(static_cast<const void*>(tmp[i]),offset + i*(row_size),row_size);
	}

    return 0;
}  // main
