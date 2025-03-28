#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"

DRAM(csx_type) matrix;

#define info(fmt, ...)                          \
    bsg_printf("[%d,%d]: " fmt "\n", cello::my::pod_x(), cello::my::pod_y(), ##__VA_ARGS__)
        
int cello_main(int argc, char *argv[])
{
    info("hello, from cello_csr");
#if 0
    cello::on_every_pod([](){
        info("matrix:\n"
             "  inner_size = %d\n"
             "  outer_size = %d\n"
             "  outer_pointers.size() = %d\n"
             "  outer_pointers.data() = %x\n"
             "  inner_indices = %x\n"
             "  values = %x\n"
             , matrix.inner_size()
             , matrix.outer_size()
             , matrix.outer_pointers().size()
             , matrix.outer_pointers().data()
             , matrix.inner_indices()
             , matrix.values());
        
    });
    matrix.outer_pointers().foreach([](int i, int ptr){
        int start = ptr;
        int j = matrix.outer_pointers().lcl(i)+1;
        int end = matrix.outer_pointers().data()[j];
        info("outer_pointers[%3d] = %3d - %3d: %3d\n"
             , i
             , start
             , end
             , end-start);
    });
#endif
#if 1
    matrix.foreach_outer_index_inner_indices_values
        ([](int32_t row, int32_t *cols, float *values, int32_t nnz) {
            for (int i = 0; i < nnz; i++) {
                
            }
        });
#endif   
    return 0;
}
