#include <standard/host/program.hpp>
#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>

namespace standard
{

/**
 * @brief main function for a cello program
 */
int Main(int argc, char *argv[]) {
    standard::program *program = make_program();
    return program->execute(argc, argv);
}
}

declare_program_main("standard_program", standard::Main);
