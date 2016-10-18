
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

const double sobel_filter_x[kernelSize][kernelSize] = 
{
  { -0.25, 0, 0.25 },
  { -0.5, 0, 0.5 },
  { -0.25, 0, 0.25 }
};


const double sobel_filter_y[kernelSize][kernelSize] = 
{
  {  -0.25, -0.5,  -0.25 },
  {  0,  0,  0 },
  {  0.25,  0.5,  0.25 }
};

double px(byte* pixels, int x, int y, int w, int h)
{
  if(x < 0 || y < 0)
    return px(pixels, std::max(0, x), std::max(0, y), w, h);
  if(x >= w || y >= h)
    return px(pixels, std::min(x, w-1), std::min(y, h-1), w, h);

  int index = x + (w * y);
  int r =  (int)pixels[(index*3)+0];
  int g =  (int)pixels[(index*3)+1];
  int b =  (int)pixels[(index*3)+2];
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

  return ceil(sqrt((x_weight * x_weight) + (y_weight * y_weight)));
}

void sobel_filter(byte* out, byte* pixels, int length, image_task& task)
{
  const int rgba = task.components;
  const int w = task.size.w;
  const int h = task.size.h;

  int x = 0, y = 0;

  for(int i = 0; i < length; i+=rgba, ++x)
  {
    if(x >= w)
    {
      x = 0;
      ++y;
    }

    int value = sobel_op(pixels, x, y, w, h);
    int clamped = std::max(0, std::min(value, 255));

    if(clamped < 120)
    {
      clamped = 0;
    }
    else
    {
      clamped = 255;
    }

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
  
    while(bytesWritten < bufferSize)
    {
      byte output[chunkSize];
      byte input[chunkSize];

      mfc_get(input, readAt + bufferStart, chunkSize, tagID, 0, 0);
      mfc_read_tag_status_any();

      sobel_filter(output, input, chunkSize, task);
      
      mfc_put(output, writeAt + bufferStart, chunkSize, tagID, 0, 0);
      mfc_read_tag_status_all();
      
      bytesWritten += chunkSize;
      writeAt += chunkSize;
      readAt += chunkSize;
    }
  }

  return 0;
}
