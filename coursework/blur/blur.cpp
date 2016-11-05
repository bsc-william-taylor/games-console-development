
#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>
#include <algorithm>
#include <vector>

#include "../structures.h"

const int chunkSize = 15360;
const int blurMag = 6;
const int tagID = 3;

int kernel_size(int radius)
{
    return radius * 2 + 1;
}

void gaussian_kernel(double* kernel, int radius)
{
    const int kernelSize = radius * 2 + 1;
    const double sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(2.0 * M_PI) * radius);
    const double twoRadiusSquaredRecip = 1.0 / (2.0 * radius * radius);
    
    double sum = 0.0;
    double r = -radius;
    
    for (int i = 0; i < kernelSize; ++i, ++r)
    {
        double x = r * r;
        kernel[i] = sqrtTwoPiTimesRadiusRecip * exp(-x * twoRadiusSquaredRecip);
        sum += kernel[i];
    }
    
    double div = sum;
    for (int i = 0; i < kernelSize; i++)
    {
        kernel[i] /= div;
    }
}

void blur(byte* outBytes, byte* inBytes, int w, int h, image_task& task)
{
    const int size = kernel_size(blurMag);
    const int rgba = task.components;
    
    double kernel[size];
    gaussian_kernel(kernel, blurMag);
    byte tempBytes[w*h];
    
    for(int y = 0; y < h; y++)
    {
        for(int x = 0; x < w; x++)
        {
            int kernelSize = size;
            double total = 0.0;
            
            
            for (int k = 0; k < kernelSize; k++)
            {
                int px = clamp(0, w - 1, x - blurMag + k);
                int index = px + w * y;
                total += kernel[k] * inBytes[index];
            }
            
            int index = x + w * y;
            tempBytes[index] = (byte)total;
        }
    }
    
    
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int kernelSize = size;
            double total = 0.0;
            
            for(int k = 0; k < kernelSize; k++)
            {
                int py = clamp(0, h - 1, y - blurMag + k);
                int index = py * w + x;
                total += kernel[k] * tempBytes[index];
            }
            
            int index = x + w*y;
            outBytes[index] = (byte)total;
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
    unsigned long long writeAt = task.output + bufferStart;
    unsigned long long readAt = task.input + bufferStart;
    unsigned long long bytesWritten = 0;
    
    byte input[bufferSize], output[bufferSize];    
    int index = -1;
    
    while(bytesWritten < bufferSize)
    {
        int size = chunkSize;
        if(bytesWritten + size >= bufferSize)
            size = bufferSize - bytesWritten;
        
        byte buffer[size];
        mfc_get(buffer, readAt, size, tagID, 0, 0);
        mfc_read_tag_status_any();
        
        for(int i =  0; i < size; i++)
        {
            input[++index] = buffer[i];
        }
        
        bytesWritten += size;
        readAt += size;
    }
    
    blur(output, input, 640, 80, task);
    bytesWritten = 0;
    index = -1;
    
    while(bytesWritten < bufferSize)
    {
        int size = chunkSize;
        if(bytesWritten + size >= bufferSize)
            size = bufferSize - bytesWritten;
        
        byte buffer[size];
        for(int i = 0; i < size; i++)
        {
            buffer[i] = output[++index];
        }
        
        mfc_put(buffer, writeAt, size, tagID, 0, 0);
        mfc_read_tag_status_all();
        
        bytesWritten += size;
        writeAt += size;
    }
    
    return 0;
}
