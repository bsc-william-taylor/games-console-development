
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <algorithm>
#include <stdio.h>
#include <math.h>

#include "../structures.h"

typedef unsigned char byte;

const long long chunkSize = 9600;
const int kernelSize = 3;
const int tagID = 3;

// INCREASE KERNAL SIZE???
const double sobel_filter_x[kernelSize][kernelSize] = 
{
  { -1, 0, 1 },
  { -2, 0, 2 },
  { -1, 0, 1 }
};


const double sobel_filter_y[kernelSize][kernelSize] = 
{
  {  -1, -2,  -1 },
  {  0,  0,  0 },
  {  1, 2,  1 }
};

double px(byte* pixels, int x, int y, int w, int h)
{
  if(x < 0 || y < 0)
    return 0;
  if(x >= 640 || y >= 480)
    return 0;

  int index = (x + w * y) * 3;
  int r =  (int)pixels[index+0];
  int g =  (int)pixels[index+1];
  int b =  (int)pixels[index+2];
  return ((double)(r + g + b) / 3.0);
}

int sobel_op(byte* pixels, int x, int y, int w, int h)
{
  int x_weight = 0;
  int y_weight = 0;

  double window[kernelSize][kernelSize] = 
  {
    { px(pixels, x-1, y-1, w, h), px(pixels, x, y-1, w, h),  px(pixels, x+1, y-1, w, h) },
    { px(pixels, x-1, y,   w, h), px(pixels, x, y,   w, h),  px(pixels, x+1, y,   w, h) },
    { px(pixels, x-1, y+1, w, h), px(pixels, x, y+1, w, h),  px(pixels, x+1, y+1, w, h) }
  };

  for(int i = 0; i < kernelSize; i++)
  {
    for(int j = 0; j < kernelSize; j++)
    {
      x_weight += window[i][j] * sobel_filter_x[i][j];
      y_weight += window[i][j] * sobel_filter_y[i][j];
    }
  }

  return px(pixels, x, y, w, h);//ceil(sqrt((x_weight * x_weight) + (y_weight * y_weight)));
}

void sobel_filter(byte* out, byte* pixels, int length, image_task& task, int&, int&)
{
  const int rgba = task.components;
  const int w = task.size.w;
  const int h = task.size.h;

  int x = 0, y = 0;

  for(int i = 0; i < length; i+=rgba, x++)
  {
    if(x >= w)
    {
      x = 0;
      ++y;
    }

    int value = sobel_op(pixels, x, y, w, h);
    int clamped = std::max(0, std::min(value, 255));

    out[i + 0] = (byte)clamped;
    out[i + 1] = (byte)clamped;
    out[i + 2] = (byte)clamped;
  }  
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
  mfc_write_tag_mask(1<<tagID);

  if(argp != 0)
  {
    image_task task __attribute__((aligned(16)));
   
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_read_tag_status_any();

    long long totalBytes = task.size.w * task.size.h * task.components;
    long long bufferSize = totalBytes / task.sections; 
    long long bufferStart = bufferSize * spu_read_in_mbox();   
    long long writeAt = task.output;    
    long long readAt = task.input;
    long long bytesWritten = 0; 
  
    int x = 0, y = 0;

    byte output[totalBytes];
    byte input[totalBytes];

    while(bytesWritten < bufferSize)
    {
      long long address = (long long)input;
      mfc_get((volatile void*)(address + bytesWritten), readAt + bufferStart, chunkSize, tagID, 0, 0);
      mfc_read_tag_status_any();
      
      bytesWritten += chunkSize;
      writeAt += chunkSize;
      readAt += chunkSize;
      //break;
    }

    /*
    while(bytesWritten < bufferSize)
    {
      sobel_filter(output, input, chunkSize, task, x, y);
      
      mfc_put(output, writeAt + bufferStart, chunkSize, tagID, 0, 0);
      mfc_read_tag_status_all();
      
      bytesWritten += chunkSize;
      writeAt += chunkSize;
      readAt += chunkSize;
    }*/    
  }

  return 0;
}
