
#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>
#include <vector>

#include "../structures.h"
#include "../mfc.h"

const int chunkSize = 15360;
const int tagID = 5;

void overlay_squares(byte* output, byte* input, byte*overlay)
{
    for (int i = 0; i < 640*80; i++)
    {
        output[i] = input[i] == 255 ? overlay[i] : overlay[i] * 0.4;
    }
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
    image_task task __attribute__((aligned(16)));
    
    mfc_write_tag_mask(1<<tagID);
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_read_tag_status_any();
    
    unsigned long long totalBytes = task.size.w * task.size.h;
    unsigned long long bufferSize = totalBytes / task.sections;
    unsigned long long bufferStart = bufferSize * spu_read_in_mbox();
    
    byte input[bufferSize], output[bufferSize], original[bufferSize];    

    read(bufferSize, chunkSize, input, task.output + bufferStart, tagID);
    read(bufferSize, chunkSize, original, task.original + bufferStart, tagID);

    overlay_squares(output, input, original);
    write(bufferSize, chunkSize, output, task.output + bufferStart, tagID);   
    return 0;
}
