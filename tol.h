#ifndef __TOL_H__
#define __TOL_H__

#include <cstdint>

#include "Dram.h"
#include "Ssd.h"

#define DRAM 0
#define SSD 1
#define HDD 2

class Storage
{
	friend class Run;
	size_t blockSize;				// optimal block size for SSD/HDD
	size_t srcSeek;					// seek in SSD/HDD where run starts
	uint8_t *d;						// pointer to DRAM buffer
	Ssd *s;							// pointer to SSD/HDD object
	size_t tail;					// tail of run queue in SSD/HDD
	size_t maxSize;					// maximum run queue size
	size_t head;					// head of run queue in SSD/HDD
public:
	Storage(Ssd *storage, uint8_t *_d, size_t bs, size_t ss, size_t t, size_t ms);
	~Storage();
	int getNext(size_t &actSze, size_t sze);
	int setNext(size_t sze);
};

class Run
{
	friend class TOL;
	Storage *source;
	size_t runBufferSize;
	size_t head;
	size_t tail;
	size_t rowSize;
public:
	Run(Ssd *_s, uint8_t *_d, size_t bs, size_t ss, size_t t, size_t ms,
		size_t rbs, size_t rowsze, size_t dramTail);
	~Run();
	int getNext(uint8_t **key);
	int setNext(uint8_t *key);
	uint8_t *getBuf();										// Get DRAM buffer
	size_t getBufferSize();
	int flush();
};

#define NT_INODE	(0)
#define NT_LEAF		(1)
#define NT_EMPTY	(2)
#define NT_LINF		(3)
#define NT_INF		(4)
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
	friend class TOL;
public:
	ETable(size_t NumRows, size_t NumCols, size_t RecordSize, size_t SortKey);
	ETable(const ETable &_t);
	~ETable();
	size_t _NumRows, _NumCols, _RecordSize, _SortKey, _rowSize;
};

class TOL {
	Run **runList;
	Run *output;
	size_t numOfRun;
	ETable t;
	Node *nodeList;
	size_t numNodes;
private:
	void setWinner(Node &curr, Node &n);
	void setLoser(Node &curr, Node &n);
	void setWWinner(Node &curr, Node &n);
	void setWLoser(Node &curr, Node &n);
	void calculateLeafWinner(Node &curr, Node &l, Node &r, size_t domain, size_t arity, bool isAscending);
	void calculateIWinner(Node &curr, Node &l, Node &r, size_t domain, size_t arity, bool isAscending);
	void cmpLeafNodes(Node &curr, Node &l, Node &r);
	void cmpINodes(Node &curr, Node &l, Node &r);
	void cmpNodes(Node &curr, Node &l, Node &r);
	void clearWinner(Node &n);
public:
	TOL(size_t nor, Run **rl, Run *o, ETable _t);
	~TOL();
	void print();
	int pass();
};

// Leaf: Should have info about where to get next part of run
// node: index(which run this key belong to), key, ofc, left child, right child,
// parent
#endif  // __TOL_H__
