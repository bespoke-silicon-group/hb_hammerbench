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
#include "host_bellman_ford.hpp"

#define ALLOC_NAME "default_allocator"


float** read_graph(std::string filename, int* num_vertices, int* num_edges)
{
  // open the file
  std::ifstream file(filename);

  // we know this is matrix market format
  // so we should read the third line

  // line buffer
  #define BUFSIZE 1024
  char buf[BUFSIZE];

  file.getline(buf, BUFSIZE); // 1st (throw away)
  file.getline(buf, BUFSIZE); // 2nd (throw away)
  file.getline(buf, BUFSIZE); // 3rd (important--matrix dimensions)

  // read in dimensions
  int n, m, e;
  if (sscanf(buf, "%d %d %d", &n, &m, &e) < 3) {
    perror("cannot read matrix dimensions");
    file.close();
    return NULL;
  }

  // allocate space for the matrix
  float** matrix = (float**) malloc(n *  sizeof(float*));
  if (matrix == NULL) {
    perror("malloc");
    file.close();
    return NULL;
  }
  for (int i = 0; i < n; i++) {
    matrix[i] = (float*) malloc(m * sizeof(float));
    if (matrix[i] == NULL) {
      perror("malloc");
      file.close();
      for (int j = 0; j < i; j++) {
        free(matrix[j]);
      }
      free(matrix);
      return NULL;
    }
    for (int j = 0; j < m; j++) {
      matrix[i][j] = INFINITY;
    }
  }

  // write in the edges as an adjacency matrix
  int v1, v2;
  float weight;
  file.getline(buf, BUFSIZE);
  while (!file.eof()) {
    if (sscanf(buf, "%d %d %f", &v1, &v2, &weight) < 3) {
      perror("bad matrix line\n");
      for (int j = 0; j < n; j++) {
        free(matrix[j]);
      }
      free(matrix);
      file.close();
      return NULL;
    }
    matrix[v1 -1][v2 - 1] = weight;
    file.getline(buf, BUFSIZE);
  }

  // we're done!
  *num_vertices = n;
  *num_edges = e;
  file.close();


  // print matrix
  // std::cout << "Printing 2-level matrix" << std::endl;
  // for (int i = 0; i < m; i++) {
  //   for (int j = 0; j < n; j++) {
  //     printf("%f ", matrix[i][j]);
  //   }
  //   printf("\n");
  // }

  return matrix;
}

float* flatten_graph(float** graph, int V)
{
  // std::cout << "Printing Flattened Matrix: " << std::endl;
  float* flat_graph = (float*) malloc(V*V*sizeof(float));
  for (int i = 0; i < V; i++) {
    // std::string line = "[";
    for (int j = 0; j < V; j++) {
      flat_graph[i*V + j] = graph[i][j];
      // line += std::to_string(graph[i][j]) + ", ";
    }
    // std::cout << line << "]" << std::endl;
  }
  return flat_graph;
}

int bellman_ford_multigroup(int argc, char ** argv)
{
  // Command Line arguments;
  const char * bin_path = argv[1];
  const char* graph_file = argv[2];
  
  std::cout << "bin_path = " << bin_path << std::endl;
  std::cout << "graph_file = " << graph_file << std::endl;
  
  // Define Params;
  int root = ROOT;
  int pod_id = PODID;
  std::cout << "root = " << root << std::endl;
  std::cout << "pod-id = " << pod_id << std::endl;

  // Read input graph;
  int V = -1; 
  int E = -1;
  float** graph = read_graph(graph_file, &V, &E);
  float* flat_graph = flatten_graph(graph, V);
  std::cout << "V = " << V << std::endl;
  std::cout << "E = " << E << std::endl;

  // Bellman-Ford on host;
  std::vector<float> distance;
  host_bellman_ford(root, V, graph, distance);


  // Initialize device;
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "bellman_ford_multigroup", HB_MC_DEVICE_ID));
 
  // Pod
  hb_mc_pod_id_t pod;
 
  hb_mc_eva_t d_matrix;
  hb_mc_eva_t d_rows[V];
  hb_mc_eva_t d_gdist;

  float* gdist = (float*) malloc(V*sizeof(float));
  for (int i = 0; i < V; i++) {
    gdist[i] = INFINITY;
  }
  gdist[root] = 0;

  hb_mc_device_foreach_pod_id(&device, pod)
  {
    // Loading program:
    printf("Loading program for pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // FLAT MATRIX
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, V*V*sizeof(float), &d_matrix)); // pointer array

    BSG_CUDA_CALL(hb_mc_device_malloc(&device, V*sizeof(float), &d_gdist)); // global distance array

    // DMA transfer;   
    printf("Transferring data: pod %d\n", pod);
    std::vector<hb_mc_dma_htod_t> htod_job;

    // FLAT MATRIX
    htod_job.push_back({d_matrix, flat_graph, V*V*sizeof(float)}); // flat adjacency matrix

    htod_job.push_back({d_gdist, gdist, V*sizeof(float)}); // global distance array

    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&device, htod_job.data(), htod_job.size()));

    // CUDA args
    hb_mc_dimension_t tg_dim = { .x = TILE_GROUP_DIM_X, .y = TILE_GROUP_DIM_Y};
    hb_mc_dimension_t grid_dim = { .x = (POD_DIM_X / TILE_GROUP_DIM_X), .y = (POD_DIM_Y / TILE_GROUP_DIM_Y)};
    #define CUDA_ARGC 4
    uint32_t cuda_argv[CUDA_ARGC] = {
      // inputs
      pod_id+(uint32_t)pod, // pod
      d_matrix,             // matrix
      (uint32_t) root,      // starting vertex
      d_gdist,              // global distance array
    };
    
    // Enqueue kernel
    printf("Enqueing kernel: pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_kernel_enqueue(&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }


  // Launch pods
  printf("Launching all pods\n");
  BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&device));
  // ...all done!

  // Read data;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    printf("Reading distance array: pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));

    // DMA
    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    dtoh_job.push_back({d_gdist, gdist, V*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job.data(), dtoh_job.size()));
  }

  // validate
  bool fail = false;

  // compare host Bellman-Ford to tile Bellman-Ford
  for (int i = 0; i < V; i++) {
    if (gdist[i] != distance[i]) {
      std::cout << "Vertex " << i << ": distance = " << gdist[i] << ", expected " << distance[i] << std::endl;
      fail = true;
    } else {
      std::cout << "Vertex " << i << ": distance = " << gdist[i] << std::endl;
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

declare_program_main("bellman-ford-multigroup", bellman_ford_multigroup);
