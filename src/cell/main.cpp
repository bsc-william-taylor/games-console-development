
#include <iostream>
#include <vector>
#include <string>

#define BENCHMARK_PPU

#include "../common/ppu_benchmark.h"
#include "../common/spu_manager.h"
#include "../common/image.h"
#include "../common/log.h"
#include "main.h"

const char* programs[4] =
{
    "./blur/blur",
    "./sobel/sobel",
    "./detection/detection",
    "./overlay/overlay"
};

void write_output(std::string filename, unsigned char* data, int w, int h)
{
    basic_image output;
    output.filename = filename;
    output.height = h;
    output.width = w;
    output.data = data;
    write_image(output);
}

void process_image(spu_manager& spu_manager, basic_image& image, int index)
{
    const int imageSize = image.width * image.height;

    unsigned char original[imageSize];
    unsigned char output[imageSize];

    for(int i = 0; i < imageSize; ++i)
    {
        original[i] = image.data[i];
    }

    image_task task __attribute__((aligned(16)));
    task.original = (unsigned long long)original;
    task.output = (unsigned long long)output;
    task.input = (unsigned long long)image.data;
    task.sections = spu_manager.spe_count();
    task.size.h = image.height;
    task.size.w = image.width;

    for(int i = 0; i < 4; i++)
    {
        LOG("%s %s", "Running -> ", programs[i]);

        spu_manager.spe_arg((void*)&task, sizeof(image_task));
        spu_manager.spe_program(programs[i]);
        spu_manager.spe_run(task.sections);
    }

    LOG("%s %s", "Writing output -> ", filename("./O", index+1, ".bmp").c_str());

    write_output(filename("./O", index+1, ".bmp"), output, image.width, image.height);
}

int main(int argc, char * argv[])
{
    ppu_benchmark track("COURSEWORK");
    spu_manager spu_manager(false);

    const int number_of_files = 10;
    basic_image images[number_of_files];
    char *filenames[number_of_files] =
    {
        "../../assets/1.bmp",
        "../../assets/2.bmp",
        "../../assets/3.bmp",
        "../../assets/4.bmp",
        "../../assets/5.bmp",
        "../../assets/6.bmp",
        "../../assets/7.bmp",
        "../../assets/8.bmp",
        "../../assets/9.bmp",
        "../../assets/10.bmp"
    };

    load_images(filenames, number_of_files, images);

    LOG("%s %d %s", "Loaded & Processing", IMAGES, "images");

    for(int i = 0; i < number_of_files; ++i)
    {
        process_image(spu_manager, images[i], i);
    }

    unload_images(images, number_of_files);
    return 0;
}
