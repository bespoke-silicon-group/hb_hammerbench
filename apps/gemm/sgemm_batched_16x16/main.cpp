#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <random>
#include <limits>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>
#include <bsg_manycore_eva.h>

#define ALLOC_NAME "default_allocator"


int kernel_matrix_mul (int argc, char **argv)
{
      int rc;
      char *bin_path, *test_name;
      struct arguments_path args = {NULL, NULL};

      argp_parse (&argp_path, argc, argv, 0, 0, &args);
      bin_path = args.path;
      test_name = args.name;

      // Define path to binary.
      // Initialize device, load binary and unfreeze tiles.
      hb_mc_device_t device;
      BSG_CUDA_CALL(hb_mc_device_init(&device, test_name, 0));


      hb_mc_pod_id_t pod;
      hb_mc_device_foreach_pod_id(&device, pod)
      {

        BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
        BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

        //************************************************************
        // Define tg_dim_x/y: number of tiles in each tile group
        // Calculate grid_dim_x/y: number of tile groups needed based on block_size_x/y
        //************************************************************
        hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y };
        hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};


        // Allocate memory on host;
        float *host_mat1;
        float *host_mat2;
        float *host_result;
        host_mat1 = (float*) malloc(BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*sizeof(float));
        host_mat2 = (float*) malloc(BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*sizeof(float));
        host_result = (float*) malloc(BLOCK_DIM_X*2*BLOCK_DIM_Y*BATCH*sizeof(float));
      
        for (int i = 0; i < BLOCK_DIM_X*BLOCK_DIM_Y*BATCH; i++) {
            host_mat1[i] = 0.0f;
            host_mat2[i] = 0.0f;
        }


        // allocate memory on device
        // Make it pod-cache aligned;
        #define CACHE_LINE_WORDS 16
        eva_t temp_device1, temp_device2;
        BSG_CUDA_CALL(hb_mc_device_malloc(&device, CACHE_LINE_WORDS*sizeof(int), &temp_device1));
        printf("temp Addr: %x\n", temp_device1);
        int align_size = (32)-1-((temp_device1>>2)%(CACHE_LINE_WORDS*32)/CACHE_LINE_WORDS);
        BSG_CUDA_CALL(hb_mc_device_malloc(&device, align_size*sizeof(int)*CACHE_LINE_WORDS, &temp_device2));

        eva_t device_mat1, device_mat2, device_result;
        BSG_CUDA_CALL(hb_mc_device_malloc(&device, BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*sizeof(float), &device_mat1));
        BSG_CUDA_CALL(hb_mc_device_malloc(&device, BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*sizeof(float), &device_mat2));
        BSG_CUDA_CALL(hb_mc_device_malloc(&device, BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*2*sizeof(float), &device_result));
        printf("device_mat1   = %x\n", device_mat1);
        printf("device_mat2   = %x\n", device_mat2);
        printf("device_result = %x\n", device_result);

        // Transfer mat1, mat2
        hb_mc_dma_htod_t htod_job[] = {
          {
            .d_addr = device_mat1,
            .h_addr = (void *) host_mat1,
            .size = BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*sizeof(float)
          },
          {
            .d_addr = device_mat2,
            .h_addr = (void *) host_mat2,
            .size = BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*sizeof(float)
          }
        };

        BSG_CUDA_CALL(hb_mc_device_dma_to_device(&device, htod_job, 2));


        // cuda arguments
        #define CUDA_ARGC 3
        uint32_t cuda_argv[CUDA_ARGC] = {device_mat1, device_mat2, device_result};

        // Enqueue kernel
        bsg_pr_info("Enqueue Kernel\n");
        BSG_CUDA_CALL(hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel_mm_opt", CUDA_ARGC, cuda_argv));

        // Execute kernel.        
        bsg_pr_info("Execute Kernel\n");
        BSG_CUDA_CALL(hb_mc_device_tile_groups_execute(&device));


        // copy result.
        bsg_pr_info("Copying result back\n");
        hb_mc_dma_dtoh_t dtoh_job [] = {
          {
            .d_addr = device_result,
            .h_addr = (void *) &host_result[0],
            .size = 2*BLOCK_DIM_X*BLOCK_DIM_Y*BATCH * sizeof(float)
          }
        };

        BSG_CUDA_CALL(hb_mc_device_dma_to_host(&device, dtoh_job, 1));

        // validate
        

        //************************************************************
        // Freeze the tiles and memory manager cleanup.
        //************************************************************
        BSG_CUDA_CALL(hb_mc_device_finish(&device));

      }

      return HB_MC_SUCCESS;
}

#ifdef VCS
int vcs_main(int argc, char ** argv) {
#else
        int main(int argc, char ** argv) {
#endif
                bsg_pr_test_info("test_matrix_mul Regression Test\n");
                int rc = kernel_matrix_mul(argc, argv);
                bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
                return rc;
        }


