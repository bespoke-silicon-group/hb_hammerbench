#include <cello/host/cello.hpp>
#include <global_pointer/global_pointer.hpp>
#include <datastructure/vector.hpp>

class program : public cello::program {
public:
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);
        vec_int_ptr = find<datastructure::vector<int>>("vec_int");
        vec_n = atoi(argv[2]);
        // a vector to initialize with
        vec_int.resize(vec_n);
        for (int i = 0; i < vec_n; i++) {
            vec_int[i] = i;
        }
        printf("vec_n: %d\n", vec_n);
        return 0;
    }

    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(vec_init.init(vec_int_ptr, vec_int, &mc, jobs_in));
        return 0;
    }

    int output() override {
        cello::program::output();
        return 0;
    }

    bsg_global_pointer::pointer<datastructure::vector<int>> vec_int_ptr;
    datastructure::vector_initializer<int> vec_init;
    int vec_n;
    std::vector<int> vec_int;
    std::vector<std::vector<int>> vec_int_jobs_in;
};

cello::program *make_program() {
    return new program();
}
