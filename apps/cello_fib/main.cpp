#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_regression.h>
#include <cello/config.hpp>
#include <vector>

hb_mc_device_t mc;

int fibonacci(int n)
{
    int k = 1, s0 = 0, s1 = 1;
    while (k < n)
    {
        int s = s0 + s1;
        s0 = s1;
        s1 = s;
        k++;
    }
    return s1;
}

int cello_hello_world_main(int argc, char *argv[])
{
    char  *program = argv[1];
    int fib_n = atoi(argv[2]);
    int fib_n_expected = fibonacci(fib_n);
    
    BSG_CUDA_CALL(hb_mc_device_init(&mc, "cello_fib", 0));

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

    // find symbol "fib_n" in the program
    hb_mc_program_t *prog = mc.pods[0].program;
    hb_mc_eva_t fib_n_ptr = 0, fib_n_expected_ptr = 0;
    BSG_CUDA_CALL(hb_mc_loader_symbol_to_eva(prog->bin, prog->bin_size, "fib_n", &fib_n_ptr));
    BSG_CUDA_CALL(hb_mc_loader_symbol_to_eva(prog->bin, prog->bin_size, "fib_expect", &fib_n_expected_ptr));
    
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
            jobs.push_back({fib_n_ptr, (void*)&fib_n, sizeof(int)});
            jobs.push_back({fib_n_expected_ptr, (void*)&fib_n_expected, sizeof(int)});

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

declare_program_main("cello_fib", cello_hello_world_main);
