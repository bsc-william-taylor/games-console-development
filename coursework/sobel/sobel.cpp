#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>

#include "../../common/spu_benchmark.h"
#include "../main.h"
#include "../mfc.h"

const int workRegionWidth = 640;
const int workRegionHeight = 80;
const int chunkSize = 16384;
const int tagID = 4;

const float sobel_filter_y[3][3] =
{
    {  1,  2,  1  },
    {  0,  0,  0  },
    { -1, -2, -1 }
};

const float sobel_filter_x[3][3] =
{
    { 1.5,  0,  -1.5 },
    { 3,    0,  -3   },
    { 1.5,  0,  -1.5 }
};

// Replacing std::min and std::max saved about 5 ms.
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

inline float px(byte* pixels, int x, int y, int w, int h)
{
    if (x < 0 || y < 0)
    {
        return px(pixels, max(x, 0), max(y, 0), w, h);
    }

    if (x >= w || y >= h)
    {
        return px(pixels, min(x, w - 1), min(y, h - 1), w, h);
    }

    return pixels[x + w * y];
}

int main(unsigned long long speID, unsigned long long argp, unsigned long long envp)
{
    const int uniqueID = spu_read_in_mbox();
    spu_benchmark track("sobel", uniqueID+1);
    image_task task __attribute__((aligned(16)));
    
    mfc_write_tag_mask(1<<tagID);
    mfc_get((void*)&task, argp, envp, tagID, 0, 0);
    mfc_read_tag_status_any();
    
    unsigned long long totalBytes = task.size.w * task.size.h;
    unsigned long long bufferSize = totalBytes / task.sections;
    unsigned long long bufferStart = bufferSize * uniqueID;

    byte input[bufferSize], output[bufferSize];    
    read(bufferSize, chunkSize, input, task.output + bufferStart, tagID);

    int x = 0, y = 0;

    for (int i = 0, dim = workRegionHeight*workRegionWidth; i < dim; ++i, ++x)
    {
        if (x >= workRegionWidth)
        {
            x = 0;
            y++;
        }

        float x_weight = 0.0;
        float y_weight = 0.0;
        float window[3][3] =
        {
            { px(input, x - 1, y - 1, workRegionWidth, workRegionHeight), px(input, x, y - 1, workRegionWidth, workRegionHeight),  px(input, x + 1, y - 1, workRegionWidth, workRegionHeight) },
            { px(input, x - 1, y    , workRegionWidth, workRegionHeight), px(input, x, y    , workRegionWidth, workRegionHeight),  px(input, x + 1, y    , workRegionWidth, workRegionHeight) },
            { px(input, x - 1, y + 1, workRegionWidth, workRegionHeight), px(input, x, y + 1, workRegionWidth, workRegionHeight),  px(input, x + 1, y + 1, workRegionWidth, workRegionHeight) }
        };

        for (int j = 0; j < 3; ++j)
        {
            __vector float vec_w = { window[j][0], window[j][1], window[j][2], 0.0f };
            __vector float vec_x = { sobel_filter_x[j][0], sobel_filter_x[j][1], sobel_filter_x[j][2], 0.0f };
            __vector float vec_y = { sobel_filter_y[j][0], sobel_filter_y[j][1], sobel_filter_y[j][2], 0.0f };

            __vector float vec_x_weight = spu_mul(vec_w, vec_x);
            __vector float vec_y_weight = spu_mul(vec_w, vec_y);

            x_weight += vec_x_weight[0] + vec_x_weight[1] + vec_x_weight[2];
            y_weight += vec_y_weight[0] + vec_y_weight[1] + vec_y_weight[2];
        }

        int value = (int)(ceil(sqrt(x_weight * x_weight + y_weight * y_weight)) * 2.0f);

        if (value <= 50)
        {
            value = 0;
        }
        else
        {
            value *= 3;
        }

        int clamped = clamp(0, 255, int(value));
        output[i] = clamped == 0 ? 0 : 255;
    }

    write(bufferSize, chunkSize, output, task.output + bufferStart, tagID);    
    return 0;
}
