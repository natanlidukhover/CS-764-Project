#include "defs.h"
#include "Ssd.h"
#include "tol.h"
#include <cmath>
#include <math.h>
#include <stdexcept>
#include <cstring>

// TODO change these APIs to read and write any number of data

// use to manage run "queue" in storate
// getNext -> read block from head of run in SSD/HDD to DRAM
// setNext -> write block from DRAM to tail of SSD/HDD
// when reading an already stored run from SSD/HDD init head to 0, tail to run
// size
// when writing a new run to SSD/HDD init head and tail to 0
// storage -> pointer to SSD/HDD object
// _d -> pointer to dram buffer
// bs -> SSD/HDD block size
// ss -> seek in SSD/HDD where run is stored
// t -> tail offset, used if SSD/HDD already has data
// ms -> maximum number of bytes which can be fitted in run on SSD/HDD
Storage::Storage(Ssd *storage, uint8_t *_d, size_t bs, size_t ss, size_t t,
		size_t ms): blockSize(bs), srcSeek(ss), d(_d), s(storage), tail(t),
	maxSize(ms)
{
	head = 0;
}

Storage::~Storage()
{
}

// _s -> pointer to SSD/HDD object
// _d -> pointer to dram buffer
// bs -> SSD/HDD block size
// ss -> seek in SSD/HDD where run is stored
// t -> tail offset, used if SSD/HDD already has data
// ms -> maximum number of bytes which can be fitted in run on SSD/HDD
// brs -> dram buffer size for this run
Run::Run(Ssd *_s, uint8_t *_d, size_t bs, size_t ss, size_t t, size_t ms,
		size_t brs, size_t rowsze): runSize(brs), rowSize(rowsze)
{
	head = 0;
	tail = 0;
	source = new Storage(_s, _d, bs, ss, t, ms);
}

Run::~Run()
{
    delete source;
}

// sze -> number of bytes to be read from persistent storage(this should be
// multiple of blockSize
// actSze -> actual size read from persistent storage, this is useful in corner
// cases when run is getting exhausted
int Storage::getNext(size_t &actSze, size_t sze = 0)
{
	if (sze == 0)
		sze = blockSize;
	if (sze % blockSize)
		return EINPARM;
	if (head >= tail)
		return EEMPTY;
	size_t i;
	int data_read = 0, ret = SUCCESS;
	for (i = head; i < tail && i < (head + sze); i += blockSize) {
		if ((ret = s->readData(d + data_read,
				srcSeek + i))) {
			return ret;
		}
		data_read += blockSize;
	}
	head += data_read;
	actSze = data_read;
	return SUCCESS;
}

int Storage::setNext(size_t sze = 0)
{
	if (sze == 0)
		sze = blockSize;
	if (sze % blockSize)
		return EINPARM;
	if (tail + sze >= maxSize)
		return EFULL;

    size_t data_written = 0;
	int ret = SUCCESS;
    for (size_t i = tail; i < runSize && i < ((size_t) tail + s); i += source->blockSize) {
        if (ret == source->s->writeData(source->d + data_written, srcSeek + i)) {
            return ret;
        }
        data_written += source->blockSize;
    }

    tail += data_written;
    return SUCCESS;
}

int Run::getNext(uint8_t **key)
{
	int ret;
	if (head >= tail) {
		size_t dataRead = 0;
		if ((ret = source->getNext(dataRead, runSize)) == SUCCESS) {
			head = 0;
			tail = dataRead;
		} else {
			return ret;
		}
	}
	*key = &(source->d[head]);
	head += rowSize;
	return SUCCESS;
}

int Run::setNext(uint8_t *key)
{
	int ret;
	if (tail + rowSize >= runSize) {
		if((ret = source->setNext(runSize)) == SUCCESS) {
			head = 0;
			tail = 0;
		} else {
			return ret;
		}
	}
	memcpy(&(source->d[tail]), key, rowSize);
	tail += rowSize;
	return SUCCESS;
}

uint8_t *Run::getBuf()
{
	return source->d;
}

size_t Run::getSize()
{
    return runSize;
}

=======
ETable::ETable(size_t NumRows, size_t NumCols, size_t RecordSize, size_t SortKey
		= 0):_NumRows(NumRows), _NumCols(NumCols), _RecordSize(RecordSize),
	_SortKey(SortKey)
{
}

ETable::ETable(const ETable &_t)
{
	_NumRows = _t._NumRows;
	_NumCols = _t._NumCols;
	_RecordSize = _t._RecordSize;
	_SortKey = _t._SortKey;
}

