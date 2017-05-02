
#include "spu_manager.h"
#include "log.h"

#include <algorithm>
#include <libspe2.h>
#include <stdio.h>
#include <stdlib.h>

enum QueryType { AllAvailable = -1, Current = 0 };

void* ppu_pthread_function(void* arg)
{
    spu_pthread_data* data = (spu_pthread_data*)arg;
    unsigned int entry = SPE_DEFAULT_ENTRY;

    int err = spe_context_run(data->speid, &entry, 0, data->argp, (void*)data->envp, NULL);

    if(err < 0)
    {
        perror("spe_context_run");
        exit(1);
    }

    pthread_exit(NULL);
}

spu_manager::spu_manager(bool debug)
: spu_arg_address(NULL)
{
    physical_cpu_nodes = spe_cpu_info_get(SPE_COUNT_PHYSICAL_CPU_NODES, AllAvailable);
    physical_spes = spe_cpu_info_get(SPE_COUNT_PHYSICAL_SPES, AllAvailable);
    usable_spes = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, AllAvailable);

    if(debug)
    {
        info();
    }
}

spu_manager::~spu_manager()
{
}

void spu_manager::info()
{
    LOG("%s %d", "Physical SPE's ", physical_spes);
    LOG("%s %d", "Usable SPE's ", usable_spes);
    LOG("%s %d", "CPU Nodes ", physical_cpu_nodes);
}

void spu_manager::spe_program(const std::string& filename)
{
    spu_program = filename;
}

void spu_manager::spe_arg(void * address, int size)
{
    spu_arg_address = address;
    spu_arg_sizeof = size;
}

void spu_manager::spe_run(int count)
{
    spe_program_handle_t * image = spe_image_open(spu_program.c_str());

    if(image == NULL)
    {
        perror("spe_run, spe_image_open, failed");
        return;
    }

    const int processes = std::min(count, usable_spes);
    std::vector<spu_pthread_data> data(processes);
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

        data[i].envp = spu_arg_sizeof;
        data[i].argp = spu_arg_address;
        data[i].speid = context;

        int perr = pthread_create(&data[i].pthread, NULL, &ppu_pthread_function, &data[i]);

        if(perr != 0)
        {
            perror("spe_run, pthread_create");
            return;
        }

        spe_in_mbox_write(context, (unsigned int*)&i, 1, SPE_MBOX_ANY_NONBLOCKING);
    }

    for(int i = 0; i < processes; ++i)
    {
        pthread_join(data[i].pthread, NULL);
        spe_context_destroy(data[i].speid);
    }

    spe_image_close(image);

}

int spu_manager::spe_count()
{
    return usable_spes;
}
