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

TOL::TOL(size_t nor, Run **rl, Run *o): runList(rl), output(o), numOfRun(nor)
{
	if (nor > 256)
		throw std::runtime_error("TOL doesn't fit in cache");
	int tol_height = ceil(log2(nor)) + 1;
}
