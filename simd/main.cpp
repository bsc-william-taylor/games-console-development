
#include "micro-vector.h"
#include "benchmark.h"

__vector float zero = (__vector float){ 0.0f, 0.0f, 0.0f, 0.0f}; 

const int TESTS = 10000;

int benchmark_simd()
{
  benchmark tracker("SIMD Benchmark");
  int sum = 0;

  for(int i = 0; i < TESTS; i++)
  {
    float numbers[4] = {1, 1, 1, 1};
    micro_vector<float, 4> vec;
    floats& f = (floats&)vec.store();

    for(int b = 0; b < TESTS; b++)
    {
      f = vec_add(f, f);
    }

    sum += (int)numbers[0];
    sum -= (int)numbers[1];
  }

  return sum;
}

int benchmark_math()
{
  benchmark tracker("Math Benchmark"); 
  int sum = 0;
   
  for(int i = 0; i < TESTS; i++)
  {
    float numbers[4] = {1, 1, 1, 1};
    
    for(int x = 0; x < TESTS; x++)
    {
      for(int b = 0; b < 4; b++)
      {
        numbers[b] += numbers[b];
      }
    }

    sum += (int)numbers[0];
    sum -= (int)numbers[1];
  } 

  return sum;
}

int main(int argc, char * argv[])
{
  printf("simd result: %d", benchmark_simd());
  printf("math result: %d", benchmark_math());
	return 0;
}
