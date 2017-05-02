#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <ctime>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "../../common/stb_image_write.h"
#include "../../common/stb_image.h"
#include "../../common/visual_tunning.h"

using byte = unsigned char;

enum class measure_in
{
    secs = 1000000,
    ms = 1000
};

template<measure_in measure, typename Functor, typename... Args>
double benchmark(Functor&& method, Args&&... args)
{
    clock_t start = clock();
    method(std::forward<Args>(args)...);
    return (clock() - start) / (CLOCKS_PER_SEC  / double(measure));
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

    return pixels[x + w * y];
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

void sobel_filter(byte * input, byte* output, int w, int h)
{
    auto x = 0, y = 0;

    for (auto i = 0u; i < h*w; i++, x++)
    {
        if (!(x < w))
        {
            x = 0;
            y++;
        }

        auto value = sobel_op(input, x, y, w, h);

        if (value <= ACCEPTED_VALUE_FOR_EDGE)
        {
            value = 0;
        }    
        else
        {
            value *= 3.0;
        }

        auto clamped = clamp(0, 255, int(value));
        output[i] = clamped == 0 ? 0 : 255;
    }
}

template<typename T, int radius>
std::vector<T>  gaussian_kernel()
{
    std::vector<T> kernel(radius * 2 + 1);

    const T sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(2.0 * 3.14159) * radius);
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
void gaussian_blur(byte* inBytes, byte* outBytes, int w, int h)
{
    auto tempBytes = static_cast<unsigned char*>(malloc(h*w));
    auto kernel = gaussian_kernel<double, R>();

    for (auto y = 0; y < h; y++)
    {
        for (auto x = 0; x < w; x++)
        {
            auto kernelSize = kernel.size();
            auto total = 0.0;

            for (auto k = 0; k < kernelSize; k++)
            {
                auto px = clamp(0, w - 1, x - R + k);
                total += kernel[k] * inBytes[px + w * y];
            }

            tempBytes[x + w * y] = total;
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
                total += kernel[k] * tempBytes[py * w + x];
            }

            outBytes[x + w * y] = total;
        }
    }

    free(tempBytes);
}

void overlay_squares(byte * inBytes, byte* outBytes, int width, int height)
{
    for (auto i = 0; i < width*height; i++)
    {
        if (outBytes[i] == 255)
        {
            outBytes[i] = inBytes[i];
        }
        else
        {
            outBytes[i] = inBytes[i] * 0.5;
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
            totalDensity += bytes[px + 640 * py];
        }
    }

    return totalDensity / ((w - x) * (h - y)*255.0);
}

void fill(unsigned char * bytes, int x, int y, int w, int h, int c)
{
    for (auto py = y; py < h; py++)
    {
        for (auto px = x; px < w; px++)
        {
            bytes[px + 640 * py] = c;
        }
    }
}

void detect_windows(byte* input, byte* output, int width, int height, int step)
{
    auto x = 0, y = 0;

    while (y < 480 && x < 640)
    {
        auto edgeDensity = edge_density(input, x, y, std::min(x + width, 640), std::min(y + height, 480));

        if (edgeDensity >= IGNORABLE_EDGE_DENSITY)
        {
            fill(output, x, y, std::min(x + width, 640), std::min(y + height, 480), 255);
        }

        x += step;

        if (x >= 640)
        {
            y += step;
            x = 0;
        }
    }
}

void detect_regions()
{
    byte* outputBuffer1 = nullptr;
    byte* outputBuffer2 = nullptr;
    byte* outputImage = nullptr;

    for (auto i = 1; i <= 10; i++)
    {
        auto in = "./assets/" + std::to_string(i) + ".bmp";
        auto out = "./assets/" + std::to_string(i) + "-out.bmp";

        auto w = 0, h = 0, n = 0;
        auto bitmap = stbi_load(in.c_str(), &w, &h, &n, 1);
        auto outputSize = w*h*n;

        outputImage = outputImage != nullptr ? outputImage : static_cast<byte*>(malloc(outputSize));
        outputBuffer1 = outputBuffer1 != nullptr ? outputBuffer1 : static_cast<byte*>(malloc(w*h));
        outputBuffer2 = outputBuffer2 != nullptr ? outputBuffer2 : static_cast<byte*>(malloc(w*h));

        gaussian_blur<6>(bitmap, outputBuffer1, w, h);
        sobel_filter(outputBuffer1, outputBuffer2, w, h);
        detect_windows(outputBuffer2, outputBuffer1, REGION_LENGTH, REGION_LENGTH, PIXELS_PER_STEP);
        overlay_squares(bitmap, outputBuffer1, w, h);

        for(auto y = 0, x = 0; y < outputSize; y+=3, x++)
        {
            outputImage[y+0] = outputBuffer1[x];
            outputImage[y+1] = outputBuffer1[x];
            outputImage[y+2] = outputBuffer1[x];
        }

        stbi_write_bmp(out.c_str(), w, h, 3, outputImage);
        stbi_image_free(bitmap);
    }

    free(outputBuffer1);
    free(outputBuffer2);
    free(outputImage);
}

int main(int argc, char * argv[])
{
    auto ms = benchmark<measure_in::ms>(detect_regions);
    std::cout << "benchmark: " << ms << "ms " << std::endl;
    return std::cin.get();
}