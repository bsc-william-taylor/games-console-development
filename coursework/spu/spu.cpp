
#include "../structures.h"
#include <spu_intrinsics.h>
#include <stdio.h>
#include <spu_mfcio.h>
#include <math.h>
#include <algorithm>

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

    if(out != NULL) 
    {
      out[i + 0] = shade;
      out[i + 1] = shade;
      out[i + 2] = shade;
    }
  }  
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
  if(argp != 0)
  {
    const unsigned int unique_identifier =  spu_read_in_mbox();
    image_task task __attribute__((aligned(16)));
   
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_write_tag_mask(1<<tagID);
    mfc_read_tag_status_any();

    const unsigned long long totalBytes = task.size.w * task.size.h * task.components;
    const unsigned long long bufferSize = totalBytes / task.sections; 
    const unsigned long long bufferStart = bufferSize * unique_identifier;   
       
    unsigned long long writeAt = task.output;    
    unsigned long long readAt = task.input;
    unsigned long long bytesWritten = 0; 
    unsigned long long chunkSize = 128*128;
  
    while(bytesWritten < bufferSize)
    {
      unsigned long long bytesToMove = 0;

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
      
      writeAt += bytesToMove;
      readAt += bytesToMove;
      bytesWritten += bytesToMove;
    }
  }

  return 0;
}
