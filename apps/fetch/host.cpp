#include <standard/host/program.hpp>

class program : public standard::program {
    int input() override {
        standard::program::input();
        auto A_p = find<hb_mc_eva_t>("A");
        foreach_pod([this, A_p](hb_mc_coordinate_t pod) mutable {
            auto pod_id = pod_coord_to_id(pod);
            hb_mc_eva_t _, buf;
            BSG_CUDA_CALL(alloc_cache_aligned(pod_id, SIZE, &_, &buf));
            *A_p = buf;
            return 0;
        });
        return 0;
    }

    virtual bool cold_cache() const { return true; }
};

standard::program* make_program() {
    return new program;
}