ETable::~ETable()
{
}

void TOL::setWinner(Node &curr, Node &n) {
	curr.winnerIndex = n.index;
	curr.winnerNT = n.nodeType;
	curr.winnerKey = n.key;
	curr.winnerR = n.r;
	curr.winnerOVC = n.ovc;
}

void TOL::setLoser(Node &curr, Node &n) {
	curr.index = n.index;
	curr.nodeType = n.nodeType;
	curr.key = n.key;
	curr.r = n.r;
	curr.ovc = n.ovc;
}

/**
 * Its assume that none of curr, l and r is INF or EMPTY node
*/
void TOL::calculateLeafWinner(Node &curr, Node &l, Node &r, size_t domain = 10, size_t arity = 0, bool isAscending = true) {
	if (arity == 0)
		arity = t._rowSize;
	// TODO compare using OVC
	if (l.ovc != INV && r.ovc != INV) {
		if (l.ovc != r.ovc) {
			// TODO check if these conditions are correct
			if (l.ovc < r.ovc) {
				setWinner(curr, l);
				setLoser(curr, r);
			} else {
				setWinner(curr, r);
				setLoser(curr, l);
			}
			curr.nodeType = NT_INODE;
			return;
		}
	}
	// If OVC comparison fail, compare whole row
	unsigned short offset = t._rowSize;
	unsigned short value = domain;
	uint8_t *prevRow = l.key;
	uint8_t *currentRow = r.key;
	for (unsigned short i = 0; i < t._rowSize; i++) {
		if (currentRow[i] != prevRow[i]) {
			offset = i;
			if (currentRow[i] < prevRow[i]) {
				value = prevRow[i];
				setWinner(curr, r);
				setLoser(curr, l);
			} else {
				value = currentRow[i];
				setWinner(curr, l);
				setLoser(curr, r);
			}
			curr.nodeType = NT_INODE;
			break;
		}
	}

	if (isAscending) {
		offset = arity - offset;
	} else {
		value = domain - value;
	}

	// E.g. domain is 1 to 99 -> 100; offset is 3; value is 94 (domain 100 - actual value of 6);
	// zerosInDomain = floor(log10(abs(100))) = floor(2) = 2;
	// offsetValueCode = offset * 10^2 + value = 3 * 100 + 94 = 300 + 94 = 394
	unsigned short zerosInDomain = floor(log10(domain));
	unsigned short offsetValueCode = offset * pow(10, zerosInDomain);

	curr.ovc = offsetValueCode + value;
}

void TOL::cmpLeafNodes(Node &curr, Node &l, Node &r) {
	if (l.nodeType == NT_LINF && r.nodeType == NT_LINF) {
		curr.nodeType = curr.winnerNT = NT_INF;
		curr.key = curr.winnerKey = NULL;
		curr.r = curr.winnerR = NULL;
		curr.ovc = curr.winnerOVC = INV;
	} else if (l.nodeType == NT_LINF) {
		curr.nodeType = NT_INF;
		curr.key = NULL;
		curr.r = NULL;
		curr.ovc = INV;

		curr.winnerIndex = r.index;
		curr.winnerNT = NT_INODE;
		curr.winnerKey = r.key;
		curr.winnerR = r.r;
		curr.winnerOVC = r.ovc;
	} else if (r.nodeType == NT_LINF) {
		curr.nodeType = NT_INF;
		curr.key = NULL;
		curr.r = NULL;
		curr.ovc = INV;

		curr.winnerIndex = l.index;
		curr.winnerNT = NT_INODE;
		curr.winnerKey = l.key;
		curr.winnerR = l.r;
		curr.winnerOVC = l.ovc;
	} else {
		calculateLeafWinner(curr, l, r);
	}
}

void TOL::cmpINodes(Node &curr, Node &l, Node &r) {
	if (l.nodeType == NT_INF && r.nodeType == NT_INF) {
		curr.nodeType = curr.winnerNT = NT_INF;
		curr.key = curr.winnerKey = NULL;
		curr.r = curr.winnerR = NULL;
		curr.ovc = curr.winnerOVC = INV;
	} else if (l.nodeType == NT_INF) {
		curr.nodeType = NT_INF;
		curr.key = NULL;
		curr.r = NULL;
		curr.ovc = INV;

		curr.winnerIndex = r.index;
		curr.winnerNT = NT_INODE;
		curr.winnerKey = r.key;
		curr.winnerR = r.r;
		curr.winnerOVC = r.ovc;
	} else if (r.nodeType == NT_INF) {
		curr.nodeType = NT_INF;
		curr.key = NULL;
		curr.r = NULL;
		curr.ovc = INV;

		curr.winnerIndex = l.index;
		curr.winnerNT = NT_INODE;
		curr.winnerKey = l.key;
		curr.winnerR = l.r;
		curr.winnerOVC = l.ovc;
	} else {
		calculateLeafWinner(curr, l, r);
	}
}

