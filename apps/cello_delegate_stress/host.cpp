#include <cello/host/cello.hpp>

class program : public cello::program {
public:
    int init(int argc, char *argv[]) override {
        return cello::program::init(argc, argv);
    }

    int check_output() override {
        BSG_CUDA_CALL(cello::program::check_output());
        bsg_global_pointer::pointer<int> data_ptr = find<int>("data");
        bsg_pr_info("Checking output\n");
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            hb_mc_coordinate_t pod = hb_mc_index_to_coordinate
                (pod_id, mc.mc->config.pods);
            data_ptr.set_pod_x(pod.x).set_pod_y(pod.y);
            for (int i = 0; i < NITERS; i++) {
                int v = data_ptr[i];
                if (v != 1) {
                    bsg_pr_info("Error: data[%d] = %d\n", i, v);
                }
            }
        }
        return 0;
    }
};

cello::program *make_program() {
    return new program();
}
