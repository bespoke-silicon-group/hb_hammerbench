
#ifndef STANDARD_HOST_PROGRAM
#define STANDARD_HOST_PROGRAM
#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore.h>
#include <bsg_manycore_regression.h>
#include <global_pointer/global_pointer.hpp>
#include <vector>

namespace standard
{
/**
 * @brief a host program
 */
class program
{
public:
    /**
     * @brief run the program
     */
    program();

    /**
     * @brief initialize from arguments
     */
    virtual int init(int argc, char ** argv);

    /**
     * @brief input data
     */
    virtual int input();

    /**
     * @brief sync input
     */
    int sync_input();

    /**
     * @brief run the program
     */
    virtual int run();

    /**
     * @brief output data
     */
    virtual int output();

    /**
     * @brief sync output
     */
    int sync_output();

    /**
     * @brief check output
     */
    virtual int check_output();

    /**
     * @brief finalize the program
     */
    int fini();

    /**
     * @brief execute the program
     */
    int execute(int argc, char ** argv) {
        BSG_CUDA_CALL(init(argc, argv));
        BSG_CUDA_CALL(input());
        BSG_CUDA_CALL(sync_input());
        BSG_CUDA_CALL(run());
        BSG_CUDA_CALL(output());
        BSG_CUDA_CALL(sync_output());
        BSG_CUDA_CALL(check_output());
        BSG_CUDA_CALL(fini());
        return 0;
    }

    /**
     * @brief find a symbol
     */
    hb_mc_eva_t find(const char*symbol);

    /**
     * @brief find a symbol
     */
    template <typename T>
    bsg_global_pointer::pointer<T> find(const char*symbol) {
        return bsg_global_pointer::pointer<T>(find(symbol));
    }

    /**
     * @brief pod id to coordinate
     */
    hb_mc_coordinate_t pod_id_to_coord(int pod_id) {
        return hb_mc_index_to_coordinate(pod_id, mc.mc->config.pods);
    }

    /**
     * @brief coordinate to pod id
     */
    hb_mc_pod_id_t pod_coord_to_id(hb_mc_coordinate_t coord) {
        return hb_mc_coordinate_to_index(coord, mc.mc->config.pods);
    }

    /**
     * write a value to all pods
     */
    template <typename T>
    void write_all_pods(bsg_global_pointer::pointer<T>&p, const T&v) {
        hb_mc_pod_id_t pod_id;
        hb_mc_coordinate_t save = {.x = p.pod_x(), .y = p.pod_y()};
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            hb_mc_coordinate_t pod = pod_id_to_coord(pod_id);
            p.set_pod_x(pod.x);
            p.set_pod_y(pod.y);
            *p = v;
        }
        p.set_pod_x(save.x).set_pod_y(save.y);
    }

    /**
     * alloc
     */
    int alloc(hb_mc_pod_id_t pod_id, size_t size, hb_mc_eva_t *eva) {
        BSG_CUDA_CALL(hb_mc_device_pod_malloc(&mc, pod_id, size, eva));
        return 0;
    }

    /**
     * alloc aligned
     */
    int alloc_aligned(hb_mc_pod_id_t pod_id, size_t size, size_t alignment, hb_mc_eva_t *allocated, hb_mc_eva_t *aligned) {
        BSG_CUDA_CALL(alloc(pod_id, size + alignment, allocated));
        hb_mc_eva_t rem = *allocated % alignment;
        *aligned = *allocated - rem + alignment;
        return 0;
    }

    /**
     * alloc aligned to pods cache array size
     */
    int alloc_cache_aligned(hb_mc_pod_id_t pod_id, size_t size, hb_mc_eva_t *allocated, hb_mc_eva_t *aligned) {
        size_t alignment = mc.mc->config.pod_shape.x * 2 * mc.mc->config.vcache_stripe_words * sizeof(int32_t);
        BSG_CUDA_CALL(alloc_aligned(pod_id, size, alignment, allocated, aligned));
        return 0;
    }

    virtual bool cold_cache() const { return false; }

    hb_mc_device_t mc; //!< manycore device
    hb_mc_dimension_t tg = {bsg_tiles_X, bsg_tiles_Y}; //!< tile group dimensions
    std::vector<std::vector<hb_mc_dma_htod_t>> jobs_in; //!< dma jobs
    std::vector<std::vector<hb_mc_dma_dtoh_t>> jobs_out; //!< dma jobs
    hb_mc_eva_t cfg_ptr = 0; //!< config pointer
};
}
/**
 * @brief create a new program
 */
extern "C" standard::program * make_program();

#endif
