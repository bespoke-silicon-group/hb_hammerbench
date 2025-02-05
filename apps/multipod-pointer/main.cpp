#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>

hb_mc_device_t mc;

int multipod_pointer_main(int argc, char *argv[])
{
    char *program = argv[1];
    BSG_CUDA_CALL(hb_mc_device_init(&mc, "multipod-pointer", 0));

    hb_mc_dimension_t tg = mc.mc->config.pod_shape;
    hb_mc_pod_id_t pod;
    hb_mc_device_foreach_pod_id(&mc, pod)
    {
            BSG_CUDA_CALL(hb_mc_device_pod_program_init(&mc, pod, program));
            BSG_CUDA_CALL(hb_mc_device_pod_kernel_enqueue(&mc, pod,
                                                          {1,1}, tg,
                                                          "multipod_pointer", 0, {}));            
    }

    BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&mc));

    hb_mc_device_foreach_pod_id(&mc, pod)
    {
            BSG_CUDA_CALL(hb_mc_device_pod_program_finish(&mc, pod));
    }
    BSG_CUDA_CALL(hb_mc_device_finish(&mc));
    return 0;
}

declare_program_main("multipod pointer", multipod_pointer_main);
