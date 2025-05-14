#include <cello/host/program.hpp>
#include <cello/profile.hpp>
#include <bsg_manycore_loader.h>
#include <chrono>
#include <fstream>

namespace cello
{

static hb_mc_request_packet_id_t trace_ids [] = {
    RQST_ID(RQST_ID_ANY_X, RQST_ID_ANY_Y,  RQST_ID_ADDR( CELLO_TRACE_TOGGLE_EPA )),
    { /* sentinal */ }
};

static int trace_init(hb_mc_responder_t *responder, hb_mc_manycore_t *mc)
{
    return 0;
}

static int trace_quit(hb_mc_responder_t *responder, hb_mc_manycore_t *mc)
{
    return 0;
}

static int trace_respond(hb_mc_responder_t *responder, hb_mc_manycore_t *mc, const hb_mc_request_packet_t *rqst)
{
    int s = hb_mc_request_packet_get_data(rqst);
    if (s == 1) {
        BSG_CUDA_CALL(hb_mc_manycore_trace_enable(mc));
    } else if (s == 0) {
        BSG_CUDA_CALL(hb_mc_manycore_trace_disable(mc));
    }
    return 0;
}

program::program() {
    trace_ctrl = new hb_mc_responder ("cello_trace_responder", trace_ids, trace_init, trace_quit, trace_respond);
    trace_ctrl->responder_data = (void*)this;
    bsg_pr_test_info("initialized responder @ %p\n", trace_ctrl);
}

int program::init(int argc, char **argv) {
    char *program = argv[1];
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(hb_mc_responder_add(trace_ctrl));        
    BSG_CUDA_CALL(hb_mc_device_init(&this->mc, "cello_program", HB_MC_DEVICE_ID));
    bsg_global_pointer::the_device = &this->mc;
    jobs_in.resize(this->mc.num_pods);
    jobs_out.resize(this->mc.num_pods);
    cfgs.resize(this->mc.num_pods);

    BSG_CUDA_CALL(foreach_pod_id([=](hb_mc_pod_id_t pod_id){
        BSG_CUDA_CALL(hb_mc_device_pod_program_init(&this->mc, pod_id, program));
        return 0;
    }));
    return 0;
}

int program::input() {
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(foreach_pod_id([this](hb_mc_pod_id_t pod_id){
        hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, this->mc.mc->config.pods);

        this->cfgs[pod_id].dram_buffer_size() = 256*1024*1024;
        BSG_CUDA_CALL(hb_mc_device_pod_malloc(&this->mc, pod_id, this->cfgs[pod_id].dram_buffer_size(), &this->cfgs[pod_id].dram_buffer()));
        this->cfgs[pod_id].pod_x() = pod.x;
        this->cfgs[pod_id].pod_y() = pod.y;

        BSG_CUDA_CALL(hb_mc_device_pod_malloc(&this->mc, pod_id, sizeof(cello::config), &this->cfg_ptr));

        jobs_in[pod_id].push_back({cfg_ptr, (void*)&this->cfgs[pod_id], sizeof(cello::config)});
        return 0;
    }));
    return 0;
}

int program::sync_input() {
    hb_mc_pod_id_t pod_id;
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(foreach_pod_id([this](hb_mc_pod_id_t pod_id){
        BSG_CUDA_CALL(hb_mc_device_set_default_pod(&this->mc, pod_id));
        BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&this->mc, jobs_in[pod_id].data(), jobs_in[pod_id].size()));
        jobs_in[pod_id].clear();
        return 0;
    }));
    return 0;
}

int program::run() {
    hb_mc_pod_id_t pod_id;
    std::vector<uint32_t> argv = {cfg_ptr};
    bsg_pr_test_info("%s: enqueing setup\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(foreach_pod_id([=](hb_mc_pod_id_t pod_id){
        BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&this->mc, pod_id,
                                                      {1,1}, this->tg,
                                                      "cello_setup", argv.size(), argv.data()));
        return 0;
    }));
    bsg_pr_test_info("%s: executing setup\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&this->mc));
    bsg_pr_test_info("%s: enqueing main\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(foreach_pod_id([=](hb_mc_pod_id_t pod_id){
        BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&this->mc, pod_id,
                                                      {1,1}, this->tg,
                                                      "cello_start", argv.size(), argv.data()));
        return 0;
    }));
    bsg_pr_test_info("%s: executing main\n", __PRETTY_FUNCTION__);
    const auto start{std::chrono::steady_clock::now()};
    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&this->mc));
    const auto stop {std::chrono::steady_clock::now()};
    std::ofstream ns_log("kernel_ns.log");
    const std::chrono::duration<long long, std::nano> ns{stop - start};
    ns_log << ns.count() << std::endl;
    bsg_pr_test_info("%s: main executected in %lld ns\n", __PRETTY_FUNCTION__, ns.count());
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
    BSG_CUDA_CALL(foreach_pod_id([this](hb_mc_pod_id_t pod_id){
        BSG_CUDA_CALL(hb_mc_device_set_default_pod(&this->mc, pod_id));
        BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&this->mc, jobs_out[pod_id].data(), jobs_out[pod_id].size()));
        jobs_out[pod_id].clear();
        return 0;
    }));
    return 0;
}

int program::check_output() {
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    return 0;
}

int program::fini() {
    hb_mc_pod_id_t pod_id;
    bsg_pr_test_info("%s\n", __PRETTY_FUNCTION__);
    BSG_CUDA_CALL(foreach_pod_id([this](hb_mc_pod_id_t pod_id){
        BSG_CUDA_CALL(hb_mc_device_pod_program_finish(&this->mc, pod_id));
        return 0;
    }));
    BSG_CUDA_CALL(hb_mc_device_finish(&this->mc));
    BSG_CUDA_CALL(hb_mc_responder_del(trace_ctrl));
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
cello::program * make_program() {
    return new cello::program;
}
