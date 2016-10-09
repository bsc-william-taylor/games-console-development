
#ifndef __STRUCTS_H_
#define __STRUCTS_H_

template<typename T>
struct point
{
  T x;
  T y;
};

template<typename T>
struct range
{
  T w;
  T h;  
};

template<typename T>
struct region
{
  T x, y, w, h;
};

struct image_task
{
  unsigned int sections;
  region<int> work;
  range<int> size;
  void* bytes;
  char unused[8];
};

#endif

