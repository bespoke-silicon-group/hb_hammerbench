#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bfs/graph.h"
#include "bfs/sparse_set.h"
#include "bsg_manycore_atomic.h"
#include <atomic>

#include "bsg_cuda_lite_barrier.h"



#define GRANULARITY_PULL 20
#define GRANULARITY_PUSH 5
#define GRANULARITY_INDEX 16

//bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

__attribute__((section(".dram"))) std::atomic<int> workq;

__attribute__((section(".dram"))) std::atomic<int> index_rd;

__attribute__((section(".dram"))) std::atomic<int> index_wr;

extern "C" int bfs(graph_t * bsg_attr_noalias G_csr_ptr,
        graph_t * bsg_attr_noalias G_csc_ptr,
        sparse_set_t bsg_attr_remote * bsg_attr_noalias frontier_in_sparse,
        int bsg_attr_remote * bsg_attr_noalias frontier_in_dense,
        int bsg_attr_remote * bsg_attr_noalias frontier_out_sparse,
        int bsg_attr_remote * bsg_attr_noalias frontier_out_dense,
        int bsg_attr_remote * bsg_attr_noalias visited,
        int bsg_attr_remote * bsg_attr_noalias direction,
        //bsg_attr_remote int *ite_id,
        int bsg_attr_remote * bsg_attr_noalias outlen,
        int bsg_attr_remote * bsg_attr_noalias cachewarm   ){

    //bsg_cuda_print_stat_start(*ite);
    bsg_barrier_hw_tile_group_init();
    
    //pseduo read to warm up LLC with input frontier for testing road maps
    //shold be commented if input frontier size is larger than 512KB
    int cmp = 0;
    if(*cachewarm==1){
      for(int i=0; i<frontier_in_sparse->set_size;i++){
          int tmp = frontier_in_sparse->members[i];
          if (tmp > cmp) cmp = tmp;
      }
    }

    //bsg_cuda_print_stat_start(0);
    bsg_cuda_print_stat_kernel_start();
    //PULL direction
    if (*direction == 0){
        
        graph_t G = *G_csr_ptr;
        int num_nodes = G.V;
        for (int src_base_i = workq.fetch_add(GRANULARITY_PULL, std::memory_order_relaxed); src_base_i < num_nodes; src_base_i = workq.fetch_add(GRANULARITY_PULL, std::memory_order_relaxed)) {
           // update all neibs
           
            //bsg_printf("===== src_base_i is %d, from tile %d=======================================\n",src_base_i,__bsg_tile_group_id);    
           
            int stop = (src_base_i + GRANULARITY_PULL > num_nodes) ? num_nodes : src_base_i + GRANULARITY_PULL;
            for (int src_i = src_base_i; src_i<stop; src_i++){
                //use mask to check vertex[src_i] has not been visted yet
                int bit_chunk = src_i/32;
                int bit_position = 1<< (src_i % 32);
                int visited_check = visited[bit_chunk] & bit_position;
                if (!visited_check){
                    //int temp_frontier_idx=0
                    kernel_vertex_data_ptr_t src_data = &G.vertex_data[src_i];
                    int degree = src_data->degree;
                    kernel_edge_data_ptr_t neib = src_data->neib;
                    //check each incoming edge,if one of the incoming edge matches the frontier, set match =1 and break
                    for (int dst_i = 0; dst_i < degree; dst_i++) {
                        //dst is the incoming edge currently being checked
                        int dst = neib[dst_i];
                   
                        //if find a match in the coming edge, stop matching other coming edges
                        
                        if (frontier_in_dense[dst/32] & (1<<(dst%32))){
                            int result_visit = bsg_amoor(&visited[bit_chunk],bit_position);
                            //visited[src_i] = 1;
                            int result_frontier = bsg_amoor(&frontier_out_dense[bit_chunk],bit_position);
                            int out_idx = index_wr.fetch_add(1, std::memory_order_relaxed);
                            frontier_out_sparse[out_idx] = src_i; 
                            //frontier_out_dense[src_i] = 1;
                            break;
                        }
                    }
                }
            }
        }
        //bsg_cuda_print_stat_end(1);

    }
    else{
        //bsg_cuda_print_stat_start(2);
        graph_t G = *G_csc_ptr;
        int num_nodes = G.V;
        bsg_unroll(1)
        for (int src_base_i = workq.fetch_add(GRANULARITY_PUSH, std::memory_order_relaxed); src_base_i < frontier_in_sparse->set_size; src_base_i = workq.fetch_add(GRANULARITY_PUSH, std::memory_order_relaxed)) {
        // update all neibs
            int stop = (src_base_i + GRANULARITY_PUSH > frontier_in_sparse->set_size) ? frontier_in_sparse->set_size : src_base_i + GRANULARITY_PUSH;
            
            bsg_unroll(1)
            for (int src_i = src_base_i; src_i<stop; src_i++){
                int src = frontier_in_sparse->members[src_i];
                kernel_vertex_data_ptr_t src_data = &G.vertex_data[src];
                int degree = src_data->degree;
                kernel_edge_data_ptr_t neib = src_data->neib;

                // UNROLL by 4
                int dst_i=0;
                bsg_unroll(1)
                for (; dst_i+3<degree; dst_i+=4){
                  int dst0 = neib[dst_i];
                  int dst1 = neib[dst_i+1];
                  int dst2 = neib[dst_i+2];
                  int dst3 = neib[dst_i+3];
                  asm volatile ("": : :"memory");

                  int bitmask0 = 1<<(dst0%32);
                  int bitmask1 = 1<<(dst1%32);
                  int bitmask2 = 1<<(dst2%32);
                  int bitmask3 = 1<<(dst3%32);

                  int visited_reg[4];
                  visited_reg[0] = visited[dst0/32]&bitmask0;
                  visited_reg[1] = visited[dst1/32]&bitmask1;
                  visited_reg[2] = visited[dst2/32]&bitmask2;
                  visited_reg[3] = visited[dst3/32]&bitmask3;
                  asm volatile ("": : :"memory");

                  int result_visit[4];
                  result_visit[0] = 0xffffffff;
                  result_visit[1] = 0xffffffff;
                  result_visit[2] = 0xffffffff;
                  result_visit[3] = 0xffffffff;
                  asm volatile ("": : :"memory");
                  if (!visited_reg[0]) result_visit[0] = bsg_amoor(&visited[dst0/32],bitmask0);
                  if (!visited_reg[1]) result_visit[1] = bsg_amoor(&visited[dst1/32],bitmask1);
                  if (!visited_reg[2]) result_visit[2] = bsg_amoor(&visited[dst2/32],bitmask2);
                  if (!visited_reg[3]) result_visit[3] = bsg_amoor(&visited[dst3/32],bitmask3);
                  asm volatile ("": : :"memory");

                  bool do_visit[4];
                  do_visit[0] = !(result_visit[0] & bitmask0);
                  do_visit[1] = !(result_visit[1] & bitmask1);
                  do_visit[2] = !(result_visit[2] & bitmask2);
                  do_visit[3] = !(result_visit[3] & bitmask3);

                  int out_idx[4];
                  if (do_visit[0]) out_idx[0] = index_wr.fetch_add(1, std::memory_order_relaxed);
                  if (do_visit[1]) out_idx[1] = index_wr.fetch_add(1, std::memory_order_relaxed);
                  if (do_visit[2]) out_idx[2] = index_wr.fetch_add(1, std::memory_order_relaxed);
                  if (do_visit[3]) out_idx[3] = index_wr.fetch_add(1, std::memory_order_relaxed);

                  if (do_visit[0]) frontier_out_sparse[out_idx[0]] = dst0;
                  if (do_visit[1]) frontier_out_sparse[out_idx[1]] = dst1;
                  if (do_visit[2]) frontier_out_sparse[out_idx[2]] = dst2;
                  if (do_visit[3]) frontier_out_sparse[out_idx[3]] = dst3;
                }
                // UNROLL by 3
                bsg_unroll(1)
                for (; dst_i+2<degree; dst_i+=3){
                  int dst0 = neib[dst_i];
                  int dst1 = neib[dst_i+1];
                  int dst2 = neib[dst_i+2];
                  asm volatile ("": : :"memory");

                  int bitmask0 = 1<<(dst0%32);
                  int bitmask1 = 1<<(dst1%32);
                  int bitmask2 = 1<<(dst2%32);

                  int visited_reg[3];
                  visited_reg[0] = visited[dst0/32]&bitmask0;
                  visited_reg[1] = visited[dst1/32]&bitmask1;
                  visited_reg[2] = visited[dst2/32]&bitmask2;
                  asm volatile ("": : :"memory");

                  int result_visit[3];
                  result_visit[0] = 0xffffffff;
                  result_visit[1] = 0xffffffff;
                  result_visit[2] = 0xffffffff;
                  asm volatile ("": : :"memory");
                  if (!visited_reg[0]) result_visit[0] = bsg_amoor(&visited[dst0/32],bitmask0);
                  if (!visited_reg[1]) result_visit[1] = bsg_amoor(&visited[dst1/32],bitmask1);
                  if (!visited_reg[2]) result_visit[2] = bsg_amoor(&visited[dst2/32],bitmask2);
                  asm volatile ("": : :"memory");

                  bool do_visit[3];
                  do_visit[0] = !(result_visit[0] & bitmask0);
                  do_visit[1] = !(result_visit[1] & bitmask1);
                  do_visit[2] = !(result_visit[2] & bitmask2);

                  int out_idx[3];
                  if (do_visit[0]) out_idx[0] = index_wr.fetch_add(1, std::memory_order_relaxed);
                  if (do_visit[1]) out_idx[1] = index_wr.fetch_add(1, std::memory_order_relaxed);
                  if (do_visit[2]) out_idx[2] = index_wr.fetch_add(1, std::memory_order_relaxed);

                  if (do_visit[0]) frontier_out_sparse[out_idx[0]] = dst0;
                  if (do_visit[1]) frontier_out_sparse[out_idx[1]] = dst1;
                  if (do_visit[2]) frontier_out_sparse[out_idx[2]] = dst2;
                }
                // UNROLL by 2
                bsg_unroll(1)
                for (; dst_i+1<degree; dst_i+=2){
                  int dst0 = neib[dst_i];
                  int dst1 = neib[dst_i+1];
                  asm volatile ("": : :"memory");

                  int bitmask0 = 1<<(dst0%32);
                  int bitmask1 = 1<<(dst1%32);

                  int visited_reg[2];
                  visited_reg[0] = visited[dst0/32]&bitmask0;
                  visited_reg[1] = visited[dst1/32]&bitmask1;
                  asm volatile ("": : :"memory");

                  int result_visit[2];
                  result_visit[0] = 0xffffffff;
                  result_visit[1] = 0xffffffff;
                  asm volatile ("": : :"memory");
                  if (!visited_reg[0]) result_visit[0] = bsg_amoor(&visited[dst0/32],bitmask0);
                  if (!visited_reg[1]) result_visit[1] = bsg_amoor(&visited[dst1/32],bitmask1);
                  asm volatile ("": : :"memory");

                  bool do_visit[2];
                  do_visit[0] = !(result_visit[0] & bitmask0);
                  do_visit[1] = !(result_visit[1] & bitmask1);

                  int out_idx[2];
                  if (do_visit[0]) out_idx[0] = index_wr.fetch_add(1, std::memory_order_relaxed);
                  if (do_visit[1]) out_idx[1] = index_wr.fetch_add(1, std::memory_order_relaxed);

                  if (do_visit[0]) frontier_out_sparse[out_idx[0]] = dst0;
                  if (do_visit[1]) frontier_out_sparse[out_idx[1]] = dst1;
                }

                // UNROLL by 1
                bsg_unroll(1)
                for (; dst_i<degree; dst_i+=1){
                  int dst0 = neib[dst_i];
                  asm volatile ("": : :"memory");

                  int bitmask0 = 1<<(dst0%32);

                  int visited_reg[1];
                  visited_reg[0] = visited[dst0/32]&bitmask0;
                  asm volatile ("": : :"memory");

                  int result_visit[1];
                  result_visit[0] = 0xffffffff;
                  asm volatile ("": : :"memory");
                  if (!visited_reg[0]) result_visit[0] = bsg_amoor(&visited[dst0/32],bitmask0);
                  asm volatile ("": : :"memory");

                  bool do_visit[1];
                  do_visit[0] = !(result_visit[0] & bitmask0);

                  int out_idx[1];
                  if (do_visit[0]) {
                    out_idx[0] = index_wr.fetch_add(1, std::memory_order_relaxed);
                    frontier_out_sparse[out_idx[0]] = dst0;
                  }
                }

        

/*
                  if(!(visited[dst/32]&(1<<(dst%32)))){
                    int result_visit = bsg_amoor(&visited[dst/32],1<<(dst%32));  
                    if (! (result_visit & (1<<(dst%32)) ) ){
                      int out_idx = index_wr.fetch_add(1, std::memory_order_relaxed);
                      frontier_out_sparse[out_idx] = dst;  
                    }
                    //int result_frontier = bsg_amoor(&frontier_out_dense[dst/32],1<<(dst%32));
                  }
*/


            }
        }
        //bsg_cuda_print_stat_end(2);   
    }
    //bsg_cuda_print_stat_end(0);
    bsg_cuda_print_stat_kernel_end();
    // the phase which generates output frontier in sparse set format
    //#############################################################
    bsg_barrier_hw_tile_group_sync();
    //barrier.sync();
    //#############################################################
    //bsg_cuda_print_stat_start(1);
    
    
    /*for (int src_base_i = index_rd.fetch_add(GRANULARITY_INDEX, std::memory_order_relaxed); src_base_i < *outlen; src_base_i = index_rd.fetch_add(GRANULARITY_INDEX, std::memory_order_relaxed)) {
        int stop = (src_base_i + GRANULARITY_INDEX > *outlen) ? *outlen : src_base_i + GRANULARITY_INDEX;
        for (int src_i = src_base_i; src_i<stop; src_i++){
            if(frontier_out_dense[src_i/32]&(1<<(src_i%32))){
                int out_idx = index_wr.fetch_add(1, std::memory_order_relaxed);
                frontier_out_sparse[out_idx] = src_i;
                //int result_frontier = bsg_amoor(&frontier_out_dense[src_i/32],1<<(src_i%32));
                //frontier_out_dense[src_i] = 0;
                //bsg_printf("========================= output frontier element : %d, src_base_i: %d, tile id: %d=======================================\n",src_i,src_base_i,__bsg_id);
            }
        }
    }
    */
    //bsg_cuda_print_stat_end(1);
    //write the output frontier length
    //#############################################################
    bsg_barrier_hw_tile_group_sync();
    //#############################################################
    //bsg_cuda_print_stat_start(3);
    if(__bsg_id == 0){
        *outlen = index_wr.load();   
    //    bsg_printf("========================= output frontier size : %d, work_q value: %d, rd_idx value: %d=======================================\n",index_wr.load(),workq.load(),index_rd.load());
        *direction = cmp; // write cmp so that the pseduo code is not optimized away
    }

    //bsg_cuda_print_stat_end(1);
    bsg_barrier_hw_tile_group_sync();

    //bsg_cuda_print_stat_end(1);
    return 0;
}
