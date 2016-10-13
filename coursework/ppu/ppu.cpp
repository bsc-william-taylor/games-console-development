
#include <iostream>
#include <vector>
#include <string>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "../../common/stb_image_resize.h"
#include "../../common/stb_image_write.h"
#include "../../common/stb_image.h"
#include "../../common/ppu_manager.h"
#include "../../common/benchmark.h"
#include "../structures.h"

const std::string outputImage = "../assets/greyscale.bmp";
const std::string inputImage = "../assets/picture.bmp";
const std::string spuExe = "./spu/spu";
const std::string name = "coursework";

void read_image(image_task& task, std::string fn)
{
  const int fixedHeight = 600;
  const int fixedWidth = 600;

  int w, h, n;
  unsigned char* bitmap = stbi_load(fn.c_str(), &w, &h, &n, 0);
  unsigned char* raw = (unsigned char *)malloc(fixedHeight * fixedWidth * n);
  assert(stbir_resize_uint8(bitmap, w, h, 0, raw, fixedWidth, fixedHeight, 0, n) == 1);
  
  task.components = n;
  task.input = (unsigned long long)raw;
  task.size.h = fixedHeight;
  task.size.w = fixedWidth;
  task.output = 0;

  stbi_image_free(bitmap);
}

int main(int argc, char * argv[])
{
	ppu_manager ppu_manager(true);
	benchmark track(name);

	const int spe_count = ppu_manager.spe_count();
	std::vector<std::pair<std::string, int> > programs;
	programs.push_back(std::pair<std::string, int>(spuExe, spe_count));

  const int processes = programs.size(); 
  image_task task __attribute__((aligned(16)));
  read_image(task, inputImage); 
  
  unsigned char buffer[task.size.w*task.size.h*task.components];
  task.sections = spe_count;
  task.output = (unsigned long long)buffer;

  for(int i = 0; i < processes; ++i)
	{ 
    ppu_manager.spe_arg((void*)&task, sizeof(image_task));
		ppu_manager.spe_program(programs[i].first);
	 	ppu_manager.spe_run(programs[i].second);
  }

  int w = task.size.w, h = task.size.h, n = task.components;
  stbi_write_bmp(outputImage.c_str(), w, h, n, (void*)task.output);	
  return 0;
}
