#include <cello/host/cello.hpp>
#include <memory>
#include "common.hpp"

class program : public cello::program
{
public:
    int init(int argc, char **argv) override {
        cello::program::init(argc, argv);
        query_ptr_ = find<input_vector>("query");
        ref_ptr_ = find<input_vector>("ref");
        output_ptr_ = find<output_vector>("output");
        query_ = std::unique_ptr<input_vector::mirror_type>(new input_vector::mirror_type(query_ptr_));
        ref_ = std::unique_ptr<input_vector::mirror_type>(new input_vector::mirror_type(ref_ptr_));
        output_ = std::unique_ptr<output_vector::mirror_type>(new output_vector::mirror_type(output_ptr_));
        query_path_ = argv[2];
        ref_path_ = argv[3];
        output_path_ = argv[4];
        return 0;
    }

    int input() override {
        cello::program::input();
        return 0;
    }

    int output() override {
        cello::program::output();
        return 0;
    }

    int check_output() override {
        cello::program::check_output();
        return 0;
    }

    std::vector<sequence> query_data_;
    std::vector<sequence> ref_data_;
    std::vector<score> output_data_;
    bsg_global_pointer::pointer<input_vector> query_ptr_;
    bsg_global_pointer::pointer<input_vector> ref_ptr_;
    bsg_global_pointer::pointer<output_vector> output_ptr_;
    std::unique_ptr<input_vector::mirror_type>  query_;
    std::unique_ptr<input_vector::mirror_type>  ref_;
    std::unique_ptr<output_vector::mirror_type> output_;
    const char *query_path_, *ref_path_, *output_path_;
};

cello::program *make_program()
{
    return new program();
}
