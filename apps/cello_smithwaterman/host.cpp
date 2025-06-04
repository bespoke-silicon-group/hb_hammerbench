#include <cello/host/cello.hpp>
#include <memory>
#include "common.hpp"

class program : public cello::program
{
public:
    void read_seq(const char* filename, uint8_t* seq, int num_seq) {

        FILE* file = fopen(filename, "r");
        for (int i = 0; i < num_seq; i++) {
            char temp_seq[64];
            fscanf(file, "%s", temp_seq); // skip line number;
            fscanf(file, "%s", temp_seq);
            for (int j = 0; j < 32; j++) {
                    seq[(32*i)+j] = temp_seq[j]; 
            }
        }
        fclose(file);
    }


    void read_output(const char* filename, int* output, int num_seq)
    {
        FILE* file = fopen(filename, "r");
        for (int i = 0; i < num_seq; i++) {
            int score;
            fscanf(file, "%d", &score);
            output[i] = score;
        } 
        fclose(file);
    }
    
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
        printf("%-15s %s\n" ,"query_path:"  ,query_path_);
        printf("%-15s %s\n" ,"ref_path:"    ,ref_path_);
        printf("%-15s %s\n" ,"output_path:" ,output_path_);
        query_data_.resize(NUM_SEQ);
        ref_data_.resize(NUM_SEQ);
        output_data_.resize(NUM_SEQ, {0});
        output_->init_host_from(output_data_); // init output with zeroed data
        read_seq(query_path_, reinterpret_cast<uint8_t*>(query_data_.data()), query_data_.size());
        read_seq(ref_path_, reinterpret_cast<uint8_t*>(ref_data_.data()), ref_data_.size());
        read_output(output_path_, output_data_.data(), output_data_.size());
        query_->init_host_from(query_data_);
        ref_->init_host_from(ref_data_);
        return 0;
    }

    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(query_->sync_device(jobs_in));
        BSG_CUDA_CALL(ref_->sync_device(jobs_in));
        BSG_CUDA_CALL(output_->sync_device(jobs_in));
        return 0;
    }

    int output() override {
        cello::program::output();
        BSG_CUDA_CALL(output_->sync_host(jobs_out));
        return 0;
    }

    int check_output() override {
        cello::program::check_output();
        for (int i = 0; i < NUM_SEQ; i++) {
            score actual = output_->at(i);
            score expected = output_data_[i];
            // the output file we check against the first 512
            if (actual != expected && i < 512) {
                printf("Mismatch at index %d: expected %d, got %d\n", i, expected, actual);
            }
        }
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
