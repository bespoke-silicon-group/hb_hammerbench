#ifndef CELLO_HOST_PROGRAM_HPP
#define CELLO_HOST_PROGRAM_HPP

#include <bsg_manycore_cuda.h>
#include <bsg_manycore.h>
#include <bsg_manycore_regression.h>
#include <global_pointer/global_pointer.hpp>
#include <cello/config.hpp>
#include <vector>
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
    int run();

    /**
     * @brief output data
     */
    virtual int output();

    /**
     * @brief sync output
     */
    int sync_output();

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
        BSG_CUDA_CALL(sync_output());
        BSG_CUDA_CALL(output());
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

    hb_mc_device_t mc; //!< manycore device
    hb_mc_dimension_t tg = {bsg_tiles_X, bsg_tiles_Y}; //!< tile group dimensions
    std::vector<std::vector<hb_mc_dma_htod_t>> jobs_in; //!< dma jobs
    std::vector<std::vector<hb_mc_dma_dtoh_t>> jobs_out; //!< dma jobs
    std::vector<cello::config> cfgs; //!< configurations
    hb_mc_eva_t cfg_ptr = 0; //!< config pointer
};
}
/**
 * @brief create a new program
 */
extern "C" cello::program * make_program();
#endif
