
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "image.h"
#include "log.h"

void write_images(std::vector<basic_image>& imgs)
{
    const int size = imgs.size();

    for(int i = 0; i < size; i++)
    {
        write_image(imgs[i]);
    }
}

void write_image(basic_image& img)
{
    const char* fn = img.filename.c_str();
    const int size = img.width * img.height * 3;
    unsigned char data[size];

    for(int i = 0, b = 0; i < size; i+=3, b++)
    {
        data[i+0] = img.data[b];
        data[i+1] = img.data[b];
        data[i+2] = img.data[b];
    }
    
    stbi_write_bmp(fn, img.width, img.height, 3, (void*)data);
}

void load_images(std::vector<std::string>& fns, std::vector<basic_image>& imgs)
{
    const int files = fns.size();

    for(int i = 0; i < files; i++)
    {
        int w, h, n;
        basic_image img;
        img.filename = fns[i];
        img.data = stbi_load(fns[i].c_str(), &w, &h, &n, 1);
        img.height = h;
        img.width = w;
        imgs.push_back(img);

        LOG("%s %s %s %d %d", "Loaded -> ", fns[i].c_str(), "(w, h)", w, h);
    }
}

void unload_images(std::vector<basic_image>& imgs)
{
    const int count = imgs.size();

    for(int i = 0; i < count; i++)
    {
        LOG("%s %s", "Unloading -> ", imgs[i].filename.c_str());

        stbi_image_free(imgs[i].data);
    }
}
