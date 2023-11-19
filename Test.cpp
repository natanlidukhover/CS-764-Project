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
	int count = 9;
	Plan * const plan = new ScanPlan (count);
	new SortPlan ( new FilterPlan ( new ScanPlan (count) ) );

	Iterator * const it = plan->init ();
	it->run ();

	//ScanPlan * const sc_plan = new ScanPlan (count);
	ScanIterator * const sc_it = new ScanIterator(new ScanPlan (count));
	vector<int> numbers = sc_it->run();
	int n = 3;// * 1024;
	Table tmp(n, n, 1);
	int k = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			tmp[i][j] = numbers[k++]; //Random(10);//n - i+j;
			cout << (int)tmp[i][j];
			cout << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	int sort_col = 0;
	generateRuns(tmp, 0, n - 1, sort_col);
	verifySortedRuns(tmp, 0, n - 1, sort_col);

	cout << "Sorted table" << "\n";
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			cout << (int)tmp[i][j] << " ";
		}
		cout << "\n";
	}

	
	// delete it;

	// delete plan;

    return 0;
}  // main
