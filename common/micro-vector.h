

#ifndef __MICRO_VECTOR__H_
#define __MICRO_VECTOR__H_

#include <iostream>
#include <altivec.h>
#include <simdmath.h>

typedef __vector float floats;
typedef __vector char chars;
typedef __vector bool bools;
typedef __vector int ints;

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
    micro_vector(const T * values = 0)
    {
        insert = &data.e[0];
        start = &data.e[0];
        end = &data.e[sizeof(data.e) / sizeof(T)];
        add(values);
    }

    ~micro_vector() 
    {
    }

    int length() 
    { 
        return end - start; 
    }
    
    void add(const T * values)
    {
        if(values == NULL)
        {
            return;
        }

        const int len = length();

        for(int i = 0; i < len; i++)
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
        const int len = length();

        for(int i = 0; i < len; i++)
        {
            std::cout << data.e[i];
        }
    }

    T& store() 
    { 
        return data.e[0];
    }

	  T get(int index)
    {
        return data.e[index];
    }  
}; 

#endif
