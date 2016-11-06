
#include <iostream>
#include <vector>
#include <string>

#include "../../common/spu_manager.h"
#include "../../common/benchmark.h"
#include "../../common/image.h"
#include "../../common/log.h"
#include "../structures.h"

const int IMAGES = 10;

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
  unsigned char original[image.width * image.height];
  unsigned char output[image.width * image.height];
  memset(output, 255, sizeof(output));
  for(int i = 0; i < sizeof(original); i++)
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

  std::vector<std::string> programs;
  programs.push_back("./blur/blur");
  programs.push_back("./sobel/sobel");
  programs.push_back("./detection/detection");
  programs.push_back("./overlay/overlay");
  
  for(int i = 0; i < programs.size(); i++)
  {
    LOG("%s %s", "Running -> ", programs[i].c_str());

    spu_manager.spe_arg((void*)&task, sizeof(image_task));
    spu_manager.spe_program(programs[i]);
	  spu_manager.spe_run(task.sections);  
  }

  LOG("%s %s", "Writing output -> ", filename("./O", index+1, ".bmp").c_str());

  write_output(filename("./O", index+1, ".bmp"), output, image.width, image.height);
}

int main(int argc, char * argv[])
{
	spu_manager spu_manager(true);
	benchmark track("coursework");

  std::vector<std::string> filenames;
  std::vector<basic_image> images;
  
  for(int i = 0; i < IMAGES; i++)
  {
    filenames.push_back(filename("../assets/", i+1, ".bmp"));
  }

  load_images(filenames, images);

  LOG("%s %d %s", "Loaded & Processing", IMAGES, "images");

  for(int i = 0; i < IMAGES; i++)
  { 
    process_image(spu_manager, images[i], i);
  }

  unload_images(images);
  return 0;
}