void TOL::cmpNodes(Node &curr, Node &l, Node &r) {
	// cmp lc and rc, save winner in current node winnerKey and winnerR
	// save loser in current node key and r
	// update ovc in looser node wrt to winner
	// How to cmp?
	// if leaf node - if both are inf then win is inf, los is inf,
	// current node becomes inf
	// if one is inf, los is inf, win is the other node
	// if none is inf, cmp two keys and assign according to result
	// if inode
	// if both lower win is inf, this node becomes inf
	// if one lower win is inf, los is inf, win is the winner of oth node
	// if none lower win is inf, cmp lower nodes winner, looser stays,
	// winner is store in winnerKey and winnerR
	if (l.nodeType == NT_LEAF || l.nodeType == NT_LINF)
		cmpLeafNodes(curr, l, r);
	else
		cmpINodes(curr, l, r);
}

void TOL::pass() {
	// Set next key in the root winner's run to be the value at that run's leaf
	int leafIndex = nodeList[0].winnerIndex;
	nodeList[0].winnerR->getNext(&(nodeList[leafIndex].key));
	// Compare the children of the current node where current node is the parent of the updated leaf all the way up the tree
	int parentNodeIndex = (leafIndex - 1) / 2;
	while (parentNodeIndex >= 0) {
		cmpNodes(nodeList[parentNodeIndex], nodeList[2 * parentNodeIndex + 1], nodeList[2 * parentNodeIndex + 2]);
		parentNodeIndex = (parentNodeIndex - 1) / 2;
	}
}

TOL::TOL(size_t nor, Run **rl, Run *o, ETable _t): runList(rl), output(o), numOfRun(nor), t(_t) {
	if (nor > 256) {
		throw std::runtime_error("TOL doesn't fit in cache");
	}

	int tol_height = ceil(log2(nor)) + 1;
	numNodes = pow(2, tol_height) - 1;
	nodeList = new Node[numNodes];

	// init leaf nodes
	size_t firstLeaf = pow(2, tol_height - 1) - 1;
	size_t lastLeaf = pow(2, tol_height) - 2;
	for (size_t i = 0; i < (lastLeaf - firstLeaf); i++) {
		int li = firstLeaf + i;
		nodeList[li].index = li;
		if (i > nor) {
			nodeList[li].nodeType = NT_LINF;
			nodeList[li].key = NULL;
			nodeList[li].r = NULL;
			nodeList[li].ovc = INV;

			nodeList[li].winnerIndex = INV;
			nodeList[li].winnerNT = NT_EMPTY;
			nodeList[li].winnerKey = NULL;
			nodeList[li].winnerR = NULL;
			nodeList[li].winnerOVC = INV;
		} else {
			nodeList[li].nodeType = NT_LEAF;
			runList[i]->getNext(&(nodeList[li].key));
			nodeList[li].r = runList[i];
			nodeList[li].ovc = INV;

			nodeList[li].winnerIndex = INV;
			nodeList[li].winnerNT = NT_EMPTY;
			nodeList[li].winnerKey = NULL;
			nodeList[li].winnerR = NULL;
			nodeList[li].winnerOVC = INV;
		}
	}

	// init internal nodes
	for (int i = tol_height - 1; i >= 0; i--) {
		// init all nodes at each height
		int firstNode = pow(2, i - 1) - 1;
		int lastNode = pow(2, i) - 2;
		for (int j = 0; j < (lastNode - firstNode); j++) {
			int ni = firstNode + j;
			int lc = 2 * ni + 1;
			int rc = 2 * ni + 2;
			nodeList[ni].nodeType = NT_INODE;
			nodeList[ni].key = NULL;
			nodeList[ni].r = NULL;
			nodeList[ni].ovc = INV;

			nodeList[ni].winnerNT = NT_EMPTY;
			nodeList[ni].winnerKey = NULL;
			nodeList[ni].winnerR = NULL;
			nodeList[ni].winnerOVC = INV;

			cmpNodes(nodeList[ni], nodeList[lc], nodeList[rc]);
		}
	}
}

TOL::~TOL() {
	delete[] nodeList;
}