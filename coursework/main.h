
#ifndef __MAIN_H_
#define __MAIN_H_

#include <math.h>

// size: 8 bytes
template<typename T>
struct point
{
    T x;
    T y;
};

// size: 8 bytes
template<typename T>
struct range
{
    T w;
    T h;
};

// size: 16 bytes
template<typename T>
struct region
{
    T x, y, w, h;
};

// size: 64 bytes
struct image_task
{
    unsigned long long original;
    unsigned long long output;
    unsigned long long input;
    unsigned int components;
    unsigned int sections;
    region<int> work;
    range<int> size;
    char unused[8];
};

typedef unsigned char byte;

#define clamp(min, max, v) ((v) < (min)) ? (min) : (((v) > (max)) ? (max) : (v))

#endif
