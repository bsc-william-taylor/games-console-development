
#include <stdio.h>
#include <spu_mfcio.h>

typedef unsigned long long ulong64;

int data[1000] __attribute__((aligned(16)));
int tagID = 0;

int main(ulong64 speID, ulong64 argp, ulong64 envp)
{
  printf("SpuID:%llu Argp:%llu Envp:%llu \n", speID, argp, envp);

  if(argp != 0)
  {
    mfc_get(data, argp, sizeof(data), tagID, 0, 0);
    mfc_write_tag_mask(1<<tagID);
    mfc_read_tag_status_any();

    for(int i = 0; i < 10; i++)
    {
      printf("%d,", data[i]);
    }

    
  }

  return 0;
}
