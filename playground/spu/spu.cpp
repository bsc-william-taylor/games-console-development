
#include <stdio.h>

typedef unsigned long long ulong64;

int main(ulong64 speID, ulong64 argp, ulong64 envp)
{
	printf("HelloWorld SPE-ID=%lld", speID);
	return 0;
}
