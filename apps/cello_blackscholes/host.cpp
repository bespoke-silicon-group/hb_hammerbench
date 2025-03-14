#include <cello/host/cello.hpp>

class program : public cello::program {
public:
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);
        return 0;
    }
    int input() override {
        cello::program::input();
        return 0;
    }
    int output() override {
        cello::program::sync_input();
        return 0;
    }
    int check_output() override {
        cello::program::check_output();
        return 0;
    }
};

cello::program *make_program() {
    return new program();
}
