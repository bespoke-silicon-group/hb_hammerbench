#ifndef PROFILE_HPP
#define PROFILE_HPP
#ifdef HOST
#include <bsg_manycore_responder.h>
#include <chrono>
#include <fstream>
#endif

#define CELLO_MMIO_EPA 0xEB00
#define CELLO_TRACE_TOGGLE_EPA  (CELLO_MMIO_EPA+0)
#define CELLO_NS_TIMER_EPA      (CELLO_MMIO_EPA+4)

#ifndef HOST
#define cello_trace_on()                                                \
    do { *(int*)bsg_remote_ptr_io(IO_X_INDEX, CELLO_TRACE_TOGGLE_EPA) = 1; bsg_fence(); } while (0)
#define cello_trace_off()                                               \
    do { *(int*)bsg_remote_ptr_io(IO_X_INDEX, CELLO_TRACE_TOGGLE_EPA) = 0; bsg_fence(); } while (0)
#define cello_timer_start()                                             \
    do { *(int*)bsg_remote_ptr_io(IO_X_INDEX, CELLO_NS_TIMER_EPA) = 0; bsg_fence(); } while (0)
#define cello_timer_stop()                                              \
    do { *(int*)bsg_remote_ptr_io(IO_X_INDEX, CELLO_NS_TIMER_EPA) = 1; bsg_fence(); } while (0)
#endif

#ifdef HOST
static hb_mc_request_packet_id_t trace_ids [] = {
    RQST_ID(RQST_ID_ANY_X, RQST_ID_ANY_Y, RQST_ID_ADDR( CELLO_TRACE_TOGGLE_EPA )),
    RQST_ID(RQST_ID_ANY_X, RQST_ID_ANY_Y, RQST_ID_ADDR( CELLO_NS_TIMER_EPA )),
    { /* sentinal */ }
};

static int resp_init(hb_mc_responder_t *responder, hb_mc_manycore_t *mc)
{
    return 0;
}

static int resp_quit(hb_mc_responder_t *responder, hb_mc_manycore_t *mc)
{
    return 0;
}

struct program {
    bool kernel_start_set = false;
    bool kernel_stop_set = false;
    std::chrono::time_point<std::chrono::steady_clock> kernel_start, kernel_stop;
};

static int resp_respond(hb_mc_responder_t *responder, hb_mc_manycore_t *mc, const hb_mc_request_packet_t *rqst)
{
    hb_mc_epa_t epa = hb_mc_request_packet_get_epa(rqst);
    int s = hb_mc_request_packet_get_data(rqst);
    program *prog = static_cast<program*>(responder->responder_data);
    if (epa == CELLO_TRACE_TOGGLE_EPA) {
        if (s == 1) {
            BSG_CUDA_CALL(hb_mc_manycore_trace_enable(mc));
        } else if (s == 0) {
            BSG_CUDA_CALL(hb_mc_manycore_trace_disable(mc));
        }
    } else if (epa == CELLO_NS_TIMER_EPA) {
        // set timer
        if (s == 0) {
            prog->kernel_start_set = true;
            prog->kernel_start = std::chrono::steady_clock::now();
        } else {
            prog->kernel_stop_set = true;
            prog->kernel_stop = std::chrono::steady_clock::now();
        }
    }
    return 0;
}

#define HOST_PROFILE_PROLOGUE                                            \
    hb_mc_responder responder  ("user_responder", trace_ids, resp_init, resp_quit, resp_respond); \
    program prog {};                                                    \
    responder.responder_data = (void*)&prog;                            \
    BSG_CUDA_CALL(hb_mc_responder_add(&responder));

#define HOST_PROFILE_EPILOGUE                                           \
    if (prog.kernel_start_set and prog.kernel_stop_set) {               \
        std::ofstream ns_log("kernel_ns.log");                          \
        const std::chrono::duration<long long, std::nano> ns{prog.kernel_stop - prog.kernel_start}; \
        ns_log << ns.count() << std::endl;                              \
    }                                                                   \
    BSG_CUDA_CALL(hb_mc_responder_del(&responder));

#endif
#endif
