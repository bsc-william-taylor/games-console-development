
#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>
#include <vector>

#include "../structures.h"
#include "../mfc.h"

const int chunkSize = 15360;
const int tagID = 4;

const double sobel_filter_y[3][3] =
{
    { 1,  2,  1 },
    { 0,  0,  0 },
    { -1, -2, -1 }
};

const double sobel_filter_x[3][3] =
{
    { 1.5,  0,  -1.5 },
    { 3,    0,  -3 },
    { 1.5,  0,  -1.5 }
};

double px(byte* pixels, int x, int y, int w, int h)
{
    if (x < 0 || y < 0)
        return px(pixels, std::max(x, 0), std::max(y, 0), w, h);
    if (x >= w || y >= h)
        return px(pixels, std::min(x, w - 1), std::min(y, h - 1), w, h);

    return pixels[x + w * y];
}

double sobel_op(byte* pixels, int x, int y, int w, int h)
{
    double x_weight = 0.0;
    double y_weight = 0.0;
    double window[3][3] =
    {
        { px(pixels, x - 1, y - 1, w, h), px(pixels, x, y - 1, w, h),  px(pixels, x + 1, y - 1, w, h) },
        { px(pixels, x - 1, y    , w, h), px(pixels, x, y   , w, h),   px(pixels, x + 1, y    , w, h) },
        { px(pixels, x - 1, y + 1, w, h), px(pixels, x, y + 1, w, h),  px(pixels, x + 1, y + 1, w, h) }
    };

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            x_weight += window[i][j] * sobel_filter_x[i][j];
            y_weight += window[i][j] * sobel_filter_y[i][j];
        }
    }

    return ceil(sqrt(x_weight * x_weight + y_weight * y_weight)) * 2.0;
}

void sobel_filter(byte* output, byte* input, int w, int h)
{
    int x = 0, y = 0;

    for (int i = 0; i < h*w; i++, x++)
    {
        if (!(x < w))
        {
            x = 0;
            y++;
        }

        int value = (int)sobel_op(input, x, y, w, h);

        if (value <= 50)
            value = 0;
        else
            value *= 3.0;

        int clamped = clamp(0, 255, int(value));
        output[i] = clamped == 0 ? 0 : 255;
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

    sobel_filter(output, input, 640,  80);
    write(bufferSize, chunkSize, output, task.output + bufferStart, tagID);    
    return 0;
}
