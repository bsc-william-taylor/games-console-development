
#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <math.h>
#include <algorithm>
#include "FreeImage.h"

#pragma comment(lib, "freeimage.lib")

typedef unsigned char byte;

const double sobel_filter_x[5][5] =
{
    { 2, 1, 0, -1, -2 },
    { 4, 2, 0, -2, -4 },
    { 4, 3, 0, -3, -4 },
    { 4, 2, 0, -2, -4 },
    { 2, 1, 0, -1, -2 }
};

const double sobel_filter_y[5][5] =
{
    {  2,  4,  4,  4,  2 },
    {  1,  2,  3,  2,  1 },
    {  0,  0,  0,  0,  0 },
    { -1, -2, -3, -2, -1 },
    { -2, -4, -4, -4, -2 }
};

template<typename T>
T clamp(T min, T max, T v)
{
    return std::max(min, std::min(v, max));
}

double px(byte* pixels, int x, int y, int w, int h)
{
    if (x < 0 || y < 0)
        return px(pixels, std::max(x,0), std::max(y, 0), w, h);
    if (x >= w || y >= h)
        return px(pixels, std::min(x, w-1), std::min(y, h-1), w, h);

    int index = (x + w * y) * 3;
    int r = (int)pixels[index + 0];
    int g = (int)pixels[index + 1];
    int b = (int)pixels[index + 2];
    return ((double)(r + g + b) / 3.0);
}

int sobel_op(byte* pixels, int x, int y, int w, int h)
{
    int x_weight = 0;
    int y_weight = 0;

    double window[5][5] =
    {
        { px(pixels, x - 2, y - 2, w, h), px(pixels, x - 1, y - 2, w, h),  px(pixels, x, y - 2, w, h), px(pixels, x+1, y - 2, w, h),  px(pixels, x + 2, y - 2, w, h) },
        { px(pixels, x - 2, y - 1, w, h), px(pixels, x - 1, y - 1, w, h),  px(pixels, x, y - 1, w, h), px(pixels, x+1, y - 1, w, h),  px(pixels, x + 2, y - 1, w, h) },
        { px(pixels, x - 2, y    , w, h), px(pixels, x - 1, y    , w, h),  px(pixels, x, y, w, h),     px(pixels, x+1, y    , w, h),  px(pixels, x + 2, y    , w, h) },
        { px(pixels, x - 2, y + 1, w, h), px(pixels, x - 1, y + 1, w, h),  px(pixels, x, y + 1, w, h), px(pixels, x+1, y + 1, w, h),  px(pixels, x + 2, y + 1, w, h) },
        { px(pixels, x - 2, y + 2, w, h), px(pixels, x - 1, y + 2, w, h),  px(pixels, x, y + 2, w, h), px(pixels, x+1, y + 2, w, h),  px(pixels, x + 2, y + 2, w, h) }
    };

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            x_weight += window[i][j] * sobel_filter_x[i][j];
            y_weight += window[i][j] * sobel_filter_y[i][j];
        }
    }

    return ceil(sqrt(x_weight * x_weight + y_weight * y_weight)) / 1.25;
}

int minSobel = 255, maxSobel = 0;

void sobel_filter(FIBITMAP * image)
{
    auto clone = FreeImage_Clone(image);
    auto output = FreeImage_GetBits(image);
    auto input = FreeImage_GetBits(clone);
    auto comp = FreeImage_GetBPP(image) / 8;
    auto h = FreeImage_GetHeight(image);
    auto w = FreeImage_GetWidth(image);
    auto x = 0, y = 0;

    for (auto i = 0; i < h*w * 3; i += comp, x++)
    {
        if (!(x < w))
        {
            x = 0;
            y++;
        }

        auto value = sobel_op(input, x, y, w, h);

        if(value < minSobel)
            minSobel = value;
        if(value > maxSobel)
            maxSobel = value;

        if (value <= 50)
            value = 0;
        else //if (value > 200)
            value *= 2.0;

        auto clamped = clamp(0, 255, value);
        output[i + 0] = clamped == 0 ? 0 : 0;
        output[i + 1] = clamped == 0 ? 0 : 0;
        output[i + 2] = clamped;// == 0 ? 0 : 255;
    }

    FreeImage_Unload(clone);
}

template<typename T, int radius>
std::vector<T>  gaussian_kernel()
{
    std::vector<T> kernel(radius * 2 + 1);

    const T sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(2.0 * M_PI) * radius);
    const T twoRadiusSquaredRecip = 1.0 / (2.0 * radius * radius);

    auto sum = 0.0;
    auto r = -radius;

    for (auto i = 0; i < kernel.size(); ++i, ++r)
    {
        T x = r * r;
        kernel[i] = sqrtTwoPiTimesRadiusRecip * exp(-x * twoRadiusSquaredRecip);
        sum += kernel[i];
    }

    auto div = sum;

    for (auto i = 0; i < kernel.size(); i++)
    {
        kernel[i] /= div;
    }

    return kernel;
}

