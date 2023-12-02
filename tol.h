#ifndef __TOL_H__
#define __TOL_H__

#include "Ssd.h"
#include "Dram.h"

#define DRAM 0
#define SSD 1
#define HDD 2

class Storage
{
	friend class Run;
	Ssd *s;
	uint8_t *d;
	size_t srcSeek;
	size_t blockSize;
	size_t head;
	size_t tail;
	Storage(Ssd *storage, uint8_t *d, size_t bs, size_t ss);
	~Storage();
};

class Run
{
	friend class TOL;
	Storage source;
	size_t runSize;
	size_t head;
	size_t tail;
	size_t rowSize;
public:
	Run(Ssd *_s, uint8_t *_d, size_t bs, size_t ss, size_t rs);
	~Run();
	int getNext(size_t s = source.blockSize, size_t offset);// SSD/HDD to DRAM
	int setNext(size_t s = source.blockSize, size_t offset);// DRAM to SSD/HDD
	uint8_t *getBuf();										// Get DRAM buffer
	size_t getSize();
};

#define NT_INODE	(0)
#define NT_LEAF		(1)
#define NT_EMPTY	(2)
#define NT_LINF		(3)
#define NT_INF		(3)
#define INV			(-1)

class Node
{
	friend class TOL;
	int index;			// index of leaf node from which this run come from
	int nodeType;
	uint8_t *key;
	Run *r;
	int ovc;

	int winnerIndex;
	int winnerNT;
	uint8_t *winnerKey;
	Run *winnerR;
	int winnerOVC;
};

class ETable
{
public:
	ETable(size_t NumRows, size_t NumCols, size_t RecordSize, size_t SortKey = 0);
	ETable(ETable _t);
	~ETable();
	size_t _NumRows, _NumCols, _RecordSize, _SortKey, _rowSize;
};

class TOL
{
	size_t numOfRun;
	Node *nodeList;
	Run **runList;
	Run *output;
	size_t numNodes;
	ETable t;
public:
	TOL(size_t nor, Run **rl, Run *o, ETable _t);
};


// Leaf: Should have info about where to get next part of run
// node: index(which run this key belong to), key, ofc, left child, right child,
// parent
#endif
