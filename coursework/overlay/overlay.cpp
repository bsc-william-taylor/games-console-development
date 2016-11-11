#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>

#include "../../common/spu_benchmark.h"
#include "../main.h"
#include "../mfc.h"

const int workRegionHeight = 80;
const int workRegionWidth = 640;
const int chunkSize = 15360;
const int tagID = 5;

const double fade = 0.5;

void overlay_squares(byte* output, byte* input, byte*overlay)
{
    const int size = workRegionWidth*workRegionHeight;
    
    for (int i = 0; i < size; i++)
    {
        output[i] = input[i] == 255 ? overlay[i] : (byte)(overlay[i] * fade);
    }
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
    const int uniqueID = spu_read_in_mbox();
    spu_benchmark track("overlay", uniqueID+1);
    image_task task __attribute__((aligned(16)));
    
    mfc_write_tag_mask(1<<tagID);
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_read_tag_status_any();
    
    unsigned long long totalBytes = task.size.w * task.size.h;
    unsigned long long bufferSize = totalBytes / task.sections;
    unsigned long long bufferStart = bufferSize * uniqueID;
    
    byte input[bufferSize], output[bufferSize], original[bufferSize];    

    read(bufferSize, chunkSize, input, task.output + bufferStart, tagID);
    read(bufferSize, chunkSize, original, task.original + bufferStart, tagID);

    overlay_squares(output, input, original);

    write(bufferSize, chunkSize, output, task.output + bufferStart, tagID);   
    return 0;
}
