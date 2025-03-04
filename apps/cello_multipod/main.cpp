#include <cello/host/cello.hpp>
/**
 * @brief program
 */
class program : public cello::program {
public:
    /**
     * @brief init
     */
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);
        return HB_MC_SUCCESS;
    }
    /**
     * @brief input
     */
    int input() override {
        cello::program::input();
        return HB_MC_SUCCESS;        
    }
};

cello::program *make_program() {
    return new program();
}
