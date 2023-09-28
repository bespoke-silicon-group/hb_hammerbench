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
#include <math.h>

#define ALLOC_NAME "default_allocator"
#define DRAM_LIST_NODES 4194304 // (=2**22)

void read_graph(int* ptr, std::string filename) {
  std::ifstream fh(filename);
  int idx = 0;
  int num;
  while (fh >> num) {
    ptr[idx] = num;
    idx++;
  }
}

void read_graph_float(float* ptr, std::string filename) {
  std::ifstream fh(filename);
  int idx = 0;
  float num;
  while (fh >> num) {
    ptr[idx] = num;
    idx++;
  }
}

int spgemm_multipod(int argc, char ** argv)
{
  int r = 0;
  
  // command line arguments;
  const char * bin_path = argv[1];
  std::string row_offset_file = argv[2];
  std::string col_idx_file = argv[3];
  std::string nnz_file  = argv[4];
  std::string output_row_offset_file = argv[5];
  std::string output_col_idx_file = argv[6];
  std::string output_nnz_file = argv[7];
  std::cout << "row_offset_file = "  << row_offset_file << std::endl;
  std::cout << "col_idx_file = "  << col_idx_file << std::endl;
  std::cout << "nnz_file = "  << nnz_file << std::endl;
  std::cout << "output_row_offset_file = "  << output_row_offset_file << std::endl;
  std::cout << "output_col_idx_file = "  << output_col_idx_file << std::endl;
  std::cout << "output_nnz_file = "  << output_nnz_file << std::endl;

  // Define parameters;
  int pod_id = PODID;
  int numpods = NUMPODS;
  int V = VERTEX;
  int E = EDGE;
  int OUTPUT_E = OUTPUT_EDGE;
  printf("pod_id = %d\n", pod_id);
  printf("numpods = %d\n", numpods);
  printf("V=%d\n", V);
  printf("E=%d\n", E);
  printf("OUTPUT_E=%d\n", OUTPUT_E);

  // Read matrix;
  int *row_offset = (int *) malloc(sizeof(int)*(V+1));
  int *col_idx    = (int *) malloc(sizeof(int)*(E));
  float *nnz        = (float *) malloc(sizeof(float)*(E));
  int *output_row_offset = (int *) malloc(sizeof(int)*(V+1));
  int *output_col_idx    = (int *) malloc(sizeof(int)*(OUTPUT_E));
  float *output_nnz        = (float *) malloc(sizeof(float)*(OUTPUT_E));
  read_graph(row_offset, row_offset_file);
  read_graph(col_idx, col_idx_file);
  read_graph_float(nnz, nnz_file);
  read_graph(output_row_offset, output_row_offset_file);
  read_graph(output_col_idx, output_col_idx_file);
  read_graph_float(output_nnz, output_nnz_file);

  // partition by row (vertex);
  int V_per_pod = (V+numpods-1)/numpods;
  int psum[NUMPODS] = {0};
  int total_psum = 0;
  for (int p = 0; p < numpods; p++) {
    int row_start = std::min(p*V_per_pod,V);
    int row_end  = std::min(row_start+V_per_pod,V);
    for (int r = row_start; r < row_end; r++) {
      for (int cid = row_offset[r]; cid < row_offset[r+1]; cid++) {
        int c = col_idx[cid];
        psum[p] += (row_offset[c+1] - row_offset[c]);
      }
    }
    total_psum += psum[p];
    printf("pod=%d, psum=%d\n", p, psum[p]);
  }
  printf("total_psum=%d\n", total_psum);
  
 
  // Initialize device;
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "spgemm_multipod", 0));

  // Pod;
  hb_mc_pod_id_t pod;
  // matrix A
  hb_mc_eva_t d_A_row_offset[NUM_POD_X];
  hb_mc_eva_t d_A_col_idx[NUM_POD_X];
  hb_mc_eva_t d_A_nnz[NUM_POD_X];
  // matrix B
  hb_mc_eva_t d_B_row_offset[NUM_POD_X];
  hb_mc_eva_t d_B_col_idx[NUM_POD_X];
  hb_mc_eva_t d_B_nnz[NUM_POD_X];
  // matrix C
  hb_mc_eva_t d_C_row_offset[NUM_POD_X];
  hb_mc_eva_t d_C_col_idx[NUM_POD_X];
  hb_mc_eva_t d_C_nnz[NUM_POD_X];
  // needed by algorithm;
  hb_mc_eva_t d_C_col_count[NUM_POD_X];
  hb_mc_eva_t d_C_list_head[NUM_POD_X];
  hb_mc_eva_t d_dram_nodes[NUM_POD_X];
  

  hb_mc_device_foreach_pod_id(&device, pod)
  {
    // Loading program;
    printf("Loading program for pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // pod id;
    int curr_pod_id = pod_id + pod;
    int row_start = std::min(curr_pod_id*V_per_pod,V);
    int row_end  = std::min(row_start+V_per_pod,V);
    int num_row = row_end-row_start;
    int nnz_start = row_offset[row_start];
    int nnz_end = row_offset[row_end];
    int num_nnz = nnz_end-nnz_start;
    printf("curr_pod_id=%d, row_start=%d, row_end=%d\n", curr_pod_id, row_start, row_end);

    // Allocate memory on device;
    // matrix A
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_row+1)*sizeof(int), &d_A_row_offset[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_nnz)*sizeof(int), &d_A_col_idx[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_nnz)*sizeof(float), &d_A_nnz[pod]));
    // matrix B
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (V+1)*sizeof(int), &d_B_row_offset[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (E)*sizeof(int), &d_B_col_idx[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (E)*sizeof(float), &d_B_nnz[pod]));
    // matrix C
    int C_nnz_start = output_row_offset[row_start];
    int C_nnz_end = output_row_offset[row_end];
    int num_C_nnz = C_nnz_end - C_nnz_start;
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_row+1)*sizeof(int), &d_C_row_offset[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_C_nnz+1)*sizeof(int), &d_C_col_idx[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_C_nnz+1)*sizeof(int), &d_C_nnz[pod]));
    // needed by algorithm
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_row+1)*sizeof(int), &d_C_col_count[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (num_row)*sizeof(int), &d_C_list_head[pod]));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, (DRAM_LIST_NODES)*3*sizeof(int), &d_dram_nodes[pod]));
    
    // DMA transfer;
    printf("Transferring data: pod %d\n", pod);
    std::vector<hb_mc_dma_htod_t> htod_job;
    // matrix A;
    int *device_row_offset = (int*) malloc(sizeof(int)*(num_row+1)); 
    for (int i = 0; i < num_row+1; i++) {
      device_row_offset[i] = row_offset[row_start+i]-row_offset[row_start];
    }
    htod_job.push_back({d_A_row_offset[pod], device_row_offset, (num_row+1)*sizeof(int)});
    htod_job.push_back({d_A_col_idx[pod], &col_idx[nnz_start], (num_nnz)*sizeof(int)});
    htod_job.push_back({d_A_nnz[pod], &nnz[nnz_start], (num_nnz)*sizeof(float)});
    // matrix B
    htod_job.push_back({d_B_row_offset[pod], row_offset, (V+1)*sizeof(int)});
    htod_job.push_back({d_B_col_idx[pod], col_idx, (E)*sizeof(int)});
    htod_job.push_back({d_B_nnz[pod], nnz, (E)*sizeof(float)});
    // C col count;
    int *device_C_col_count = (int*) malloc(sizeof(int)*(num_row+1)); 
    for (int i = 0; i < num_row+1; i++) {
      device_C_col_count[i] = 0;
    }
    htod_job.push_back({d_C_col_count[pod], device_C_col_count, (num_row+1)*sizeof(int)});
    BSG_CUDA_CALL(hb_mc_device_dma_to_device(&device, htod_job.data(), htod_job.size()));



    // CUDA args;
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 14
    uint32_t cuda_argv[CUDA_ARGC] = {
      d_A_row_offset[pod],
      d_A_col_idx[pod],
      d_A_nnz[pod],
      d_B_row_offset[pod],
      d_B_col_idx[pod],
      d_B_nnz[pod],
      d_C_row_offset[pod],
      d_C_col_idx[pod],
      d_C_nnz[pod],
      d_C_col_count[pod],
      d_C_list_head[pod],
      d_dram_nodes[pod],
      num_row,
      pod
    };

    // Enqueue kernel;
    printf("Enqueing kernel: pod %d\n", pod);
    BSG_CUDA_CALL(hb_mc_kernel_enqueue(&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }

  // Launch pods;
  printf("Launching all pods\n");
  BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&device));

  // Read from device;
  bool fail = false;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    printf("Reading results: pods %d\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    int curr_pod_id = pod_id + pod;
    int row_start = std::min(curr_pod_id*V_per_pod,V);
    int row_end  = std::min(row_start+V_per_pod,V);
    int num_row = row_end-row_start;
    int nnz_start = row_offset[row_start];
    int nnz_end = row_offset[row_end];
    int num_nnz = nnz_end-nnz_start;
    int C_nnz_start = output_row_offset[row_start];
    int C_nnz_end = output_row_offset[row_end];
    int num_C_nnz = C_nnz_end - C_nnz_start;

    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    int *actual_C_row_offset  = (int*) malloc((num_row+1)*sizeof(int));
    int *actual_C_col_idx     = (int*) malloc((num_C_nnz)*sizeof(int));
    float *actual_C_nnz         = (float*) malloc((num_C_nnz)*sizeof(float));
    dtoh_job.push_back({d_C_row_offset[pod], actual_C_row_offset, (num_row+1)*sizeof(int)});
    dtoh_job.push_back({d_C_col_idx[pod], actual_C_col_idx, (num_C_nnz)*sizeof(int)});
    dtoh_job.push_back({d_C_nnz[pod], actual_C_nnz, (num_C_nnz)*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_dma_to_host(&device, dtoh_job.data(), dtoh_job.size()));

    // Validate;
    // row_offset;
    for (int row = 0; row < num_row+1; row++) {
      int host_row = row+row_start;
      int expected = output_row_offset[host_row]-output_row_offset[row_start];
      int actual = actual_C_row_offset[row];
      //printf("row_offset : row=%d, expected=%d, actual=%d\n",
      //  host_row, expected, actual);
      if (expected != actual) {
        printf("row_offset mismatch: row=%d, expected=%d, actual=%d\n",
          host_row, expected, actual);
        fail = true;
      }
    }
    // col_idx;
    for (int nz = 0; nz < num_C_nnz; nz++) {
      int host_nz = C_nnz_start+nz;
      int expected = output_col_idx[host_nz];
      int actual = actual_C_col_idx[nz];
      //printf("col_idx : nz=%d, expected=%d, actual=%d\n",
      //  host_nz, expected, actual);
      if (expected != actual) {
        printf("col_idx mismatch: nz=%d, expected=%d, actual=%d\n",
          host_nz, expected, actual);
        fail = true;
      }
    }
    // nnz;
    for (int nz = 0; nz < num_C_nnz; nz++) {
      int host_nz = C_nnz_start+nz;
      float expected = output_nnz[host_nz];
      float actual = actual_C_nnz[nz];
      //printf("nnz: nz=%d, expected=%f, actual=%f\n",
      //  host_nz, expected, actual);
      if (expected != actual) {
        printf("nnz mismatch: nz=%d, expected=%f, actual=%f\n",
          host_nz, expected, actual);
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

declare_program_main("spgemm_multipod", spgemm_multipod);

