#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <bsg_manycore_regression.h>

#define ALLOC_NAME "default_allocator"

#ifndef SIZE
Please define SIZE in Makefile.
#endif

int compare(const void *a, const void *b) {
  uint32_t x = *(uint32_t*)a;
  uint32_t y = *(uint32_t*)b;
  return (x > y) - (x < y);
}

int kernel_radix_sort(int argc, char **argv) {
  int rc;
  char *bin_path, *test_name;
  struct arguments_path args = {NULL, NULL};
  
  argp_parse(&argp_path, argc, argv, 0, 0, &args);
  bin_path = args.path;
  test_name = args.name;

  srand(42);
 
  // Initialize Device.
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, test_name, HB_MC_DEVICE_ID));

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    bsg_pr_info("Loading program for pod %d\n.", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // Allocate a block of memory in host.
    uint32_t * A_host = (uint32_t*) malloc(sizeof(uint32_t)*SIZE);
    // uint32_t * A_expected_host = (uint32_t*) malloc(sizeof(uint32_t)*SIZE);

    // initialize with some numbers;
    for (int i = 0; i < SIZE; i++) {
      A_host[i] = (uint32_t) (rand());
      // A_expected_host[i] = A_host[i];
    }
    // qsort(A_expected_host, SIZE, sizeof(uint32_t), compare);

    // Make it pod-cache aligned
#define POD_CACHE_ALIGNED
#ifdef POD_CACHE_ALIGNED
    eva_t temp_device1, temp_device2;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, CACHE_LINE_WORDS*sizeof(int), &temp_device1));
    printf("temp Addr: %x\n", temp_device1);
    int align_size = (32)-1-((temp_device1>>2)%(CACHE_LINE_WORDS*32)/CACHE_LINE_WORDS);
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, align_size*sizeof(int)*CACHE_LINE_WORDS, &temp_device2));
#endif

    // Allocate a block of memory in device.
    eva_t A_device;
    eva_t A_buffer;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, SIZE * sizeof(uint32_t), &A_device));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, SIZE * sizeof(uint32_t), &A_buffer));

    printf("A_device Addr: %x\n", A_device);
 
    // DMA Transfer to device.
    hb_mc_dma_htod_t htod_job [] = {
      {
        .d_addr = A_device,
        .h_addr = (void *) &A_host[0],
        .size = SIZE * sizeof(uint32_t)
      }
    };

    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&device, htod_job, 1));

    if (DEBUG_PRINT) {
      printf("\n=== INPUT ===\n");
      for (int i = 0; i < SIZE; i++) {
        printf("  A[%d] = %u\n", i, A_host[i]);
      }
    }
    // CUDA arguments
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 3
    uint32_t cuda_argv[CUDA_ARGC] = {A_device, A_buffer, SIZE};
    
    // Enqueue Kernel.
    BSG_CUDA_CALL(hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel_radix_sort", CUDA_ARGC, cuda_argv));
    
    // Launch kernel.
    hb_mc_manycore_trace_enable((&device)->mc);
    BSG_CUDA_CALL(hb_mc_device_tile_groups_execute(&device));
    hb_mc_manycore_trace_disable((&device)->mc);


    // Copy result and validate.
    hb_mc_dma_dtoh_t dtoh_job [] = {
      {
        .d_addr = A_device,
        .h_addr = (void *) &A_host[0],
        .size = SIZE * sizeof(uint32_t)
      }
    };

    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job, 1));

    if (DEBUG_PRINT) {
      printf("\n=== OUTPUT ===\n");
      for (int i = 0; i < SIZE; i++) {
        printf("  A[%d] = %u\n", i, A_host[i]);
      }
    }
    // Freeze tiles.
    BSG_CUDA_CALL(hb_mc_device_program_finish(&device));
  }

  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  return HB_MC_SUCCESS; 
}


declare_program_main("radix_sort", kernel_radix_sort);
