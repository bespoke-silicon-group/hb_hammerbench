#include <cello/host/cello.hpp>
#include "common.hpp"
#include <unsupported/Eigen/SparseExtra>

using eigen_matrix = typename csx_mirror::eigen_sparse_matrix_type;

class program : public cello::program
{
    int init(int argc, char *argv[]) override {
        BSG_CUDA_CALL(cello::program::init(argc, argv));
        std::string mtx_a_path = argv[2];
        std::string mtx_b_path = argv[3];
        bsg_pr_test_info
            ("SPGEMM:\n"
             "  mtx_a_path: %s\n"
             "  mtx_b_path: %s\n",
             mtx_a_path.c_str(),
             mtx_b_path.c_str()
             );

        Eigen::loadMarket(A_host, mtx_a_path);
        Eigen::loadMarket(B_host, mtx_b_path);

        // take upper left block to fit and make a valid product
        index_type k = std::min(A_host.cols(), B_host.rows());
        A_host = A_host.block(0, 0, A_host.rows(), k);
        B_host = B_host.block(0, 0, k, B_host.cols());

        A = new csx_mirror(find<csx_type>("A"));
        A->init_host_from(A_host);
        
        B = new csx_mirror(find<csx_type>("B"));
        B->init_host_from(B_host);

        std::vector<HBListNodePtr> C_list_head_data(A->rows);
        std::fill(C_list_head_data.begin(), C_list_head_data.end(), 0);
        C_list_head
            = new list_head_vector_mirror(find<list_head_vector>("C_list_head"));
        C_list_head->init_host_from(C_list_head_data);
        
        return 0;
    }

    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(A->sync_device(jobs_in));
        BSG_CUDA_CALL(B->sync_device(jobs_in));
        BSG_CUDA_CALL(C_list_head->sync_device(jobs_in));
        return 0;
    }

    csx_mirror *A, *B, *C;
    list_head_vector_mirror *C_list_head;
    eigen_matrix A_host, B_host, C_host;
};

cello::program *make_program()
{
    return new program();
}
