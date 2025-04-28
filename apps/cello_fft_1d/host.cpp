#include <cello/host/cello.hpp>
#include <unsupported/Eigen/FFT>
#include <iostream>
#include <cmath>
#include "common.hpp"

class program : public cello::program {
    int init(int argc, char *argv[]) override {
        BSG_CUDA_CALL(cello::program::init(argc, argv));
        n_points = atoi(argv[2]);
        bsg_pr_test_info("n_points = %d\n", n_points);
        if (n_points < 2) {
            bsg_pr_test_info("Error: requires at least 2-points of unity\n");
            return HB_MC_FAIL;
        }
        d_points_in = new point_vector::mirror(find<point_vector>("points_in"));
        d_points_out = new point_vector::mirror(find<point_vector>("points_out"));
        h_points_in.resize(n_points);
        BSG_CUDA_CALL(d_points_out->init_host_from(h_points_in));
        h_points_in[1] = FP32Complex{1.0, 0.0};
        BSG_CUDA_CALL(d_points_in->init_host_from(h_points_in));
        return 0;
    }
    int input() override {
        BSG_CUDA_CALL(cello::program::input());
        BSG_CUDA_CALL(d_points_in->sync_device(jobs_in));
        BSG_CUDA_CALL(d_points_out->sync_device(jobs_in));
        return 0;
    }
    int output() override {
        BSG_CUDA_CALL(cello::program::output());
        BSG_CUDA_CALL(d_points_in->sync_host(jobs_out));
        BSG_CUDA_CALL(d_points_out->sync_host(jobs_out));
        return 0;
    }
    int check_output() override {
        BSG_CUDA_CALL(cello::program::check_output());
        fft.fwd(h_points_out, h_points_in);
        for (size_t i = 0; i < h_points_out.size(); i++) {
            bsg_pr_test_info("points[%3d] = %+1.4f + i%+1.4f\n"
                             ,i
                             ,h_points_out[i].real()
                             ,h_points_out[i].imag()
                             );
        }
        return 0;
    }
    int n_points = 0;
    point_vector::mirror *d_points_in = nullptr;
    point_vector::mirror *d_points_out = nullptr;    
    std::vector<FP32Complex> h_points_in;
    std::vector<FP32Complex> h_points_out;
    Eigen::FFT<float> fft;
};


cello::program *make_program() {
    return new program();
}
