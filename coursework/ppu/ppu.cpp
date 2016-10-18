
#include <iostream>
#include <vector>
#include <string>

#include "../../common/spu_manager.h"
#include "../../common/benchmark.h"
#include "../../common/image.h"
#include "../../common/log.h"
#include "../structures.h"

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
  unsigned char output[image.width * image.height * 3];

  image_task task __attribute__((aligned(16)));
  task.output = (unsigned long long)output;
  task.input = (unsigned long long)image.data;
  task.components = 3;
  task.sections = 6;
  task.size.h = image.height;
  task.size.w = image.width;

  LOG("%s %s", "Running -> ", "./spu/spu");
  
  spu_manager.spe_arg((void*)&task, sizeof(image_task));
	spu_manager.spe_program("./spu/spu");
	spu_manager.spe_run(6);

  LOG("%s %s", "Writing output -> ", filename("./O", index+1, ".bmp").c_str());

  write_output(filename("./O", index+1, ".bmp"), output, image.width, image.height);
}

int main(int argc, char * argv[])
{
	spu_manager spu_manager(true);
	benchmark track("coursework");

  std::vector<std::string> filenames;
  std::vector<basic_image> images;
  
  for(int i = 0; i < 10; i++)
    filenames.push_back(filename("../assets/", i+1, ".bmp"));

  load_images(filenames, images);
  const int imagesCount = images.size();
  LOG("%s %d %s", "Loaded", imagesCount, "images");

  for(int i = 0; i < imagesCount; i++) 
    process_image(spu_manager, images[i], i);

  unload_images(images);
  return 0;
}
