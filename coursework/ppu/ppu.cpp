
#include <iostream>
#include <vector>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "../../common/stb_image_resize.h"
#include "../../common/stb_image_write.h"
#include "../../common/stb_image.h"
#include "../../common/ppu_manager.h"
#include "../../common/benchmark.h"
#include "../structures.h"

const int fixedHeight = 600;
const int fixedWidth = 600;

void read_image(image_task& task, std::string fn)
{
  int w, h, n;
  unsigned char* bitmap = stbi_load(fn.c_str(), &w, &h, &n, 0);
  unsigned char* raw = (unsigned char *)malloc(fixedHeight * fixedWidth * n);
  assert(stbir_resize_uint8(bitmap, w, h, 0, raw, fixedWidth, fixedHeight, 0, n) == 1);
  
  task.size.w = fixedWidth;
  task.size.h = fixedHeight;
  task.input = (unsigned long long)raw;
  task.output = 0;
  task.components = n;
 
  printf("Image dimensions: (w, h, c) = %d, %d, %d \n", w, h, n);
}

int main(int argc, char * argv[])
{
	ppu_manager ppu_manager(true);
	benchmark track("coursework");

	const int spe_count = ppu_manager.spe_count();
	std::vector<std::pair<std::string, int> > programs;
	programs.push_back(std::pair<std::string, int>("./spu/spu", spe_count));

  const int processes = programs.size(); 
  image_task task __attribute__((aligned(16)));
  read_image(task, "../assets/picture.bmp"); 
  
  unsigned char buffer[task.size.w*task.size.h*task.components];
  memset(buffer, 0, sizeof(buffer));
  task.sections = spe_count;
  task.output = (unsigned long long)buffer;

  for(int i = 0; i < processes; ++i)
	{ 
    ppu_manager.spe_arg((void*)&task, sizeof(image_task));
		ppu_manager.spe_program(programs[i].first);
	 	ppu_manager.spe_run(programs[i].second);
  }

  assert(stbi_write_bmp("../assets/greyscale.bmp", task.size.w, task.size.h, task.components, (void*)task.output) == 1);	
  return 0;
}
