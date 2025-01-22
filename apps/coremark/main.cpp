#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <random>
#include <limits>
#include <chrono>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>
#include <bsg_manycore_eva.h>

#define ALLOC_NAME "default_allocator"


// Host Main;
int coremark_multipod(int argc, char **argv)
{
  int r = 0;
  
  // Command line arg;
  const char *bin_path = argv[1];

  // parameters;
  bsg_pr_test_info("NITER=%d\n", NITER);


  // Pod;
  // Initialize devices;
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "coremark_multipod", HB_MC_DEVICE_ID));
  eva_t d_crc;

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    bsg_pr_test_info("Loading program for pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // crc results;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, bsg_tiles_X*bsg_tiles_Y*3*sizeof(uint16_t), &d_crc));

    // cuda arguments
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y };
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 2
    uint32_t cuda_argv[CUDA_ARGC] = {d_crc, NITER};

    // Enqueue kernel
    bsg_pr_test_info("Enqueue Kernel: pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }

  // Launch pods;
  bsg_pr_test_info("Launching all pods\n");
  auto start = std::chrono::high_resolution_clock::now();
  BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&device));
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  bsg_pr_test_info("Execution time: %f\n seconds", elapsed.count());

  
  // Validate;
  bool fail = false;
  uint16_t host_crc[bsg_tiles_X*bsg_tiles_Y*3];

  hb_mc_device_foreach_pod_id(&device, pod) {
    bsg_pr_test_info("Reading results: pods %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    
    // DMA transfer: device -> host;
    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    dtoh_job.push_back({d_crc, host_crc, bsg_tiles_X*bsg_tiles_Y*3*sizeof(uint16_t)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job.data(), dtoh_job.size()));

    // validate;
    for (int i = 0; i < bsg_tiles_X*bsg_tiles_Y; i++) {
      if (host_crc[(3*i)+0] != 0xe714) fail = true;
      if (host_crc[(3*i)+1] != 0x1fd7) fail = true;
      if (host_crc[(3*i)+2] != 0x8e3a) fail = true;
    }   
  }


  // FINISH;
  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  if (fail) {
    return HB_MC_FAIL;
  } else {
    return HB_MC_SUCCESS;
  }
}

declare_program_main("coremark_multipod", coremark_multipod);
