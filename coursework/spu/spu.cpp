
#include "../structures.h"
#include <spu_intrinsics.h>
#include <stdio.h>
#include <spu_mfcio.h>
#include <math.h>
#include <algorithm>
//#include <libmisc.h>

// Because the libs are not setup corretly, shame on you UWS!
#define BUFFER_MAX 128 * 128 * 4

int tagID = 3;

typedef unsigned char byte;

void grayscale(byte* pixels, byte* out, int length, int rgba)
{
  for(int i = 0; i < length; ++i)
  {
    double r = (double)pixels[i * rgba + 0];
    double g = (double)pixels[i * rgba + 1];
    double b = (double)pixels[i * rgba + 2];

    double shade = sqrt((r*r + g*g + b*b) / 3.0);

    if(out != NULL) 
    {
      out[i * rgba + 0] = (byte)std::min(shade, 255.0);
      out[i * rgba + 1] = (byte)std::min(shade, 255.0);
      out[i * rgba + 2] = (byte)std::min(shade, 255.0);
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
   
    //printf("%d image-size: %llu \n", unique_identifier, totalBytes);
    //printf("buffer size: %llu, offset: %llu, total: %llu  \n", bufferSize, bufferStart, totalBytes); 
    //printf("work region: (x, y) = %d,%d (w, h) = %d,%d bytes: %d \n", x, y, w, h, bufferSize); 
       
    unsigned long long writeAt = task.output;//(unsigned long long)(ea + bufferStart);    
    unsigned long long bytesWritten = 0; 
    unsigned long long chunkSize = 128*128;
  
    //unsigned long long diff = writeAt - task.output; 
    //printf("at: %llu, ea: %llu, diff: %llu \n", writeAt, task.output, diff);
    while(bytesWritten < bufferSize)
    {
      unsigned long long bytesToMove = 0;

      if(bytesWritten + chunkSize > bufferSize)
      {
        bytesToMove = 16;
      }
      else
      {
        bytesToMove = chunkSize;
      }

      byte output[bytesToMove];
      for(int z = 0; z < bytesToMove; ++z)
      {
        output[z] = 255;
      }     
      
      //printf("%d: at %llu, writing: %llu, written: %llu \n", unique_identifier, writeAt, bytesToMove, bytesWritten);  
      mfc_put(output, writeAt + bufferStart, bytesToMove, tagID, 0, 0);
      mfc_read_tag_status_all();
      
      writeAt += bytesToMove;
      bytesWritten += bytesToMove;
    }

    //printf("total bytes written: %llu \n", bytesWritten);
  }

  return 0;
}
