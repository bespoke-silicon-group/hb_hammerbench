#include <cello/host/cello.hpp>
#include <unsupported/Eigen/SparseExtra>
#include "common.hpp"

class program : public cello::program {
public:
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);
        std::string filename = argv[2];
        Eigen::loadMarket(matrix, filename);
        csx = new csx_type::mirror_type(find<csx_type>("matrix"));
        csx->init_host_from(matrix);
        return 0;
    }
    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(csx->sync_device(jobs_in));
        return 0;
    }
    int output() override {
        cello::program::output();
        BSG_CUDA_CALL(csx->sync_host(jobs_out));
        return 0;
    }
    int check_output() override {
        cello::program::check_output();
        return 0;
    }
    csx_type::mirror_type *csx;
    eigen_matrix matrix;
};

cello::program *make_program() {
    return new program();
}
