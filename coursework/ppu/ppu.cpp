
#include <iostream>
#include <vector>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "../../common/stb_image_write.h"
#include "../../common/stb_image.h"
#include "../../common/ppu_manager.h"
#include "../../common/benchmark.h"
#include "../structures.h"

void read_image(image_task& task, std::string fn)
{
  memset((void*)&task, 0, sizeof(image_task));
  int w, h, n;
  unsigned char * data = stbi_load(fn.c_str(), &w, &h, &n, 0);
  task.size.w = w;
  task.size.h = h;
  task.bytes = (void*)data;
}

int main(int argc, char * argv[])
{
	ppu_manager ppu_manager(true);
	benchmark track("coursework");

	const int spe_count = ppu_manager.spe_count();
	std::vector<std::pair<std::string, int> > programs;
	programs.push_back(std::pair<std::string, int>("./spu/spu", spe_count));
  //programs.push_back(std::pair<std::string, int>("./spu/spu", spe_count));

  const int processes = programs.size();
  image_task task __attribute__((aligned(16)));
  read_image(task, "../assets/picture.bmp"); 
  task.sections = spe_count;

  for(int i = 0; i < processes; ++i)
	{ 
    ppu_manager.spe_arg((void*)&task, sizeof(image_task));
		ppu_manager.spe_program(programs[i].first);
	 	ppu_manager.spe_run(programs[i].second);
	}

  stbi_write_bmp("../assets/grayscale.bmp", task.size.w, task.size.h, 3, task.bytes);	
	return 0;
}
