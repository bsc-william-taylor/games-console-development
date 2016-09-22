
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../stb_image_write.h"
#include "../stb_image.h"

int main(int argc, char * argv[])
{
	std::cout << "Image Loading..." << std::endl;
	
	int width, height, components;

	unsigned char * data = stbi_load("../picture.bmp", &width, &height, &components, 0);
	
	std::cout << "w:" << width << " h: " << height << std::endl;
	std::cout << "writing new image duplicate.png" << std::endl;

	stbi_write_png("../duplicate.png", width, height, components, data, 0); 	
	stbi_image_free(data);
	return 0;
}
