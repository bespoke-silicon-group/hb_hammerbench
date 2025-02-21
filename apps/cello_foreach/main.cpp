#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>
#include <cello/config.hpp>
#include <vector>

hb_mc_device_t mc;

int cello_hello_world_main(int argc, char *argv[])
{
    char *program = argv[1];
    BSG_CUDA_CALL(hb_mc_device_init(&mc, "cello_hello_world", 0));

    //hb_mc_dimension_t tg = mc.mc->config.pod_shape;
    hb_mc_dimension_t tg = {bsg_tiles_X, bsg_tiles_Y};
    hb_mc_pod_id_t pod_id;
    hb_mc_device_foreach_pod_id(&mc, pod_id)
    {
            hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, mc.mc->config.pod_shape);
            std::vector<uint32_t> argv = {pod.x, pod.y};
            BSG_CUDA_CALL(hb_mc_device_pod_program_init(&mc, pod_id, program));
            BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, pod_id,
                                                          {1,1}, tg,
                                                          "setup", argv.size(), argv.data()));
    }

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));
    
    hb_mc_device_foreach_pod_id(&mc, pod_id)
    {
            hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, mc.mc->config.pod_shape);

            cello::config cfg;
            cfg.dram_buffer_size() = 16*1024*1024;
            BSG_CUDA_CALL(hb_mc_device_pod_malloc(&mc, pod_id, cfg.dram_buffer_size(), &cfg.dram_buffer()));
            
            hb_mc_eva_t cfg_ptr;
            BSG_CUDA_CALL(hb_mc_device_pod_malloc(&mc, pod_id, sizeof(cello::config), &cfg_ptr));

            std::vector<hb_mc_dma_htod_t> jobs;
            jobs.push_back({cfg_ptr, (void*)&cfg, sizeof(cello::config)});

            BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&mc, jobs.data(), jobs.size()));

            std::vector<uint32_t> argv = {cfg_ptr};
            BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, pod_id,
                                                          {1,1}, tg,
                                                          "cello_start", argv.size(), argv.data()));
    }

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));
    
    hb_mc_device_foreach_pod_id(&mc, pod_id)
    {
            BSG_CUDA_CALL(hb_mc_device_pod_program_finish(&mc, pod_id));
    }

    BSG_CUDA_CALL(hb_mc_device_finish(&mc));
    return 0;
}

declare_program_main("cello_hello_world", cello_hello_world_main);
