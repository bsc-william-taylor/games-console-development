
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <algorithm>
#include <stdio.h>
#include <math.h>

#include "../structures.h"

const long long chunkSize = 128*128;
const int tagID = 3;

typedef unsigned char byte;

void greyscale(byte* out, byte* pixels, int length, int rgba)
{
  for(int i = 0; i < length; i+=rgba)
  {
    double r = (double)pixels[i + 0];
    double g = (double)pixels[i + 1];
    double b = (double)pixels[i + 2];

    double shade = sqrt((r*r + g*g + b*b) / 3.0);
    double clamped = std::max(0.0, std::min(shade, 255.0));

    out[i + 0] = (byte)clamped;
    out[i + 1] = (byte)clamped;
    out[i + 2] = (byte)clamped;
  }  
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
  const unsigned int unique_identifier =  spu_read_in_mbox();

  if(argp != 0)
  {
    image_task task __attribute__((aligned(16)));
   
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_write_tag_mask(1<<tagID);
    mfc_read_tag_status_any();

    long long totalBytes = task.size.w * task.size.h * task.components;
    long long bufferSize = totalBytes / task.sections; 
    long long bufferStart = bufferSize * unique_identifier;   
    long long writeAt = task.output;    
    long long readAt = task.input;
    long long bytesWritten = 0; 
  
    while(bytesWritten < bufferSize)
    {
      long long bytesToMove = 0;

      if(bytesWritten + chunkSize > bufferSize)
      {
        bytesToMove = 48;
      }
      else
      {
        bytesToMove = chunkSize;
      }

      byte output[bytesToMove];
      byte input[bytesToMove];

      mfc_get(input, readAt + bufferStart, bytesToMove, tagID, 0, 0);
      mfc_read_tag_status_any();

      greyscale(output, input, bytesToMove, task.components);
      
      mfc_put(output, writeAt + bufferStart, bytesToMove, tagID, 0, 0);
      mfc_read_tag_status_all();
      
      bytesWritten += bytesToMove;
      writeAt += bytesToMove;
      readAt += bytesToMove;
    }
  }

  return 0;
}
