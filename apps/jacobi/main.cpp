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
#include <vector>
#include <bsg_manycore_regression.h>

#include <chrono>
#include <iostream>

#define ALLOC_NAME "default_allocator"
#define Index3D(nx,ny,nz,x,y,z) (((((y)*nx)+(x))*nz)+(z))


void host_jacobi(int c0, int c1, float *A0, float * Anext,
                 const int nx, const int ny, const int nz)
{
  bsg_pr_info("host jacobi: nx=%d, ny=%d, nz=%d\n", nx, ny, nz);
  float c0f = (float) c0;
  float c1f = (float) c1;
	for(int x=0; x<nx; x++) {
		for(int y=0; y<ny; y++) {
			for(int z=0; z<nz; z++) {

        float self = A0[Index3D(nx,ny,nz,x,y,z)];
        float xp1, xm1, yp1, ym1, zp1, zm1;
        if (x > 0)    { xm1 = A0[Index3D(nx,ny,nz,x-1,y,z)]; } else {xm1= 0.0f;}
        if (x < (nx-1)) { xp1= A0[Index3D(nx,ny,nz,x+1,y,z)]; } else {xp1= 0.0f;}
        if (y > 0)    { ym1= A0[Index3D(nx,ny,nz,x,y-1,z)]; } else {ym1= 0.0f;}
        if (y < (ny-1)) { yp1= A0[Index3D(nx,ny,nz,x,y+1,z)]; } else {yp1= 0.0f;}
        if (z > 0)    { zm1= A0[Index3D(nx,ny,nz,x,y,z-1)]; } else {zm1= 0.0f;}
        if (z < (nz-1)) { zp1= A0[Index3D(nx,ny,nz,x,y,z+1)]; } else {zp1= 0.0f;}

        float neighbor = xm1+xp1+ym1+yp1+zm1+zp1;
        float jacobi =  (c1f*neighbor) - (c0f*self);
				Anext[Index3D(nx,ny,nz,x,y,z)] = jacobi;

        //printf("x=%d,y=%d,z=%d,xp1=%f,xm1=%f,yp1=%f,ym1=%f,zm1=%f,zp1=%f\n",x,y,z,xp1,xm1,yp1,ym1,zm1,zp1);
        //printf("x=%d,y=%d,z=%d,neighbor=%f,self=%f,Anext=%f\n",x,y,z,neighbor,self,jacobi);
			}
		}
	}
}


// Host main;
int jacobi_multipod (int argc, char **argv)
{
  int r = 0;

  // Command Line arg;
  const char *bin_path = argv[1];

  // Paramters;
  int nx = NX; 
  int ny = NY; 
  int nz = NZ; 
  int N  = (nx*ny*nz);
  bsg_pr_info("nx=%d\n",nx);
  bsg_pr_info("ny=%d\n",ny);
  bsg_pr_info("nz=%d\n",nz);
  int c0 = 2;
  int c1 = 4;

  // Allocate memory on host;
  float *A0 = (float*) malloc(N*sizeof(float));
  float *Anext = (float*) malloc(N*sizeof(float));
  for (int i = 0; i < N; i++) {
    A0[i] = (float) (i % 13);
    Anext[i] = 0.0f;
  }
  host_jacobi(c0, c1, A0, Anext, nx, ny, nz);


  // Initialize Device
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "jacobi_multipod", HB_MC_DEVICE_ID));
  eva_t d_A0;
  eva_t d_Anext;


  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
      //if (pod % 2 == 1) continue;

    bsg_pr_info("Loading program for pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // Allocate memory on device;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, N * sizeof(float), &d_A0));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, N * sizeof(float), &d_Anext));

    // DMA transfer;
    bsg_pr_info("Transferring data: pod %d\n", pod);
    std::vector<hb_mc_dma_htod_t> htod_job;
    htod_job.push_back({d_A0, A0, N*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&device, htod_job.data(), htod_job.size()));


    // Cuda arguments;
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 8
    uint32_t cuda_argv[CUDA_ARGC] = {c0, c1, d_A0, d_Anext, nx, ny, nz, pod};

    // Enqueue kernel.
    bsg_pr_info("Enqueue Kernel: pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_kernel_enqueue(&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }


  // Launch pods;
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
        for (int podi = podc-1; podi >= 0; podi--)
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
  bool fail = false;  
  float *actual_Anext = (float*) malloc(N*sizeof(float));

  hb_mc_device_foreach_pod_id(&device, pod) {
      //if (pod % 2 == 1) continue;

    bsg_pr_info("Reading results: pods %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));

    // clear;
    for (int i = 0; i < N; i++) {
      actual_Anext[i] = 0.0f;
    }

    // DMA transfer; device -> host;
    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    dtoh_job.push_back({d_Anext, actual_Anext, N*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job.data(), dtoh_job.size()));


    // validate;
    for (int i = 0; i < N; i++) {
      int actual = actual_Anext[i];
      int expected = Anext[i];
      int x = (i % (nx*nz)) / nz;
      int y = i / (nx*nz);
      int z = i % nz;
      if (actual != expected) {
        bsg_pr_info("mismatch: (%d %d %d) actual=%f, expected=%f\n", x, y, z, actual, expected);
        fail = true;
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

declare_program_main("jacobi_multipod", jacobi_multipod);
