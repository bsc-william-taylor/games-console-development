#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "image.h"
#include "log.h"

struct m128c { __vector unsigned char e; };

void write_image(basic_image& img)
{
    const char* fn = img.filename.c_str();
    const int size = img.width * img.height * 3;
    unsigned char data[size];
    m128c *ptr = (m128c *)data;

    for(int i = 0, b = 0; i < size; i += 15, b += 5)
    {
        ptr->e = (__vector unsigned char){ img.data[b    ], img.data[b    ], img.data[b    ],
                                           img.data[b + 1], img.data[b + 1], img.data[b + 1],
                                           img.data[b + 2], img.data[b + 2], img.data[b + 2],
                                           img.data[b + 3], img.data[b + 3], img.data[b + 3],
                                           img.data[b + 4], img.data[b + 4], img.data[b + 4],
                                           0 };

        ptr = (m128c *)((char unsigned *)ptr + 15);
    }
    
    stbi_write_bmp(fn, img.width, img.height, 3, (void*)data);
}

void load_images(char **fns, int number_of_files, basic_image *imgs)
{
    for(int i = 0; i < number_of_files; i++)
    {
        int w, h, n;
        basic_image img;
        img.filename = fns[i];
        img.data = stbi_load(fns[i], &w, &h, &n, 1);
        img.height = h;
        img.width = w;
        imgs[i] = img;

        LOG("%s %s %s %d %d", "Loaded -> ", fns[i].c_str(), "(w, h)", w, h);
    }
}

void unload_images(basic_image *imgs, int count)
{
    for(int i = 0; i < count; i++)
    {
        LOG("%s %s", "Unloading -> ", imgs[i].filename.c_str());

        stbi_image_free(imgs[i].data);
    }
}
