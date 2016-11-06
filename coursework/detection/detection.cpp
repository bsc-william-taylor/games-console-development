
#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>
#include <vector>

#include "../structures.h"
#include "../mfc.h"

const int chunkSize = 15360;
const int tagID = 1;

double edge_density(unsigned char * bytes, int x, int y, int w, int h)
{
    double totalDensity = 0.0;

    for (int py = y; py < h; py++)
    {
        for (int px = x; px < w; px++)
        {
            int index = px + 640 * py;
            totalDensity += bytes[index];
        }
    }

    return totalDensity / ((w - x) * (h - y)*255.0);
}

void fill(byte* bytes, int x, int y, int w, int h, int c)
{
    for (int py = y; py < h; py++)
    {
        for (int px = x; px < w; px++)
        {
            bytes[px + 640 * py] = c;
        }
    }
}

void detect_windows(byte* output, byte* input, int width, int height, int step)
{
    int w = 640, h = 80;
    int x = 0, y = 0;

    while (y < 480 && x < 640)
    {
        double edgeDensity = edge_density(input, x, y, std::min(x + width, w), std::min(y + height, h));

        if (edgeDensity >= 0.3)
        {
            fill(output, x, y, std::min(x + width, w), std::min(y + height, h), 255);
        }

        x += step;

        if (x >= 640)
        {
            y += step;
            x = 0;
        }
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
    
    byte input[bufferSize], output[bufferSize];    
    read(bufferSize, chunkSize, input, task.output + bufferStart, tagID);
    detect_windows(output, input, 45, 45, 12);
    write(bufferSize, chunkSize, output, task.output + bufferStart, tagID);       
    return 0;
}
