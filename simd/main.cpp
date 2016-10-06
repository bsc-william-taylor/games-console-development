
#include <iostream>
#include <altivec.h>
#include <simdmath.h>

template<typename T, int L>
class micro_vector 
{
  T *insert, *start, *end;
  union store 
  {  
    __vector T v;
    T e[L];
  } data;
public:
  micro_vector(const T * values = 0, int length = -1)
  {
     setupPointers();
     memset(data.e, 0, this->length());
     add(values, length);
  }

  void add(const T * values, int length = -1)
  {
     if(values == 0)
     {
        return;
     }

     const int sz = length == -1 ? this->length() : length;
    
     for(int i = 0; i < sz; i++)
     {
        data.e[i] = values[i];
     }
  }

	void add(T value) 
  {
    (*insert) = value;
    ++insert;
  }

  void print()
  {
    for(int i = 0; i < length(); i++)
    {
      std::cout << data.e[i];
    }
  }

	T get(int index) 
  {
		return data.e[index];
	}

	int length() 
  { 
     return end - start; 
  }

  T& store()
  {
      return data.e[0];
  }

private:
  void setupPointers()
  {
    start = &data.e[0];
    end = &data.e[LEN(data.e)];
    insert = start;
  }
}; 

__vector float zero = (__vector float){ 0.0f, 0.0f, 0.0f, 0.0}; 

typedef __vector float floats;
typedef __vector int ints;
typedef __vector char chars;
typedef __vector bool bools;


int main(int argc, char * argv[])
{
  float numbers[4] = {1, 1, 1, 1};
	micro_vector<float, 4> elements(numbers);
  
  floats& e = (floats&)elements.store();
  e = vec_add(e, e);
  e = vec_sub(e, e);
  e = vec_madd(zero, e, e);

  elements.print();	
	return 0;
}
