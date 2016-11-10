
#ifndef __SPU_BENCHMARK_H_
#define __SPU_BENCHMARK_H_

#include <spu_mfcio.h>
#include <stdio.h>

#define TICKS_PER_MS (79.8 * 1000.0)

class spu_benchmark 
{
private:
	unsigned long long start;
	const char* uniqueName;
	int spuID;
public:
	spu_benchmark(const char* name, int spuID) :
		uniqueName(name), spuID(spuID)
	{
		spu_write_decrementer(0);
	}

	~spu_benchmark() 
	{
		//#ifdef BENCHMARK_SPU
		double ms = -spu_read_decrementer() / TICKS_PER_MS;
		printf("%s(SPU -> %d): %.0fms \n", uniqueName, spuID, ms); 
		//#endif
	}
};

#endif
