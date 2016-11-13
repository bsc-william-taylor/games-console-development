#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>

#include "../../common/spu_benchmark.h"
#include "../../common/visual_tunning.h"
#include "../main.h"
#include "../mfc.h"

const int chunkSize = 16384;
const int outColour = 255;
const int tagID = 1;

const int workRegionWidth = 640;
const int workRegionHeight = 80;

const int halfRegionExpansion = 20;
const int regionExpansion = 40;

#define normalise(density, x, y, w, h) ((density) / (((w) - (x)) * ((h) - (y)) * 255.0f))
#define max(a, b) ((a) > (b)) ? (a) : (b)
#define min(a, b) ((a) < (b)) ? (a) : (b)

float edge_density(unsigned char * bytes, int x, int y, int w, int h)
{
    float totalDensity = 0.0;

    for (int py = y; py < h; py++)
    {
        for (int px = x; px < w; px++)
        {
            totalDensity += bytes[px + 640 * py];
        }
    }

    return normalise(totalDensity, x, y, w, h);
}

void fill(byte* bytes, int x, int y, int w, int h, int c)
{
    for (int py = y; py < h; py++)
    {
        for (int px = x; px < w; px++)
        {
            bytes[px + workRegionWidth * py] = c;
        }
    }
}

void detect_windows(byte* output, byte* input, int width, int height, int step, int padding)
{
    int w = workRegionWidth, h = workRegionHeight + padding;
    int x = 0, y = 0;

    while (y < h && x < w)
    {
        float edgeDensity = edge_density(input, x, y, min(x + width, w), min(y + height, h));

        if (edgeDensity >= IGNORABLE_EDGE_DENSITY)
        {
            fill(output, x, y, min(x + width, w), min(y + height, h), outColour);
        }
        
        x += step;

        if (x >= w)
        {
            y += step;
            x = 0;
        }
    }
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
    const int uniqueID = spu_read_in_mbox();
    spu_benchmark track("detection", uniqueID+1);
    image_task task __attribute__((aligned(16)));
    
    mfc_write_tag_mask(1<<tagID);
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_read_tag_status_any();
    
    unsigned long long totalBytes = task.size.w * task.size.h;
    unsigned long long bufferSize = totalBytes / task.sections;
    unsigned long long bufferStart = bufferSize * uniqueID;
    unsigned long long address = (task.output + bufferStart);
    unsigned long long padding = 0;

    if(uniqueID != 0 && uniqueID != 5)
    {
        bufferSize += workRegionWidth * regionExpansion;
        address -= workRegionWidth * halfRegionExpansion;
        padding = regionExpansion;
    }
   
    byte input[bufferSize], output[bufferSize];    
    read(bufferSize, chunkSize, input, address, tagID);

    detect_windows(output, input, REGION_LENGTH, REGION_LENGTH, PIXELS_PER_STEP, padding);

    write(bufferSize, chunkSize, output, address, tagID);    
    return 0;
}
