#include <cello/host/cello.hpp>
#include <unsupported/Eigen/SparseExtra>
#include "common.hpp"

class program : public cello::program {
public:
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);

        // Load the matrix from the file
        std::string filename = argv[2];
        Eigen::loadMarket(matrix, filename);
        csx = new csx_type::mirror_type(find<csx_type>("matrix"));
        csx->init_host_from(matrix);

        // Initialize the outer_sums vector with zeros
        std::vector<csx_type::value_type> zeros(matrix.rows(), 0);
        outer_sums = new vector_type::mirror_type(find<vector_type>("outer_sums"));
        outer_sums->init_host_from(zeros);
        return 0;
    }
    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(csx->sync_device(jobs_in));
        BSG_CUDA_CALL(outer_sums->sync_device(jobs_in));
        return 0;
    }
    int output() override {
        cello::program::output();
        BSG_CUDA_CALL(csx->sync_host(jobs_out));
        BSG_CUDA_CALL(outer_sums->sync_host(jobs_out));
        return 0;
    }
    int check_output() override {
        cello::program::check_output();
        Eigen::VectorXf v = Eigen::VectorXf::Constant(matrix.rows(), 1);
        Eigen::VectorXf result = matrix * v;
        float error = 0;
        for (size_t i = 0; i < outer_sums->size; i++) {
            float expected = result(i);
            float actual = outer_sums->at(i);
            // bsg_pr_info("%3zu: expected = %1.6f, actual = %1.6f\n"
            //             , i
            //             , expected
            //             , actual
            //             );
            error += std::abs(expected - actual);
        }
        const float threshold = 1e-6;
        if (error > threshold) {
            bsg_pr_info("FAIL: Error = %1.6f > %1.6f\n", error, threshold);
        } else {
            bsg_pr_info("PASS: Error = %1.6f <= %1.6f\n", error, threshold);
        }
        return 0;
    }
    csx_type::mirror_type *csx;
    vector_type::mirror_type *outer_sums;
    eigen_matrix matrix;
};

cello::program *make_program() {
    return new program();
}
