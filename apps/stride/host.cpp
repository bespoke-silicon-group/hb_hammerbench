#include <standard/host/program.hpp>

class program : public standard::program {
public:
    int input() {
        BSG_CUDA_CALL(standard::program::input());
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            auto pod = pod_id_to_coord(pod_id);
            auto dram_node = find<hb_mc_eva_t>("dram_node");
            dram_node.set_pod_x(pod.x).set_pod_y(pod.y);
            hb_mc_eva_t nodes;
            BSG_CUDA_CALL(hb_mc_device_pod_malloc(&mc, pod_id, (1+VECTOR_SIZE) * sizeof(hb_mc_eva_t), &nodes));
            *dram_node = nodes;
        }
        return 0;
    }

    bool cold_cache() const override {
#ifdef WARM_CACHE
        return false;
#else
        return true;
#endif
    }
};

standard::program *make_program() {
    return new program;
}
