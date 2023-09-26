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

  // partition work;
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
  
  



  

  return HB_MC_SUCCESS;
}

declare_program_main("spgemm_multipod", spgemm_multipod);

