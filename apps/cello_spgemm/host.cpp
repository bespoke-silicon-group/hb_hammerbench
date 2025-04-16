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
        C_host_init = eigen_matrix(A_host.rows(), B_host.cols());
        C_host_init.setZero();
        C_host = A_host * B_host;

	bsg_pr_test_info("A x B = C\n");
	bsg_pr_test_info("A: rows = %ld, cols = %ld, nnz = %ld\n"
			 ,A_host.rows()
			 ,A_host.cols()
			 ,A_host.nonZeros());

	bsg_pr_test_info("B: rows = %ld, cols = %ld, nnz = %ld\n"
			 ,B_host.rows()
			 ,B_host.cols()
			 ,B_host.nonZeros());

	bsg_pr_test_info("C: rows = %ld, cols = %ld, nnz = %ld\n"
			 ,C_host.rows()
			 ,C_host.cols()
			 ,C_host.nonZeros());
        A = new csx_mirror(find<csx_type>("A"));
        A->init_host_from(A_host);
        
        B = new csx_mirror(find<csx_type>("B"));
        B->init_host_from(B_host);

        std::vector<partial_table> init(A_host.rows());
        C_product = new partial_table_vector_mirror(find<partial_table_vector>("C_product"));
        C_product->init_host_from(init);
        
        C = new csx_mirror(find<csx_type>("C"));
        C->init_host_from(C_host_init);
        return 0;
    }

    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(A->sync_device(jobs_in));
        BSG_CUDA_CALL(B->sync_device(jobs_in));
        BSG_CUDA_CALL(C_product->sync_device(jobs_in));
        BSG_CUDA_CALL(C->sync_device(jobs_in));
        return 0;
    }

    int output() override {
        cello::program::output();
        BSG_CUDA_CALL(C->sync_host(jobs_out));
        return 0;
    }

    int check_output() override {
        cello::program::check_output();
        eigen_matrix result = C->to_eigen();
        bool success = result.isApprox(C_host, 1e-5);        
        if (success) {
            bsg_pr_test_info("SPGEMM: PASSED\n");
        } else {
            bsg_pr_test_err("SPGEMM: FAILED\n");
            for (index_type i = 0; i < result.outerSize(); i++) {
                std::map<index_type, value_type> result_map, expect_map;

                eigen_matrix::InnerIterator result_it(result, i);
                eigen_matrix::InnerIterator expect_it(C_host, i);
                for (; result_it; ++result_it) {
                    result_map[result_it.col()] = result_it.value();
                }
                for (; expect_it; ++expect_it) {
                    expect_map[expect_it.col()] = expect_it.value();
                }
                for (auto it = result_map.begin(); it != result_map.end(); it++) {
                    auto expect_it = expect_map.find(it->first);
                    if (expect_it == expect_map.end()) {
                        bsg_pr_test_err("SPGEMM: result[%2d][%2d] = %f, but not found in expected\n",
                                        i, it->first, it->second);
                    } else if (it->second != expect_it->second) {
                        bsg_pr_test_err("SPGEMM: result[%2d][%2d] = %f, but expected %f\n",
                                        i, it->first, it->second, expect_it->second);
                    }
                }
                for (auto it = expect_map.begin(); it != expect_map.end(); it++) {
                    auto result_it = result_map.find(it->first);
                    if (result_it == result_map.end()) {
                        bsg_pr_test_err("SPGEMM: expected[%2d][%2d] = %f, but not found in result\n",
                                        i, it->first, it->second);
                    }
                }
            }
        }
        return 0;
    }
    
    csx_mirror *A, *B, *C;
    partial_table_vector_mirror *C_product;
    //list_head_vector_mirror *C_list_head;
    //count_vector_mirror *C_col_count;
    eigen_matrix A_host, B_host, C_host, C_host_init;
};

cello::program *make_program()
{
    return new program();
}
