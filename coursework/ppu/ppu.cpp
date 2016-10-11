
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
  int w, h, n;
  unsigned char * data = stbi_load(fn.c_str(), &w, &h, &n, 0);
  task.size.w = w;
  task.size.h = h;
  task.bytes = (unsigned char*)data;
  task.output = 0;
  task.components = n;
 
  printf("Image dimensions: (w, h, c) = %d, %d, %d \n", w, h, n);
}

int main(int argc, char * argv[])
{
	ppu_manager ppu_manager(true);
	benchmark track("coursework");

	const int spe_count = 4;//ppu_manager.spe_count();
	std::vector<std::pair<std::string, int> > programs;
	programs.push_back(std::pair<std::string, int>("./spu/spu", spe_count));

  const int processes = programs.size(); 
  image_task task __attribute__((aligned(16)));
  read_image(task, "../assets/picture.bmp"); 
  
  unsigned char buffer[task.size.w*task.size.h*task.components];
  memset(buffer, 20, sizeof(buffer));
  task.sections = spe_count;
  task.output = (unsigned long long)buffer;

  //printf("before: %llu %d \n", task.output, (int)buffer[0]);

  for(int i = 0; i < processes; ++i)
	{ 
    ppu_manager.spe_arg((void*)&task, sizeof(image_task));
		ppu_manager.spe_program(programs[i].first);
	 	ppu_manager.spe_run(programs[i].second);
  }

  stbi_write_bmp("../assets/white.bmp", task.size.w, task.size.h, task.components, (void*)task.output);	

  printf("after: %d \n", (int)buffer[0]);
  return 0;
}
