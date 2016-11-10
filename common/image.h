
#ifndef __IMAGE__H_
#define __IMAGE__H_

#include "stb_image_write.h"
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

struct basic_image
{
    std::string filename;
    unsigned char* data;
    int height;
    int width;
};

template<typename T>
std::string filename(std::string path, T value, std::string ext)
{
    std::stringstream ss;
    ss << path;
    ss << value; 
    ss << ext;
    return ss.str();
}

//void load_images(std::vector<std::string>& fns, std::vector<basic_image>& imgs);
void load_images(char **fns, int number_of_files, basic_image *imgs);
void unload_images(basic_image* imgs, int count);
//void write_images(std::vector<basic_image>& imgs);
void write_image(basic_image& image);

#endif

