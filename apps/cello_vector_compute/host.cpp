#include <cello/host/program.hpp>
#include "common.hpp"

class program : public cello::program {
    int init(int argc, char *argv[]) override {
        BSG_CUDA_CALL(cello::program::init(argc, argv));
        vsize = atoi(argv[2]);
        bsg_pr_test_info("vsize = '%s' = %d\n", argv[2], vsize);
        std::vector<int> v(vsize);
        V = new vec::mirror(find<vec>("V"));        
        BSG_CUDA_CALL(V->init_host_from(v));
        return 0;
    }
    int input() override {
        BSG_CUDA_CALL(cello::program::input());
        BSG_CUDA_CALL(V->sync_device(jobs_in));
        return 0;
    }
    int output() override {
        BSG_CUDA_CALL(cello::program::output());
        return 0;
    }
    vec::mirror *V;
    int vsize;
};

cello::program *make_program() {
    return new program;
}
    
