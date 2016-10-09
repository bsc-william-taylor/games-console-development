
#include "../structures.h"
#include <stdio.h>
#include <spu_mfcio.h>
#include <math.h>
#include <algorithm>

int tagID = 0;

void grayscale(unsigned char * pixels, int x, int y, int w, int h)
{
  for(int i = x; i < w; ++i)
  {
    for(int j = y; j < h; ++j)
    {
      int index = i * j;
      pixels[index] = pixels[index];
    }
  } 
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
  if(argp != 0)
  {
    const unsigned int unique_identifier = spu_read_in_mbox();
    image_task task __attribute__((aligned(16)));
   
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_write_tag_mask(1<<tagID);
    mfc_read_tag_status_any();
  
    int sectionHeight = ceil((float)task.size.h / (float)task.sections);
    int sectionWidth = ceil((float)task.size.w / (float)task.sections);    

    int x = unique_identifier * sectionWidth;
    int y = unique_identifier * sectionHeight;
    int w = std::min(x + sectionWidth, task.size.w);
    int h = std::min(y + sectionHeight, task.size.h);

    printf("work region: (x, y) = %d,%d (w, h) = %d,%d \n", x, y, w, h); 
    grayscale((unsigned char *)task.bytes, x, y, w, h);
  }

  return 0;
}
