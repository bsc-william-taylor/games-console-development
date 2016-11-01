
#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>
#include <vector>

#include "../structures.h"

const long long chunkSize = 9600;
const int blurMag = 6;
const int tagID = 3;

int kernel_size(int radius)
{
    return radius * 2 + 1;   
}

void gaussian_kernel(double* kernel, int radius)
{
    const int kernelSize = radius * 2 + 1;
    const double sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(2.0 * M_PI) * radius);
    const double twoRadiusSquaredRecip = 1.0 / (2.0 * radius * radius);

    double sum = 0.0;
    double r = -radius;

    for (int i = 0; i < kernelSize; ++i, ++r)
    {
        double x = r * r;
        kernel[i] = sqrtTwoPiTimesRadiusRecip * exp(-x * twoRadiusSquaredRecip);
        sum += kernel[i];
    }

    double div = sum;
    for (int i = 0; i < kernelSize; i++)
    {
        kernel[i] /= div;
    }
}

void blur(byte* outBytes, byte* inBytes, int length, image_task& task)
{
  const int size = kernel_size(blurMag);
  const int rgba = task.components;
  const int w = task.size.w;
  const int h = task.size.h;

  double kernel[size];
  gaussian_kernel(kernel, blurMag);  
  byte tempBytes[chunkSize];

    for(int y = 0; y < h; y++)
    {
        for(int x = 0; x < w; x++)
        {
            int kernelSize = size;
            double total = 0.0;

            for (int k = 0; k < kernelSize; k++)
            {
                int px = clamp(0, w - 1, x - blurMag + k);
                int index = px * 3 + w * y * 3;
                total += kernel[k] * inBytes[index];
            }
            
            int index = x * 3 + w * y * 3;
            tempBytes[index + 0] = (byte)total;
            tempBytes[index + 1] = (byte)total;
            tempBytes[index + 2] = (byte)total;
        }
    }

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int kernelSize = size;
            double total = 0.0;

            for(int k = 0; k < kernelSize; k++)
            {
                int py = clamp(0, h - 1, y - blurMag + k);
                int index = py * w * 3 + x * 3;
                total += kernel[k] * tempBytes[index];
            }

            int index = x*3 + w*y*3;         
            outBytes[index + 0] = (byte)total;
            outBytes[index + 1] = (byte)total;
            outBytes[index + 2] = (byte)total;
        }
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

      blur(output, input, chunkSize, task);
      
      mfc_put(output, writeAt + bufferStart, chunkSize, tagID, 0, 0);
      mfc_read_tag_status_all();
      
      bytesWritten += chunkSize;
      writeAt += chunkSize;
      readAt += chunkSize;
      break;
    }  
  }

  return 0;
}
