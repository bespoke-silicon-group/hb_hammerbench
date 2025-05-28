#ifndef CELLO_HOST_PROGRAM_HPP
#define CELLO_HOST_PROGRAM_HPP

#include <bsg_manycore_cuda.h>
#include <bsg_manycore.h>
#include <bsg_manycore_responder.h>
#include <bsg_manycore_regression.h>
#include <global_pointer/global_pointer.hpp>
#include <cello/config.hpp>
#include <cello/stats.hpp>
#include <vector>
#include <chrono>

namespace cello
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
    int collect_statistics();

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
        BSG_CUDA_CALL(collect_statistics());
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
     * do something foreach pod coordinate
     */
    template <typename F>
    int foreach_pod(F && f) {
        hb_mc_coordinate_t pod;
        foreach_coordinate(pod, zero, pods) {
            BSG_CUDA_CALL(f(pod));
        }
        return 0;
    }

    /**
     * do something foreach pod coordinate
     */
    template <typename F>
    int foreach_pod_id(F && f) {
        hb_mc_coordinate_t pod;
        foreach_coordinate(pod, zero, pods) {
            hb_mc_pod_id_t pod_id = pod_coord_to_id(pod);
            BSG_CUDA_CALL(f(pod_id));
        }
        return 0;
    }    

    hb_mc_device_t mc; //!< manycore device
    hb_mc_dimension_t tg = {bsg_tiles_X, bsg_tiles_Y}; //!< tile group dimensions
    hb_mc_dimension_t pods = {bsg_pods_X, bsg_pods_Y}; //!< pods dimensions
    hb_mc_coordinate_t zero = {0, 0}; // zero point origin
    std::vector<std::vector<hb_mc_dma_htod_t>> jobs_in; //!< dma jobs
    std::vector<std::vector<hb_mc_dma_dtoh_t>> jobs_out; //!< dma jobs
    std::vector<cello::config> cfgs; //!< configurations
    hb_mc_eva_t cfg_ptr = 0; //!< config pointer
    hb_mc_responder_t *responders;
    bool kernel_start_set = false, kernel_stop_set = false;
    std::chrono::time_point<std::chrono::steady_clock> kernel_start, kernel_stop;
};
}
/**
 * @brief create a new program
 */
extern "C" cello::program * make_program();
#endif
