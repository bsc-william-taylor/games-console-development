
#ifndef __MFC_H__
#define __MFC_H__

#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>

inline void write(int bufferSize, int chunkSize, unsigned char* output, unsigned long long writeAt, int tagID)
{
    int bytesWritten = 0;
    int index = -1;

    while(bytesWritten < bufferSize)
    {
        int size = chunkSize;

        if(bytesWritten + size >= bufferSize)
        {
            size = bufferSize - bytesWritten;
        }

        unsigned char buffer[size];
        for(int i = 0; i < size; i++)
        {
            buffer[i] = output[++index];
        }

        mfc_put(buffer, writeAt, size, tagID, 0, 0);
        mfc_read_tag_status_all();

        bytesWritten += size;
        writeAt += size;
    }
}

inline void read(int bufferSize, int chunkSize, unsigned char* input, unsigned long long readAt, int tagID)
{
    int bytesRead = 0;
    int index = -1;

    while(bytesRead < bufferSize)
    {
        int size = chunkSize;

        if(bytesRead + size >= bufferSize)
        {
            size = bufferSize - bytesRead;
        }

        unsigned char buffer[size];
        mfc_get(buffer, readAt, size, tagID, 0, 0);
        mfc_read_tag_status_any();

        for(int i =  0; i < size; i++)
        {
            input[++index] = buffer[i];
        }

        bytesRead += size;
        readAt += size;
    }
}

#endif
