#include <vector>
#include "bsg_manycore_cuda.h"
#include "bsg_manycore_regression.h"

int program_main(int argc, char *argv[])
{
    hb_mc_device_t mc;
    BSG_CUDA_CALL(hb_mc_device_init(&mc, "lock", HB_MC_DEVICE_ID));

    hb_mc_pod_id_t pod_id;
    hb_mc_device_foreach_pod_id(&mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_pod_program_init(&mc, pod_id, argv[1]));
    }

    // init
    hb_mc_device_foreach_pod_id(&mc, pod_id)
    {
        hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, mc.mc->config.pods);
        std::vector<uint32_t> kargv = {pod.x, pod.y};
        BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, pod_id, {1,1}, {bsg_tiles_X,bsg_tiles_Y}, "init", kargv.size(), kargv.data()));
    }
    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));

#define TEST(name)                                                      \
    do {                                                                \
        hb_mc_device_foreach_pod_id(&mc, pod_id)                        \
            {                                                           \
                BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, pod_id, {1,1}, {bsg_tiles_X,bsg_tiles_Y}, #name, 0, nullptr)); \
            }                                                           \
        BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));          \
        hb_mc_device_foreach_pod_id(&mc, pod_id)                        \
            {                                                           \
                BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, pod_id, {1,1}, {bsg_tiles_X,bsg_tiles_Y}, #name"_check", 0, nullptr)); \
            }                                                           \
        BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));          \
    } while(0)

    // lock
    TEST(lock);
    TEST(tile_lock);

    hb_mc_device_foreach_pod_id(&mc, pod_id)
    {
        BSG_CUDA_CALL(hb_mc_device_pod_program_finish(&mc, pod_id));
    }

    BSG_CUDA_CALL(hb_mc_device_finish(&mc));

    return 0;
}

declare_program_main("lock", program_main);
