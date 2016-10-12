
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

// size = 64 bytes along 16 alignments
struct image_task
{
  unsigned int components;
  unsigned int sections;
  region<int> work;
  range<int> size;
  unsigned long long output;
  unsigned long long input;
  char unused[16];
};

#endif

