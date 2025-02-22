#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>
#include <vector>

hb_mc_device_t mc;

int multipod_pointer_main(int argc, char *argv[])
{
    char *program = argv[1];
    BSG_CUDA_CALL(hb_mc_device_init(&mc, "multipod-pointer", 0));

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

    BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, 1,
                                                  {1,1}, tg,
                                                  "multipod_pointer_t0", 0, {}));

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));

    BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, 1,
                                                  {1,1}, tg,
                                                  "multipod_pointer_t1", 0, {}));

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));

    BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, 1,
                                                  {1,1}, tg,
                                                  "multipod_pointer_t2", 0, {}));

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));

    BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, 1,
                                                  {1,1}, tg,
                                                  "multipod_pointer_t3", 0, {}));

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));

    BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, 1,
                                                  {1,1}, tg,
                                                  "multipod_pointer_t4", 0, {}));

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));

    hb_mc_device_foreach_pod_id(&mc, pod_id)
    {
            BSG_CUDA_CALL(hb_mc_device_pod_program_finish(&mc, pod_id));
    }

    BSG_CUDA_CALL(hb_mc_device_finish(&mc));
    return 0;
}

declare_program_main("multipod pointer", multipod_pointer_main);