template<int R>
void gaussian_blur(FIBITMAP* input, FIBITMAP* output)
{
    int h = FreeImage_GetHeight(input);
    int w = FreeImage_GetWidth(input);

    auto kernel = gaussian_kernel<double, R>();
    auto outBytes = FreeImage_GetBits(output);
    auto inBytes = FreeImage_GetBits(input);
    auto tempBytes = (unsigned char*)malloc(h*w * 3);
   
    for(auto y = 0; y < h; y++)
    {
        for(auto x = 0; x < w; x++)
        {
            auto kernelSize = kernel.size();
            auto total = 0.0;

            for (auto k = 0; k < kernelSize; k++)
            {
                auto px = clamp(0, w - 1, x - R + k);
                auto index = px * 3 + w * y * 3;
                total += kernel[k] * inBytes[index];
            }

            auto index = x * 3 + w * y * 3;
            tempBytes[index + 0] = total;
            tempBytes[index + 1] = total;
            tempBytes[index + 2] = total;
        }
    }

    for (auto y = 0; y < h; y++)
    {
        for (auto x = 0; x < w; x++)
        {
            auto kernelSize = kernel.size();
            auto total = 0.0;

            for(auto k = 0; k < kernelSize; k++)
            {
                auto py = clamp(0, h - 1, y - R + k);
                auto index = py * w * 3 + x * 3;
                total += kernel[k] * tempBytes[index];
            }

            auto index = x*3 + w*y*3;         
            outBytes[index + 0] = total;
            outBytes[index + 1] = total;
            outBytes[index + 2] = total;
        }
    }

    free(tempBytes);
}

void overlay_squares(FIBITMAP * image, FIBITMAP * overlay)
{
    auto outBytes = FreeImage_GetBits(overlay);
    auto inBytes = FreeImage_GetBits(image);
  
    auto height = FreeImage_GetHeight(image);
    auto width = FreeImage_GetWidth(image);

    for(int i = 0; i < width*height*3; i+=3)
    {
        if(outBytes[i+2] == 255)
        {
            outBytes[i + 0] = inBytes[i + 0];
            outBytes[i + 1] = inBytes[i + 1];
            outBytes[i + 2] = inBytes[i + 2];
        }
        else
        {
            outBytes[i + 0] = inBytes[i + 0] * 0.4;
            outBytes[i + 1] = inBytes[i + 1] * 0.4;
            outBytes[i + 2] = inBytes[i + 2] * 0.4;
        }
    }
}

double edge_density(unsigned char * bytes, int x, int y, int w, int h)
{
    auto totalDensity = 0.0;

    for (auto py = y; py < h; py++)
    {
        for (auto px = x; px < w; px++)
        {
            auto index = px * 3 + 640 * py * 3;
            totalDensity += bytes[index + 2];
        }
    }
    
    const auto maxDensity = 10.0*10.0*255.0;
    return totalDensity / maxDensity;
}

void fill(unsigned char * bytes, int x, int y, int w, int h)
{
    for (auto py = y; py < h; py++)
    {
        for (auto px = x; px < w; px++)
        {
            auto index = px * 3 + 640 * py * 3;
            bytes[index + 0] = 0;
            bytes[index + 1] = 0;
            bytes[index + 2] = 255;
        }
    }
}

void detect_windows(FIBITMAP * image)
{
    auto bytes = FreeImage_GetBits(image);
    auto height = 25;
    auto width = 25;
    auto x = 0, y = 0;

    auto minDensity = 5, maxDensity = 0;

    while(y < 480 && x < 640)
    {
        auto edgeDensity = edge_density(bytes, x, y, std::min(x+width, 640), std::min(y + height, 480));
    
        if(edgeDensity < minDensity)
            minDensity = edgeDensity;
        if(edgeDensity > maxDensity)
            maxDensity = edgeDensity;

        if(edgeDensity >= 0.025)
        {
            fill(bytes, x, y, std::min(x + width, 640), std::min(y + height, 480));
        }

        x += width;

        if(x >= 640)
        {
            y += height;
            x = 0;
        }
    }

    std::cout << "max density: " << maxDensity << " min density: " << minDensity << std::endl;
}

void process_image(std::string in, std::string out)
{
    auto bitmap = FreeImage_Load(FIF_BMP, in.c_str());
    auto mask = FreeImage_Clone(bitmap);

    gaussian_blur<15>(bitmap, mask);
    sobel_filter(mask);
    detect_windows(mask);
    overlay_squares(bitmap, mask);

    FreeImage_Save(FIF_BMP, mask, out.c_str());
    FreeImage_Unload(mask);
    FreeImage_Unload(bitmap);

    std::cout << "min: " << minSobel << " max:" << maxSobel << std::endl;
}

int main(int argc, char * argv[])
{
    FreeImage_Initialise();

    for(auto i = 1; i <= 10; i++)
    {
        auto in = "./inputs/" + std::to_string(i) + ".bmp";
        auto out = "./outputs/" + std::to_string(i) + "-out.bmp";
        process_image(in, out);
    }
 
    FreeImage_DeInitialise();
    return std::cin.get();
}