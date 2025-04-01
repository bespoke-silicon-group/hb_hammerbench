#include <cello/host/cello.hpp>

class program : cello::program {
public:
    int check_output() override {
        cello::program::check_output();
        auto counter_ptr = find<int>("counter");
        if (*counter_ptr != NITERS) {
            printf("Error: expected counter=%d, actual counter=%d\n"
                   , NITERS, *counter_ptr);
        }
        auto data_ptr = find<int>("data");
        for (int i = 0; i < NITERS; i++) {
            if (data_ptr[i] != i) {
                printf("Error: expected data[%d]=%d, actual data[%d]=%d\n"
                       , i, i, i, (int)data_ptr[i]);
            }
        }
        return 0;
    }
};

cello::program *make_program() {
    return new program();
}
