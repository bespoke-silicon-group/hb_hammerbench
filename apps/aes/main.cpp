#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <cstdlib>
#include <time.h>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <bsg_manycore_regression.h>
#include <bsg_manycore.h>
#include <cstdint>
#include <vector>
#include "aes.hpp"

#include <chrono>
#include <iostream>

#define ALLOC_NAME "default_allocator"
#define NUM_TILES (bsg_tiles_X*bsg_tiles_Y)
#define MSG_LEN 1024

int aes_multipod(int argc, char **argv)
{
  int r = 0;
  
  // command line;
  const char *bin_path = argv[1];

  // parameters;
  const int num_iter = NUM_ITER;
  const int msg_len = MSG_LEN;
  bsg_pr_info("num_iter=%d\n", num_iter);  // # of iter per tile;
  bsg_pr_info("msg_len=%d\n", msg_len);    // in bytes;


  // Host memory;
  //uint8_t buf[NUM_TILES*NUM_ITER*MSG_LEN];
  uint8_t* buf = (uint8_t*) malloc(NUM_TILES*NUM_ITER*MSG_LEN*sizeof(uint8_t));
  //struct AES_ctx ctx[NUM_TILES*NUM_ITER];
  struct AES_ctx* ctx = (struct AES_ctx*) malloc(NUM_TILES*NUM_ITER*sizeof(struct AES_ctx));

  for (int i = 0; i < NUM_TILES*NUM_ITER; i++) {
    for (int j = 0; j < MSG_LEN; j++) {
      buf[(MSG_LEN*i)+j] = (uint8_t) j;
    }
  }
  
  memset(ctx, 0, NUM_TILES*NUM_ITER*sizeof(struct AES_ctx));
  uint8_t key[AES_KEYLEN] = {0, 1, 2, 3, 4, 5, 6, 7,
                             8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
  for (int i = 0; i < NUM_TILES*NUM_ITER; i++) {
    AES_init_ctx(&ctx[i], key);
  }


  // Host calculation;
  uint8_t expected_buf[MSG_LEN];
  struct AES_ctx expected_ctx;

  for (int j = 0; j < MSG_LEN; j++) {
    expected_buf[j] = (uint8_t) j;
  }
  memset(&expected_ctx, 0, sizeof(expected_ctx));
  AES_init_ctx(&expected_ctx, key);
  
  AES_CBC_encrypt_buffer(&expected_ctx, expected_buf, MSG_LEN);



  // Initialize device;
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "aes_multipod", HB_MC_DEVICE_ID));

  #define NUM_PODS 8
  eva_t d_ctx[NUM_PODS];
  eva_t d_buf[NUM_PODS];

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {

//if (pod % 2 == 1) continue;

    bsg_pr_info("Loading program for pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));
  
    // Allocate memory on device;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NUM_TILES*NUM_ITER*sizeof(struct AES_ctx), &d_ctx[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NUM_TILES*NUM_ITER*MSG_LEN*sizeof(uint8_t), &d_buf[pod]));

    // DMA transfer;
    bsg_pr_info("Transferring data: pod %d\n", pod);
    std::vector<hb_mc_dma_htod_t> htod_job;
    htod_job.push_back({d_ctx[pod], &ctx[0], NUM_TILES*NUM_ITER*sizeof(struct AES_ctx)});
    htod_job.push_back({d_buf[pod], &buf[0], NUM_TILES*NUM_ITER*MSG_LEN*sizeof(uint8_t)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&device, htod_job.data(), htod_job.size()));


    // Cuda args;
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 5
    uint32_t cuda_argv[CUDA_ARGC] = {d_ctx[pod], d_buf[pod], MSG_LEN, NUM_ITER, pod};

    // Enqueue kernel;
    bsg_pr_info("Enqueue Kernel: pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }


  // Launch pod;
  bsg_pr_info("Launching all pods\n");



  //BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&device));

        hb_mc_pod_id_t podv[(&device)->num_pods];
        //hb_mc_pod_id_t pod;
        hb_mc_device_foreach_pod_id(&device, pod)
        {
                podv[pod]=pod;
        }

        //hb_mc_device_podv_kernels_execute(&device, podv, &device->num_pods);
        int podc = (&device)->num_pods;

        /* launch as many tile groups as possible on all pods */
        //BSG_CUDA_CALL(hb_mc_device_podv_try_launch_tile_groups(&device, podv, podc));



        for (int podi = 0; podi < podc; podi++)
        {
                hb_mc_pod_t *pod = &(&device)->pods[podv[podi]];
                int r;
                hb_mc_tile_group_t *tg;
                hb_mc_dimension_t last_failed = hb_mc_dimension(0,0);
                // scan for ready tile groups
                for (tg = pod->tile_groups; tg != pod->tile_groups+pod->num_tile_groups; tg++)
                {
                        // only look at ready tile groups
                        if (tg->status != HB_MC_TILE_GROUP_STATUS_INITIALIZED)
                                continue;

                        // skip if we know this shape fails
                        if (last_failed.x == tg->dim.x &&
                            last_failed.y == tg->dim.y)
                                continue;

                        // keep going if we can't allocate
                        r = hb_mc_device_pod_tile_group_allocate_tiles(&device, pod, tg);
                        if (r != HB_MC_SUCCESS) {
                                // mark this shape as the last failed
                                last_failed = tg->dim;
                                continue;
                        }

                        // launch the tile tile group
                        BSG_CUDA_CALL(hb_mc_device_pod_tile_group_launch_first(&device, pod, tg));
                }
        }



  auto start = std::chrono::high_resolution_clock::now();



        // try launching as many tile groups as possible on all pods
        for (int podi = 0; podi < podc; podi++)
        {
                hb_mc_pod_t *pod = &(&device)->pods[podv[podi]];
                hb_mc_tile_group_t *tg;
                // scan for ready tile groups
                for (tg = pod->tile_groups; tg != pod->tile_groups+pod->num_tile_groups; tg++)
                {
                        // launch the tile tile group
                        BSG_CUDA_CALL(hb_mc_device_pod_tile_group_launch_last(&device, pod, tg));
                }
        }



        /* until all tile groups have completed */
        while (hb_mc_device_podv_all_tile_groups_finished(&device, podv, podc) != HB_MC_SUCCESS)
        {
                /* wait for any tile group to finish on any pod */
                hb_mc_pod_id_t pod;
                BSG_CUDA_CALL(hb_mc_device_podv_wait_for_tile_group_finish_any(&device, podv, podc,
                                                                               &pod));

                /* try launching launching tile groups on pod with most recent completion */
                //BSG_CUDA_CALL(hb_mc_device_pod_try_launch_tile_groups(&device, &(&device)->pods[pod]));
        }



    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time: " << duration.count() << " us" << std::endl; 


  // Read from devices;
  //uint8_t actual_buf[NUM_TILES*NUM_ITER*MSG_LEN];
  uint8_t* actual_buf = (uint8_t*) malloc(NUM_TILES*NUM_ITER*MSG_LEN*sizeof(uint8_t));

  bool fail = false;
  hb_mc_device_foreach_pod_id(&device, pod) {

//if (pod % 2 == 1) continue;

    bsg_pr_info("Reading results: pods %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));

    // clear buf;
    for (int i = 0; i < NUM_TILES*NUM_ITER*MSG_LEN; i++) {
      actual_buf[i] = (uint8_t) 0;
    }   

    // DMA transfer: device -> host;
    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    dtoh_job.push_back({d_buf[pod], &actual_buf[0], NUM_TILES*NUM_ITER*MSG_LEN*sizeof(uint8_t)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job.data(), dtoh_job.size()));


    // Validate;
    for (int i = 0; i < NUM_TILES*NUM_ITER; i++) {
      for (int j = 0; j < MSG_LEN; j++) {
        uint8_t actual = actual_buf[(MSG_LEN*i)+j];
        uint8_t expected = expected_buf[j];
        if (actual != expected) {
          bsg_pr_info("Mismatch: i=%d, j=%d, actual=%d, expected=%d\n", i, j, actual, expected);
          fail = true;
        }
      }
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

declare_program_main("aes_multipod", aes_multipod);

