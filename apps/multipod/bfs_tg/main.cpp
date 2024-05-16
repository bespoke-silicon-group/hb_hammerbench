#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <fstream>

#define ALLOC_NAME "default_allocator"
#define NUM_QUERY 8
#define NUMPODS 64

void read_graph(int* ptr, std::string filename)
{
  std::ifstream fh(filename);
  int idx = 0;
  int num;
   
  while (fh >> num) {
    ptr[idx] = num;
    idx++;
  }
}


void read_vector(std::vector<int> &vec, std::string filename)
{
  std::ifstream fh(filename);
  int num;
  while (fh >> num) {
    vec.push_back(num);
  }
}



// Program main;
int bfs_multipod(int argc, char ** argv)
{
  int r = 0;

  // Command Line arguments;
  const char * bin_path = argv[1];
  std::string fwd_offsets_file  = argv[2];
  std::string fwd_nonzeros_file = argv[3];


  // Read input graph;
  int V = VERTEX;
  int E = EDGE;
  int niter = NITER;
  std::cout << "V = " << V << std::endl;
  std::cout << "E = " << E << std::endl;
  int * fwd_offsets = (int *) malloc(sizeof(int)*(V+1));
  int * fwd_nonzeros = (int *) malloc(sizeof(int)*(E));
  read_graph(fwd_offsets,  fwd_offsets_file);
  read_graph(fwd_nonzeros,  fwd_nonzeros_file);

  
  // List of roots;
  int roots[NUM_QUERY] = {985640, 1, 8146, 355929, 105206, 235761, 614511, 691594};


  // dense frontier size;
  int num_dense_words = (V+31)/32;
  int rem = num_dense_words%16;
  if (rem != 0) {
    num_dense_words += 16-rem;
  }

  int *next_dense_frontier = (int*) malloc(NUM_QUERY*num_dense_words*sizeof(int));
  for (int i = 0; i < NUM_QUERY*num_dense_words; i++) {
    next_dense_frontier[i] = 0;
  }



  // Initialize device;
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "bfs_multipod", 0));

 
 
  // Device memory;
  hb_mc_eva_t d_offsets;
  hb_mc_eva_t d_nonzeros;
  hb_mc_eva_t d_curr_frontier;
  hb_mc_eva_t d_curr_frontier_offsets;
  hb_mc_eva_t d_next_dense_frontier;
  hb_mc_eva_t d_curr_distance;
  // Host memory;
  std::vector<int> curr_frontier;
  std::vector<int> curr_distance(NUM_QUERY*V, -1);
  int curr_frontier_offsets[NUM_QUERY+1];
  curr_frontier_offsets[0] = 0;

  // Pod
  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    // Loading program;
    printf("Loading program for pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // for each query;
    for (int i = 0; i < NUM_QUERY; i++) {
      // read distance file;
      //std::string filename = std::format("../inputs/roadNet-CA.{}.distance.txt", roots[i]);
      std::string filename = "../inputs/roadNet-CA." + std::to_string(roots[i]) + ".distance.txt";
      std::cout << "Reading distance file: "<< filename << std::endl;
      std::vector<int> distance;
      read_vector(distance, filename);

      // get temp frontier;
      std::vector<int> temp_frontier;
      for (int j = 0; j < V; j++) {
        if (distance[j] == niter) {
          temp_frontier.push_back(j);
        }
      }

      // get curr distance;
      for (int j = 0; j < V; j++) {
        if (distance[j] <= niter) {
          curr_distance[(i*NUM_QUERY)+j] = distance[j];
        }
      }   

      for (int j = 0; j < temp_frontier.size()/NUMPODS; j++) {
        curr_frontier.push_back(temp_frontier[j]);
      }

      curr_frontier_offsets[i+1] = curr_frontier.size();
    }


    // Allocate memory on device;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (V+1)*sizeof(int), &d_offsets));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, E*sizeof(int), &d_nonzeros));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, curr_frontier.size()*sizeof(int), &d_curr_frontier));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (NUM_QUERY+1)*sizeof(int), &d_curr_frontier_offsets));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, curr_distance.size()*sizeof(int), &d_curr_distance));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NUM_QUERY*num_dense_words*sizeof(int), &d_next_dense_frontier));

    // DMA transfer;   
    printf("Transferring data: pod %d\n", pod);
    std::vector<hb_mc_dma_htod_t> htod_job;
    htod_job.push_back({d_offsets, fwd_offsets, (V+1)*sizeof(int)});
    htod_job.push_back({d_nonzeros, fwd_nonzeros, E*sizeof(int)});
    htod_job.push_back({d_curr_frontier, curr_frontier.data(), curr_frontier.size()*sizeof(int)});
    htod_job.push_back({d_curr_frontier_offsets, curr_frontier_offsets, (NUM_QUERY+1)*sizeof(int)});
    htod_job.push_back({d_curr_distance, curr_distance.data(), curr_distance.size()*sizeof(int)});
    htod_job.push_back({d_next_dense_frontier, next_dense_frontier, NUM_QUERY*num_dense_words*sizeof(int)});
    BSG_CUDA_CALL(hb_mc_device_dma_to_device(&device, htod_job.data(), htod_job.size()));

    // CUDA args
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 7
    uint32_t cuda_argv[CUDA_ARGC] = {
      // inputs
      d_offsets,
      d_nonzeros,
      d_curr_distance,
      d_curr_frontier,
      d_curr_frontier_offsets,
      d_next_dense_frontier,
      num_dense_words
    };
    
    // Enqueue kernel
    printf("Enqueing kernel: pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_kernel_enqueue(&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }


  // Launch pods
  printf("Launching all pods\n");
  BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&device));


  // Validate;
  bool fail = false;

  hb_mc_device_foreach_pod_id(&device, pod)
  {
    printf("Reading next frontier data: pods %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));

    // DMA
    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    dtoh_job.push_back({d_next_dense_frontier, next_dense_frontier, NUM_QUERY*num_dense_words*sizeof(int)});
    BSG_CUDA_CALL(hb_mc_device_dma_to_host(&device, dtoh_job.data(), dtoh_job.size()));

    for (int i = 0; i < NUM_QUERY; i++) {
      printf("Checking query: %d\n", i);
      // collect frontiers;
      std::set<int> next_frontier;
      next_frontier.clear();
      for (int j = 0; j < num_dense_words; j++) {
        for (int b = 0; b < 32; b++) {
          int bit = (next_dense_frontier[(i*num_dense_words) + j] >> b) & 1;
          if (bit) {
            next_frontier.insert((32*j)+b);
          }
        }
      }
 
      // Expected next frontier;
      std::set<int> expected_next_frontier;
      // FWD
      for (int f = curr_frontier_offsets[i]; f < curr_frontier_offsets[i+1]; f++) {
        int src = curr_frontier[f];
        int nz_start = fwd_offsets[src];
        int nz_end = fwd_offsets[src+1];
        for (int nz = nz_start; nz < nz_end; nz++) {
          int dst = fwd_nonzeros[nz];
          if (curr_distance[(V*i)+dst] == -1) {
            expected_next_frontier.insert(dst);
          }
        }
      }
     

      // is expected in actual?
      std::set<int>::iterator it;
      for (it = expected_next_frontier.begin(); it != expected_next_frontier.end(); it++) {
        int val = *it;
        if (next_frontier.find(val) == next_frontier.end()) {
          printf("FAIL: Frontier %d not in actual.\n", val);
          fail = true;
        }
      }

      // is actual in expected?
      for (it = next_frontier.begin(); it != next_frontier.end(); it++) {
        int val = *it;
        if (expected_next_frontier.find(val) == expected_next_frontier.end()) {
          printf("FAIL: Frontier %d not in expected.\n", val);
          fail = true;
        }
      }
    }
  }





  // Finish;
  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  if (fail) {
    return HB_MC_FAIL;
  } else {
    return HB_MC_SUCCESS;
  }
  
}



declare_program_main("bfs-multipod", bfs_multipod);

