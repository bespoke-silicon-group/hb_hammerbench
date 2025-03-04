#include <cello/host/cello.hpp>
#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>
#include <cello/config.hpp>
#include <vector>

namespace cello
{

static hb_mc_device_t mc;

/**
 * @brief main function for a cello program
 */
int Main(int argc, char *argv[]) {
    cello::program *program = make_program();
    return program->execute(argc, argv);
}
}

declare_program_main("cello_prorgram", cello::Main);
