#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "Table.h"
#include "Dram.h"
#include "defs.h"
#include <iostream>

using namespace std;
int main(int argc, char* argv[]) {
    TRACE(true);
	int data_size = 50 * 1024 * 1024; //in bytes
	int rows = data_size/50 ;
	Plan * const plan = new ScanPlan (data_size);
	new SortPlan ( new FilterPlan ( new ScanPlan (data_size) ) );

	Iterator * const it = plan->init ();
	it->run ();

	//ScanPlan * const sc_plan = new ScanPlan (count);
	ScanIterator * const sc_it = new ScanIterator(new ScanPlan (data_size));
	vector<int> numbers = sc_it->run();
	
	Table tmp(rows, 3, 17);
	int k = 0;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < 50; j++) {
			tmp[i][j] = numbers[k++]; //Random(10);//n - i+j;
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

	
	// delete it;

	// delete plan;

    return 0;
}  // main
