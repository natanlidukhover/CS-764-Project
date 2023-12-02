#include "defs.h"
#include "tol.h"
#include <math.h>
#include <cmath>

Storage::Storage(Ssd *storage, uint8_t *_d, size_t bs, size_t ss):blockSize(bs),
	sourceSeek(ss), d(_d), s(storage)
{
}

Storage::~Storage()
{
}

Run::Run(Ssd *_s, uint8_t *_d, size_t bs, size_t ss, size_t rs): runSize(rs)
{
	head = 0;
	tail = 0;
	source = new Storage(_s, _d, bs, ss);
}

Run::~Run()
{
	delete source;
}

// edit these functions
// Run::getNext should return next 1 row
// Storage::getNext called by Run::getNext should fill up its buffer
int Run::getNext(size_t s = source.blockSize, size_t offset = 0)
{
	// think about how will data move from all levels and in the data structures
	if (s % source.blockSize)
		return EINPARAM;
	if (head >= tail)
		return EEMPTY;
	int i, data_read = 0, ret = SUCCESS;
	for (i = head; i < tail && i < (head + s); i += source.blockSize) {
		if ((ret = source.s->readData(source.d + data_read,
				srcSeek + i))) {
			return ret;
		}
		data_read += source.blockSize;
	}
	head += data_read;
	return SUCCESS;
}

int Run::setNext(size_t s = source.blockSize, size_t offset = 0)
{
	if (s % source.blockSize)
		return EINPARAM;
	if (tail >= runSize)
		return EFULL;
	int i, data_written = 0;
	for (int i = tail; i < runSize && i < (tail + s); i += source.blockSize) {
		if ((ret = source.s->writeData(source.d + data_written,
				srcSeek + i))) {
			return ret;
		}
		data_written += source.blockSize
	}
	tail += data_written;
	return SUCCESS;
}

uint8_t *Run::getBuf()
{
	return source.d;
}

size_t Run::getSize()
{
	return runSize;
}

ETable::ETable(size_t NumRows, size_t NumCols, size_t RecordSize, size_t SortKey
		= 0):_NumRows(NumRows), _NumCols(NumCols), _RecordSize(RecordSize),
	_SortKey(SortKey)
{
}

ETable::ETable(ETable _t)
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

void TOL::setLooser(Node &curr, Node &n) {
	curr.index = n.index;
	curr.nodeType = n.nodeType;
	curr.key = n.key;
	curr.r = n.r;
	curr.ovc = n.ovc;
}

/**
 * Its assume that none of curr, l and r is INF or EMPTY node
*/
void TOL::calculateLeafWinner(Node &curr, Node &l, Node &r, size_t domain = 10,
		size_t arity = t._rowSize, bool isAscending = true) {
	// TODO compare using OVC
	if (l.ovc != INV && r.ovc != INV) {
		if (l.ovc != r.ovc) {
			// TODO check if these conditions are correct
			if (l.ovc < r.ovc) {
				setWinner(curr, l);
				setLooser(curr, r);
			} else {
				setWinner(curr, r);
				setLooser(curr, l);
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
				setLooser(curr, l);
			} else {
				value = currentRow[i];
				setWinner(curr, l);
				setLooser(curr, r);
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

		curr.winnerNT = NT_INODE;
		curr.winnerKey = r.key;
		curr.winnerR = r.r;
		curr.winnerOVC = r.ovc;
	} else if (r.nodeType == NT_LINF) {
		curr.nodeType = NT_INF;
		curr.key = NULL;
		curr.r = NULL;
		curr.ovc = INV;

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

		curr.winnerNT = NT_INODE;
		curr.winnerKey = r.key;
		curr.winnerR = r.r;
		curr.winnerOVC = r.ovc;
	} else if (r.nodeType == NT_INF) {
		curr.nodeType = NT_INF;
		curr.key = NULL;
		curr.r = NULL;
		curr.ovc = INV;

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
	// save looser in current node key and r
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

TOL::pass() {
	// Set next key in the root winner's run to be the value at that run's leaf
	int leafIndex = nodeList[0].winnerIndex;
	nodeList[leafIndex] = nodeList[0].winnerR->getNext();
	// Compare the children of the current node where current node is the parent of the updated leaf all the way up the tree
	int parentNodeIndex = (leafIndex - 1) / 2;
	while (parentNodeIndex >= 0) {
		cmpNodes(nodeList[parentNodeIndex], nodeList[2 * parentNodeIndex + 1], nodeList[2 * parentNodeIndex + 2]);
		parentNodeIndex = (parentNodeIndex - 1) / 2;
	}
}

TOL::TOL(size_t nor, Run **rl, Run *o, ETable _t): runList(rl), output(o), numOfRun(nor), _t(t)
{
	if (nor > 256)
		throw std::runtime_error("TOL doesn't fit in cache");

	int tol_height = ceil(log2(nor)) + 1;
	numNodes = pow(2, tol_height) - 1;
	nodeList = new Node[numNodes];

	// init leaf nodes
	int firstLeaf = pow(2, tol_height - 1) - 1;
	int lastLeaf = pow(2, tol_height) - 2;
	for (int i = 0; i < (lastLeaf - firstLeaf); i++) {
		int li = firstLeaf + i;
		nodeList[li].index = i;
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
			nodeList[li].key = runList[i]->getNext();
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

TOL::~TOL()
{
	delete[] nodeList;
}
