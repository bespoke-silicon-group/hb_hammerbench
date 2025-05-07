#include <standard/host/program.hpp>
#include <bsg_manycore_loader.h>
#include <chrono>
#include <fstream>

namespace standard
{
program::program() {}

int program::init(int argc, char **argv) {
    char *program = argv[1];
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(hb_mc_device_init(&this->mc, "standard_program", HB_MC_DEVICE_ID));
    bsg_global_pointer::the_device = &this->mc;
    jobs_in.resize(this->mc.num_pods);
    jobs_out.resize(this->mc.num_pods);

    //hb_mc_dimension_t tg = mc.mc->config.pod_shape;
    hb_mc_pod_id_t pod_id;
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_pod_program_init(&this->mc, pod_id, program));
    }

    auto pod_x_p  = find<int>("__bsg_pod_x");
    auto pod_y_p  = find<int>("__bsg_pod_y");
    auto pod_id_p = find<int>("__bsg_pod_id");
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        auto pod = pod_id_to_coord(pod_id);
        pod_x_p.set_pod_x(pod.x).set_pod_y(pod.y);
        pod_y_p.set_pod_x(pod.x).set_pod_y(pod.y);
        pod_id_p.set_pod_x(pod.x).set_pod_y(pod.y);
        int x = pod.x;
        int y = pod.y;
        int id = y * mc.mc->config.pods.x + x;
        *pod_x_p = x;
        *pod_y_p = y;
        *pod_id_p = id;
    }
    return 0;
}

int program::input() {
    hb_mc_pod_id_t pod_id;
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    return 0;
}

int program::sync_input() {
    hb_mc_pod_id_t pod_id;
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_set_default_pod(&this->mc, pod_id));
        BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&this->mc, jobs_in[pod_id].data(), jobs_in[pod_id].size()));
        jobs_in[pod_id].clear();
    }    
    return 0;
}

int program::run() {
    bsg_pr_test_info("%s: enqueing setup\n", __PRETTY_FUNCTION__);
    hb_mc_pod_id_t pod_id;
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&this->mc, pod_id,
                                                      {1,1}, this->tg,
                                                      "setup", 0, 0));
    }
    bsg_pr_test_info("%s: executing setup\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&this->mc));
    if (cold_cache()) {
        BSG_CUDA_CALL(hb_mc_manycore_flush_vcache(mc.mc));
        BSG_CUDA_CALL(hb_mc_manycore_invalidate_vcache(mc.mc));
    }
    bsg_pr_test_info("%s: enqueing kernel\n", __PRETTY_FUNCTION__);
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&this->mc, pod_id,
                                                      {1,1}, this->tg,
                                                      "kernel", 0, 0));
    }
    bsg_pr_test_info("%s: executing kernel\n", __PRETTY_FUNCTION__);
    const auto start{std::chrono::steady_clock::now()};
    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&this->mc));
    const auto stop {std::chrono::steady_clock::now()};
    std::ofstream ns_log("kernel_ns.log");
    const std::chrono::duration<long long, std::nano> ns{stop - start};
    ns_log << ns.count() << std::endl;
    bsg_pr_test_info("%s: kernel executected in %lld ns\n", __PRETTY_FUNCTION__, ns.count());
    bsg_pr_test_info("%s: done\n", __PRETTY_FUNCTION__);
    return 0;
}

int program::output() {
  bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    return 0;
}

int program::sync_output() {
    hb_mc_pod_id_t pod_id;
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_set_default_pod(&this->mc, pod_id));
        BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&this->mc, jobs_out[pod_id].data(), jobs_out[pod_id].size()));
        jobs_out[pod_id].clear();
    }
    return 0;
}

int program::check_output() {
  bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    return 0;
}

int program::fini() {
    hb_mc_pod_id_t pod_id;
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_pod_program_finish(&this->mc, pod_id));
    }
    BSG_CUDA_CALL(hb_mc_device_finish(&this->mc));
    return 0;
}

hb_mc_eva_t program::find(const char*symbol) {
    hb_mc_program_t *prog = mc.pods[0].program;
    hb_mc_eva_t eva;
    BSG_CUDA_CALL(hb_mc_loader_symbol_to_eva(prog->bin, prog->bin_size, symbol, &eva));
    return eva;
}

}

/**
 * @brief create a new program
 */
__attribute__((weak))
standard::program * make_program() {
    return new standard::program;
}
