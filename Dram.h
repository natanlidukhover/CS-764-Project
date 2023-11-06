#ifndef _DRAM_H_
#define _DRAM_H_

#include <cstdlib>

class Dram
{
public:
	Dram(size_t Size);
	~Dram();
	void *GetSpace(size_t Size);
	void *FreeSpace(void *ptr, size_t size);
private:
	size_t _Size, _SizeOccupied;
	void *StartPtr, *FreePtr;
};

extern Dram dram;

#endif // _DRAM_H