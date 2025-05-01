#include <standard/host/program.hpp>
using namespace bsg_global_pointer;

class program : public standard::program {

    int init(int argc, char *argv[]) override {
        BSG_CUDA_CALL(standard::program::init(argc, argv));
        A_p = find<hb_mc_eva_t>("A");
        B_p = find<hb_mc_eva_t>("B");
        C_p = find<hb_mc_eva_t>("C");
    }

    int input() override {
        BSG_CUDA_CALL(standard::program::input());
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            auto pod = pod_id_to_coord(pod_id);
            hb_mc_eva_t A_eva, B_eva, C_eva, _;
            BSG_CUDA_CALL(alloc_cache_aligned(pod_id, SIZE*sizeof(int32_t), &_, &A_eva));
            BSG_CUDA_CALL(alloc_cache_aligned(pod_id, SIZE*sizeof(int32_t), &_, &B_eva));
            BSG_CUDA_CALL(alloc_cache_aligned(pod_id, SIZE*sizeof(int32_t), &_, &C_eva));
            A_p.set_pod_x(pod.x).set_pod_y(pod.y);
            B_p.set_pod_x(pod.x).set_pod_y(pod.y);
            C_p.set_pod_x(pod.x).set_pod_y(pod.y);
            *A_p = A_eva;
            *B_p = B_eva;
            *C_p = C_eva;
        }
    }

    int output() override {
        BSG_CUDA_CALL(standard::program::output());
    }

    int check_output() override {
        BSG_CUDA_CALL(standard::program::check_output());
    }

    pointer<hb_mc_eva_t> A_p, B_p, C_p;
};

standard::program *make_program() {
    return new standard::program;
}
