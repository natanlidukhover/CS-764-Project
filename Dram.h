class Dram
{
public:
	Dram(size_t Size);
	~Dram();
	void *GetSpace(size_t Size);
	int Dram::FreeSpace(void *ptr, size_t size);
private:
	size_t _Size, _SizeOccupied;
	void *StartPtr, *FreePtr;
}
