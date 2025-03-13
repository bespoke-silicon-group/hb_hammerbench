#include <cello/host/cello.hpp>
#include <global_pointer/global_pointer.hpp>
#include <datastructure/vector.hpp>
#include <memory>

class program : public cello::program {
public:
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);
        vec_mirror = std::unique_ptr<datastructure::vector_host_mirror<int>>
            (new datastructure::vector_host_mirror<int>(find<datastructure::vector<int>>("vec_int")));
        vec_n = atoi(argv[2]);
        return 0;
    }

    int input() override {
        cello::program::input();
        // a vector to initialize with
        std::vector<int> vec_int;
        vec_int.resize(vec_n);
        for (int i = 0; i < vec_n; i++) {
            vec_int[i] = i;
        }
        printf("vec_n: %d\n", vec_n);
        vec_mirror->init_host_from(vec_int);
        BSG_CUDA_CALL(vec_mirror->sync_device(jobs_in));
        return 0;
    }

    int output() override {
        cello::program::output();
        BSG_CUDA_CALL(vec_mirror->sync_host(jobs_out));
        return 0;
    }

    int check_output() override {
        cello::program::check_output();
        for (int i = 0; i < vec_n; i++) {
            printf("vec_int[%3d]: %3d\n", i, vec_mirror->at(i));
        }
        return 0;
    }

    bsg_global_pointer::pointer<datastructure::vector<int>> vec_int_ptr;
    std::unique_ptr<datastructure::vector_host_mirror<int>> vec_mirror;
    int vec_n;    
};

cello::program *make_program() {
    return new program();
}
