
#ifndef __BENCHMARK_H_
#define __BENCHMARK_H_

#include <ppu_intrinsics.h>
#include <stdio.h>
#include <string>

#define TICKS_PER_MS (79.8 * 1000.0)

class benchmark 
{
private:
	unsigned long long start;
	std::string uniqueName;
public:
	benchmark(const std::string& name) :
		uniqueName(name),
		start(__mftb())
	{
	}

	~benchmark() 
	{
		double ms =(__mftb() - start) / TICKS_PER_MS;
		printf("\n%s: %.0fms \n", uniqueName.c_str(), ms); 
	}
};

#endif
