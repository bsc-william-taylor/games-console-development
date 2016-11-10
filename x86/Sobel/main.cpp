#define _USE_MATH_DEFINES

#include "FreeImage.h"

#pragma comment(lib, "freeimage.lib")

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <ctime>

using byte = unsigned char;

void timeit(std::function<void()> operation) 
{
    auto start = clock();
    operation();
    auto ms = (clock() - start) / static_cast<double>(CLOCKS_PER_SEC / 1000);
    std::cout << "Finished in " << ms << "ms" << std::endl;
    std::cin.get();
}

const double sobel_filter_y[3][3] =
{
    {  1,  2,  1 },
    {  0,  0,  0 },
    { -1, -2, -1 }
};

const double sobel_filter_x[3][3] =
{
    { 1.5,  0,  -1.5 },
    { 3,    0,  -3 },
    { 1.5,  0,  -1.5 }
};

template<typename T>
T clamp(T min, T max, T v)
{
    return std::max(min, std::min(v, max));
}

double px(byte* pixels, int x, int y, int w, int h)
{
    if (x < 0 || y < 0)
        return px(pixels, std::max(x, 0), std::max(y, 0), w, h);
    if (x >= w || y >= h)
        return px(pixels, std::min(x, w - 1), std::min(y, h - 1), w, h);

    auto index = (x + w * y) * 3;
    auto r = static_cast<int>(pixels[index + 0]);
    auto g = static_cast<int>(pixels[index + 1]);
    auto b = static_cast<int>(pixels[index + 2]);
    return (r + g + b) / 3.0;
}

double sobel_op(byte* pixels, int x, int y, int w, int h)
{
    auto x_weight = 0.0;
    auto y_weight = 0.0;

    double window[3][3] =
    {
        { px(pixels, x - 1, y - 1, w, h), px(pixels, x, y - 1, w, h),  px(pixels, x + 1, y - 1, w, h) },
        { px(pixels, x - 1, y    , w, h), px(pixels, x, y   , w, h),   px(pixels, x + 1, y    , w, h) },
        { px(pixels, x - 1, y + 1, w, h), px(pixels, x, y + 1, w, h),  px(pixels, x + 1, y + 1, w, h) }
    };

    for (auto i = 0; i < 3; i++)
    {
        for (auto j = 0; j < 3; j++)
        {
            x_weight += window[i][j] * sobel_filter_x[i][j];
            y_weight += window[i][j] * sobel_filter_y[i][j];
        }
    }

    return ceil(sqrt(x_weight * x_weight + y_weight * y_weight)) * 2.0;
}

void sobel_filter(FIBITMAP * image)
{
    auto clone = FreeImage_Clone(image);
    auto output = FreeImage_GetBits(image);
    auto input = FreeImage_GetBits(clone);
    auto comp = FreeImage_GetBPP(image) / 8;
    auto h = FreeImage_GetHeight(image);
    auto w = FreeImage_GetWidth(image);
    auto x = 0, y = 0;

    for (auto i = 0u; i < h*w * 3; i += comp, x++)
    {
        if (!(x < w))
        {
            x = 0;
            y++;
        }

        auto value = sobel_op(input, x, y, w, h);

        if (value <= 50)
            value = 0;
        else
            value *= 3.0;

        auto clamped = clamp(0, 255, int(value));
        output[i + 0] = clamped == 0 ? 0 : 0;
        output[i + 1] = clamped == 0 ? 0 : 0;
        output[i + 2] = clamped == 0 ? 0 : 255;
    }

    FreeImage_Unload(clone);
}

template<typename T, int radius>
std::vector<T>  gaussian_kernel()
{
    std::vector<T> kernel(radius * 2 + 1);

    const T sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(2.0 * M_PI) * radius);
    const T twoRadiusSquaredRecip = 1.0 / (2.0 * radius * radius);

    T sum = (T)0.0;
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

    auto tempBytes = static_cast<unsigned char*>(malloc(h*w * 3));
    auto kernel = gaussian_kernel<double, R>();
    auto outBytes = FreeImage_GetBits(output);
    auto inBytes = FreeImage_GetBits(input);

    for (auto y = 0; y < h; y++)
    {
        for (auto x = 0; x < w; x++)
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

            for (auto k = 0; k < kernelSize; k++)
            {
                auto py = clamp(0, h - 1, y - R + k);
                auto index = py * w * 3 + x * 3;
                total += kernel[k] * tempBytes[index];
            }

            auto index = x * 3 + w*y * 3;
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

    for (auto i = 0; i < width*height * 3; i += 3)
    {
        if (outBytes[i + 1] == 255)
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

    return totalDensity / ((w - x) * (h - y)*255.0);
}

void fill(unsigned char * bytes, int x, int y, int w, int h, int r, int g, int b)
{
    for (auto py = y; py < h; py++)
    {
        for (auto px = x; px < w; px++)
        {
            auto index = px * 3 + 640 * py * 3;
            bytes[index + 0] = b;
            bytes[index + 1] = g;
            bytes[index + 2] = r;
        }
    }
}

void detect_windows(FIBITMAP * image, int width, int height, int step)
{
    auto clone = FreeImage_Clone(image);
    auto cloneBytes = FreeImage_GetBits(clone);
    auto bytes = FreeImage_GetBits(image);
    auto x = 0, y = 0;

    while (y < 480 && x < 640)
    {
        auto edgeDensity = edge_density(cloneBytes, x, y, std::min(x + width, 640), std::min(y + height, 480));

        if (edgeDensity >= 0.3)
        {
            fill(bytes, x, y, std::min(x + width, 640), std::min(y + height, 480), 0, 255, 0);
        }

        x += step;

        if (x >= 640)
        {
            y += step;
            x = 0;
        }
    }

    FreeImage_Unload(clone);
}

void detect_regions()
{
    FreeImage_Initialise();

    for (auto i = 1; i <= 10; i++)
    {
        auto in = "./inputs/" + std::to_string(i) + ".bmp";
        auto out = "./outputs/" + std::to_string(i) + "-out.bmp";
        auto bitmap = FreeImage_Load(FIF_BMP, in.c_str());
        auto mask = FreeImage_Clone(bitmap);

        gaussian_blur<6>(bitmap, mask);
        sobel_filter(mask);
        detect_windows(mask, 45, 45, 12);
        overlay_squares(bitmap, mask);

        FreeImage_Save(FIF_BMP, mask, out.c_str());
        FreeImage_Unload(mask);
        FreeImage_Unload(bitmap);
    }

    FreeImage_DeInitialise();
}

int main(int argc, char * argv[])
{
    timeit(detect_regions);
    return 0;
}