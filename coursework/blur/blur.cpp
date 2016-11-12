#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>

#include "../../common/spu_benchmark.h"
#include "../main.h"
#include "../mfc.h"

const int workRegionHeight = 80;
const int workRegionWidth = 640;
const int chunkSize = 16384;
const int blurMag = 6;
const int tagID = 3;

#define kernel_size(radius) ((radius) * 2 + 1)

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
    const int spuID = spu_read_in_mbox();
    spu_benchmark track("blur", spuID+1);
    image_task task __attribute__((aligned(16)));

    mfc_write_tag_mask(1<<tagID);
    mfc_get((void *)&task, argp, envp, tagID, 0, 0);
    mfc_read_tag_status_any();

    unsigned long long totalBytes = task.size.w * task.size.h;
    unsigned long long bufferSize = totalBytes / task.sections;
    unsigned long long bufferStart = bufferSize * spuID;

    byte input[bufferSize], output[bufferSize];
    read(bufferSize, chunkSize, input, task.input + bufferStart, tagID);

    const int size = kernel_size(blurMag);
    float kernel[size];
    {
        const int kernelSize = kernel_size(blurMag);
        const float sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(2.0 * M_PI) * blurMag);
        const float twoRadiusSquaredRecip = 1.0 / (2.0 * blurMag * blurMag);

        float sum = 0.0;
        float r = -blurMag;

        for (int i = 0; i < kernelSize; ++i, ++r)
        {
            float x = r * r;
            kernel[i] = sqrtTwoPiTimesRadiusRecip * exp(-x * twoRadiusSquaredRecip);
            sum += kernel[i];
        }

        float div = sum;
        for (int i = 0; i < kernelSize; ++i)
        {
            kernel[i] /= div;
        }
    }

    byte tempBytes[workRegionWidth*workRegionHeight];

    for(int y = 0; y < workRegionHeight; y++)
    {
        for(int x = 0; x < workRegionWidth; x++)
        {
            int kernelSize = size;
            float total = 0.0;

            for (int k = 0; k < kernelSize; k++)
            {
                int px = clamp(0, workRegionWidth - 1, x - blurMag + k);
                total += kernel[k] * input[px+workRegionWidth*y];
            }

            tempBytes[x+workRegionWidth*y] = (byte)total;
        }
    }

    for (int y = 0; y < workRegionHeight; y++)
    {
        for (int x = 0; x < workRegionWidth; x++)
        {
            int kernelSize = size;
            float total = 0.0;

            for(int k = 0; k < kernelSize; k++)
            {
                int py = clamp(0, workRegionHeight - 1, y - blurMag + k);
                total += kernel[k] * tempBytes[py*workRegionWidth+x];
            }

            output[x+workRegionWidth*y] = (byte)total;
        }
    }
    
    write(bufferSize, chunkSize, output, task.output + bufferStart, tagID);
    return 0;
}
