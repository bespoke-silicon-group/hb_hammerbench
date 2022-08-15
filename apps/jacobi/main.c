#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <bsg_manycore_regression.h>

#define ALLOC_NAME "default_allocator"
#define Index3D(nx,ny,nz,x,y,z) (((((y)*nx)+(x))*nz)+(z))


void host_jacobi(int c0, int c1, float *A0, float * Anext,
                 const int nx, const int ny, const int nz)
{
  printf("host jacobi: nx=%d, ny=%d, nz=%d\n", nx, ny, nz);
  float c0f = (float) c0;
  float c1f = (float) c1;
	for(int x=0; x<nx; x++) {
		for(int y=0; y<ny; y++) {
			for(int z=1; z<nz-1; z++) {

        float self = A0[Index3D(nx,ny,nz,x,y,z)];
        float xp1, xm1, yp1, ym1, zp1, zm1;
        if (x > 0)    { xm1 = A0[Index3D(nx,ny,nz,x-1,y,z)]; } else {xm1= 0.0f;}
        if (x < (nx-1)) { xp1= A0[Index3D(nx,ny,nz,x+1,y,z)]; } else {xp1= 0.0f;}
        if (y > 0)    { ym1= A0[Index3D(nx,ny,nz,x,y-1,z)]; } else {ym1= 0.0f;}
        if (y < (ny-1)) { yp1= A0[Index3D(nx,ny,nz,x,y+1,z)]; } else {yp1= 0.0f;}
        zm1= A0[Index3D(nx,ny,nz,x,y,z-1)];
        zp1= A0[Index3D(nx,ny,nz,x,y,z+1)];

        float neighbor = xm1+xp1+ym1+yp1+zm1+zp1;
        float jacobi =  (c1f*neighbor) - (c0f*self);
				Anext[Index3D(nx,ny,nz,x,y,z)] = jacobi;

        //printf("x=%d,y=%d,z=%d,xp1=%f,xm1=%f,yp1=%f,ym1=%f,zm1=%f,zp1=%f\n",x,y,z,xp1,xm1,yp1,ym1,zm1,zp1);
        //printf("x=%d,y=%d,z=%d,neighbor=%f,self=%f,Anext=%f\n",x,y,z,neighbor,self,jacobi);
			}
		}
	}

}


int kernel_jacobi (int argc, char **argv) {
  int rc;
  char *bin_path, *test_name;
  struct arguments_path args = {NULL, NULL};

  argp_parse (&argp_path, argc, argv, 0, 0, &args);
  bin_path = args.path;
  test_name = args.name;

  bsg_pr_test_info("Running the CUDA Jacobi Kernel on %dx%d tile groups.\n\n", TILE_GROUP_DIM_Y, TILE_GROUP_DIM_X);

  srand(time);

  // Initialize Device
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, test_name, 0));

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    bsg_pr_info("Loading program for test %s onto pod %d\n", test_name, pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // JACOBI:
    // input 16 x 8 x 512 x Nw
    int32_t Nx = 16;
    int32_t Ny = 8;
    int32_t Nz = 512+2; // adding pad of 2 to intentionally disalign the cache lines.
    int32_t Nw = 1;
    int32_t N  = Nx * Ny * Nz * Nw;

    eva_t A0_device, Anext_device;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, N * sizeof(float), &A0_device));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, N * sizeof(float), &Anext_device));

    int c0 = 2;
    int c1 = 4;

    float A0_host[N];
    float Anext_host[N];
    for (int i = 0; i < N; i++) {
      A0_host[i] = (float)(i % 32);
      //A0_host[i] = 1.0f;
      Anext_host[i] = 0.0f;
    }

    float Anext_expected[N];
    for (int i = 0; i < N; i++) { // fill with 0
      Anext_expected[i] = 0.0f;
    }
    for (int w = 0; w < Nw; w++) {
      int start_idx = Nx*Ny*Nz*w;
      host_jacobi(c0, c1, &A0_host[start_idx], &Anext_expected[start_idx], Nx, Ny, Nz);
    }

    /*****************************************************************************************************************
    * Copy A0 from host onto device DRAM.
    ******************************************************************************************************************/
    hb_mc_dma_htod_t htod_A_job [2] = {
      {
        .d_addr = A0_device,
        .h_addr = (void *) &A0_host[0],
        .size   = N * sizeof(float)
      },
      {
        .d_addr = Anext_device,
        .h_addr = (void *) &Anext_host[0],
        .size   = N * sizeof(float)
      }
    };

    BSG_CUDA_CALL(hb_mc_device_dma_to_device(&device, htod_A_job, 2));
    //BSG_CUDA_CALL(hb_mc_device_memcpy (&device, (void *) ((intptr_t) A0_device), (void *) &A0_host[0], N * sizeof(float), HB_MC_MEMCPY_TO_DEVICE));
    //BSG_CUDA_CALL(hb_mc_device_memcpy (&device, (void *) ((intptr_t) Anext_device), (void *) &Anext_host[0], N * sizeof(float), HB_MC_MEMCPY_TO_DEVICE));

    /*****************************************************************************************************************
    * Define block_size_x/y: amount of work for each tile group
    * Define tg_dim_x/y: number of tiles in each tile group
    * Calculate grid_dim_x/y: number of tile groups needed based on block_size_x/y
    ******************************************************************************************************************/
    hb_mc_dimension_t tg_dim = { .x = 16, .y = 8};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};

    /*****************************************************************************************************************
    * Prepare list of input arguments for kernel.
    ******************************************************************************************************************/
    #define CUDA_ARGC 8
    int cuda_argv[CUDA_ARGC] = {c0, c1, A0_device, Anext_device, Nx, Ny, Nz, Nw};

    // Enqueue kernel.
    BSG_CUDA_CALL(hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel_jacobi", CUDA_ARGC, cuda_argv));

    // Launch kernel.
    hb_mc_manycore_trace_enable((&device)->mc);
    BSG_CUDA_CALL(hb_mc_device_tile_groups_execute(&device));
    hb_mc_manycore_trace_disable((&device)->mc);

    // Bring back Anext to host.
    hb_mc_dma_dtoh_t dtoh_job [] = {
      {
        .d_addr = Anext_device,
        .h_addr = (void*) &Anext_host[0],
        .size = N * sizeof(float)
      }
    };
    BSG_CUDA_CALL(hb_mc_device_dma_to_host(&device, &dtoh_job, 1));

    // Finish
    BSG_CUDA_CALL(hb_mc_device_program_finish(&device));

    // Validate
    bsg_pr_test_info("end of test ... checking ... \n\n");
    int mismatch = 0;
    for (int i = 0; i < N; i++) {
      if (Anext_host[i] != Anext_expected[i]) {
        int x = (i % (Nx*Nz)) / Nz;
        int y = i / (Nx*Nz);
        int z = i % Nz;
        //printf("idx %d (%d,%d,%d) computed = %f -- expecting = %f\n", i, x,y,z, Anext_host[i], Anext_expected[i]);
        mismatch = 1;
      }
    }

    if (mismatch) {
      return HB_MC_FAIL;
    }
  }

  BSG_CUDA_CALL(hb_mc_device_finish(&device));

  return HB_MC_SUCCESS;
}

declare_program_main("test_jacobi", kernel_jacobi);
