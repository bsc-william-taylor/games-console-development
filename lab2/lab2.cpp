
#include <iostream>
#include "../common/micro-vector.h"

struct vectors 
{
  micro_vector<float, 4> floats;
  micro_vector<int, 4> ints;
}

int main(int argc, char * argv[])
{
  float numbers[4] = {1, 1, 1, 1};
	micro_vector<float, 4> elements(numbers);
  
  floats& e = (floats&)elements.store();
  e = vec_add(e, e);
  e = vec_sub(e, e);
  e = vec_madd(e, e, e);

  elements.print();	
	return 0;
}
