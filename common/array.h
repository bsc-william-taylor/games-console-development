
#ifndef __ARRAY__H_
#define __ARRAY__H_

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

template<typename T, int L>
class array 
{
  T elements[L];
public:
  array() 
  {
    assert(L > 0);
    clear();
  }

  array(T first, ...)
  {
    assert(L > 0);
    int length = L;   
    elements[0] = first;

    va_list ap;
    va_start(ap, first);
    for (int i = 1; i < length; i++)
      elements[i] = va_arg(ap, T);
    va_end(ap);
  }

  T * begin()
  {
    return &elements[0];
  }

  T * end()
  {
    return begin() + length();
  }

  void clear()
  {
    memset(elements, 0, sizeof(elements));
  }

  int length()
  {
    return L;
  }

  int bytes()
  {
    return length() * sizeof(T);
  }

  T& operator[](int i)
  {
    return elements[i];
  }

  void * data()
  {
    return begin();
  }
};

#endif