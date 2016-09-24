
#include "ppu_manager.h"
#include <algorithm>
#include <libspe2.h>
#include <stdio.h>
#include <stdlib.h>

enum QueryType { AllAvailable = -1, Current = 0 };

void* ppu_pthread_function(void* arg) 
{
	ppu_pthread_data* data = (ppu_pthread_data*)arg;
	unsigned int entry = SPE_DEFAULT_ENTRY;
	int err = spe_context_run(data->speid, &entry, 0, data->argp, NULL, NULL);

	if(err < 0)
	{
		perror("spe_context_run");
		exit(1);
	}

	pthread_exit(NULL);
}

ppu_manager::ppu_manager(bool debug)
{
	physical_cpu_nodes = spe_cpu_info_get(SPE_COUNT_PHYSICAL_CPU_NODES, AllAvailable);
	physical_spes = spe_cpu_info_get(SPE_COUNT_PHYSICAL_SPES, AllAvailable);
	
	usable_spes = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, AllAvailable);
	
	if(debug)
	{
		info();
	}
}

ppu_manager::~ppu_manager()
{
}

void ppu_manager::info()
{
	printf("SPE's (Physical/Usable): %d:%d \n", physical_spes, usable_spes);
	printf("CPU physical nodes: %d \n\n", physical_cpu_nodes);	
}

void ppu_manager::spe_program(const std::string& filename)
{
	spu_program = filename;
}

void ppu_manager::spe_run(int count)
{
	spe_program_handle_t * image = spe_image_open(spu_program.c_str());

	if(image == NULL)
	{
		perror("spe_run, spe_image_open, failed");
		return;
	}
	
	const int processes = std::min(count, usable_spes);
	std::vector<ppu_pthread_data> data(processes);
	for(int i = 0; i < processes; ++i)
	{
		spe_context_ptr_t context = spe_context_create(0, NULL);
		
		if(context == NULL)
		{
			perror("spe_run, spe_context_create, failed");
			return;
		}

		
		int err = spe_program_load(context, image);

		if(err != 0)
		{
			perror("spe_run, spe_context_create, failed");
			return;
		}
	
		data[i].argp = (void*)NULL;
		data[i].speid = context;
		
		int perr = pthread_create(&data[i].pthread, NULL, &ppu_pthread_function, &data[i]);

		if(perr != 0)
		{
			perror("spe_run, pthread_create");
			return;
		}
	}

	for(int i = 0; i < processes; ++i)
	{
		pthread_join(data[i].pthread, NULL);
		spe_context_destroy(data[i].speid);
	}

	spe_image_close(image);
}

int ppu_manager::spe_count()
{
	return usable_spes;
}
