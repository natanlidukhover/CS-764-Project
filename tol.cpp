#include "defs.h"
#include "Ssd.h"
#include "tol.h"
#include <cmath>
#include <math.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

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
		size_t rbs, size_t rowsze, size_t dramTail): runBufferSize(rbs), rowSize(rowsze), tail(dramTail)
{
	head = 0;
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
	int data_read = 0;
	for (i = head; i < tail && i < (head + sze); i += blockSize) {
		data_read += s->readData(d + data_read, srcSeek + i);
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
	if (tail + sze > maxSize)
		return EFULL;

    size_t data_written = 0;
    for (size_t i = tail; i < (tail + sze); i += blockSize) {
        data_written += s->writeData(d + data_written, srcSeek + i);
    }

    tail += data_written;
    return SUCCESS;
}

int Run::getNext(uint8_t **key)
{
	int ret;
	if (head >= tail) {
		size_t dataRead = 0;
		if ((ret = source->getNext(dataRead, source->blockSize)) == SUCCESS) {
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
	if (tail + rowSize > runBufferSize) {
		if((ret = source->setNext(runBufferSize)) == SUCCESS) {
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

size_t Run::getBufferSize()
{
    return runBufferSize;
}

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

void TOL::setWWinner(Node &curr, Node &n) {
	curr.winnerIndex = n.winnerIndex;
	curr.winnerNT = n.winnerNT;
	curr.winnerKey = n.winnerKey;
	curr.winnerR = n.winnerR;
	curr.winnerOVC = n.winnerOVC;
}

void TOL::setWinner(Node &curr, Node &n) {
	curr.winnerIndex = n.index;
	curr.winnerNT = n.nodeType;
	curr.winnerKey = n.key;
	curr.winnerR = n.r;
	curr.winnerOVC = n.ovc;
}

void TOL::setWLoser(Node &curr, Node &n) {
	curr.index = n.winnerIndex;
	curr.nodeType = n.winnerNT;
	curr.key = n.winnerKey;
	curr.r = n.winnerR;
	curr.ovc = n.winnerOVC;
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
void TOL::calculateIWinner(Node &curr, Node &l, Node &r, size_t domain = 10, size_t arity = 0, bool isAscending = true) {
	if (arity == 0)
		arity = t._rowSize;
	// TODO compare using OVC
	if (l.winnerOVC != INV && r.winnerOVC != INV) {
		if (l.winnerOVC != r.winnerOVC) {
			// TODO check if these conditions are correct
			if (l.winnerOVC < r.winnerOVC) {
				setWWinner(curr, l);
				setWLoser(curr, r);
			} else {
				setWWinner(curr, r);
				setWLoser(curr, l);
			}
			curr.nodeType = NT_INODE;
			return;
		}
	}
	// If OVC comparison fail, compare whole row
	unsigned short offset = t._rowSize;
	unsigned short value = domain;
	uint8_t *prevRow = l.winnerKey;
	uint8_t *currentRow = r.winnerKey;
	for (unsigned short i = 0; i < t._rowSize; i++) {
		if (currentRow[i] != prevRow[i]) {
			offset = i;
			if (currentRow[i] < prevRow[i]) {
				value = prevRow[i];
				setWWinner(curr, r);
				setWLoser(curr, l);
			} else {
				value = currentRow[i];
				setWWinner(curr, l);
				setWLoser(curr, r);
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
		calculateIWinner(curr, l, r);
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

int TOL::pass() {
	// Check for infinite case
	if (nodeList[0].nodeType == NT_INF) {
		return EINF;
	}
	// Store current winner
	int ret = output->setNext(nodeList[0].key);
	if (ret != SUCCESS) {
		return ret;
	}
	// Set next key in the root winner's run to be the value at that run's leaf
	size_t leafIndex = nodeList[0].winnerIndex;
	ret = (nodeList[0].winnerR)->getNext(&nodeList[leafIndex].key);
	if (ret != SUCCESS) {
		// If the run is exhausted, the leaf node should be infinite
		nodeList[leafIndex].nodeType = NT_LINF;
		nodeList[leafIndex].key = NULL;
		nodeList[leafIndex].r = NULL;
		nodeList[leafIndex].ovc = INV;
	}
	// Compare the children of the current node where current node is the parent of the updated leaf all the way up the tree
	size_t parentNodeIndex = (leafIndex - 1) / 2;
	while (parentNodeIndex >= 0) {
		cmpNodes(nodeList[parentNodeIndex], nodeList[2 * parentNodeIndex + 1], nodeList[2 * parentNodeIndex + 2]);
		if (parentNodeIndex == 0) {
			break;
		}
		parentNodeIndex = (parentNodeIndex - 1) / 2;
	}
	return SUCCESS;
}

void TOL::print() {
	size_t tol_height = ceil(log2(numOfRun)) + 1;
	for (size_t i = 1; i <= tol_height; i++) {
		size_t firstNode = pow(2, i - 1) - 1;
		size_t lastNode = pow(2, i) - 2;
		for (size_t j = firstNode; j <= lastNode; j++) {
			std::cout << "node index: " << j << endl;
			std::cout << "nodeType: " << nodeList[j].nodeType << endl;
			std::cout << "key: " << endl;
			if (nodeList[j].nodeType == NT_INODE || nodeList[j].nodeType == NT_LEAF) {
				for (size_t k = 0; k < this->t._RecordSize; k++) {
					std::cout << (int)nodeList[j].key[k] << " ";
				}
				std::cout << endl;
			}
			std::cout << "run: " << nodeList[j].r << endl;
			std::cout << "ovc: " << nodeList[j].ovc << endl;

			std::cout << "winnerIndex: " << nodeList[j].winnerIndex << endl;
			std::cout << "winnerNT: " << nodeList[j].winnerNT << endl;
			std::cout << "winnerKey: " << endl;
			if (nodeList[j].winnerNT == NT_INODE || nodeList[j].winnerNT == NT_LEAF) {
				for (size_t k = 0; k < this->t._RecordSize; k++) {
					std::cout << (int)nodeList[j].winnerKey[k] << " ";
				}
				std::cout << endl;
			}
			std::cout << "winnerR: " << nodeList[j].winnerR << endl;
			std::cout << "winnerOVC: " << nodeList[j].winnerOVC << endl;
			std::cout << endl;
		}
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
	for (size_t i = 0; i <= (lastLeaf - firstLeaf); i++) {
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
		for (int j = 0; j <= (lastNode - firstNode); j++) {
			int ni = firstNode + j;
			int lc = 2 * ni + 1;
			int rc = 2 * ni + 2;
			nodeList[ni].nodeType = NT_INODE;
			nodeList[ni].key = NULL;
			nodeList[ni].r = NULL;
			nodeList[ni].ovc = INV;

			nodeList[ni].winnerIndex = INV;
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
