#include <cello/host/cello.hpp>
#include <cstring>
#include <memory>
#include "bs.hpp"
#include "common.hpp"
//#define TRACE
class program : public cello::program {
public:
    /**
     * @brief read the input from a file
     */
    void read_input(const char* input_path, OptionData* options, int num_option) {
        FILE* file = fopen(input_path, "r");
        if (!file) {
            fprintf(stderr, "Error: Unable to open file %s\n", input_path);
            exit(1);
        }

        int rv;
        int numOptions;

        // read first line;
        fscanf(file, "%i", &numOptions);

        // read option data;
        for (int i = 0; i < num_option; i++) {
            float s, strike, r, divq, v, t;
            float unused0, unused1;
            int unused2;
            fscanf(file, "%f %f %f %f %f %f %c %f %f",
                   &s, &strike, &r, &divq, &v, &t,
                   &unused2, &unused0, &unused1);
            options[i].s = s;
            options[i].strike = strike;
            options[i].r = r;
            options[i].v = v;
            options[i].t = t;
            options[i].call = 0.0f;
            options[i].put = 0.0f;
            options[i].unused = 0.0f;
            options[i].completed = 0;
        }

        fclose(file);
    }
    
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);
        options_data_ptr = find<vector_type>("data");
        options_data = std::unique_ptr<vector_type::mirror_type>(new vector_type::mirror_type(options_data_ptr));

        num_options = atoi(argv[2]);
        input_path = argv[3];
        printf("num_options: %d\n", num_options);
        printf("input_path: %s\n", input_path);

        options.resize(num_options);
        std::memset((char*)options.data(), 0, num_options * sizeof(OptionData));
        read_input(input_path, options.data(), num_options);

        options_data->init_host_from(options);
        return 0;
    }

    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(options_data->sync_device(jobs_in));
        return 0;
    }

    int output() override {
        cello::program::output();
        BSG_CUDA_CALL(options_data->sync_host(jobs_out));
        return 0;
    }

    int check_output() override {
        cello::program::check_output();
        for (int i = 0; i < num_options; i++) {
            BlkSchlsEqEuroNoDiv(&options[i]);
        }
        bsg_global_pointer::pointer<OptionData> output = find<OptionData>("output");
        //printf("output.completed = %d\n", (int)output->COMPLETED());
#if 0
        for (int i = 0; i < num_options; i++) {
            OptionData actual = options_data_ptr->at(i);
            OptionData expected = options[i];
            printf("%d: at %x: "
                   "actual:   {s=%1.6f, strike=%1.6f, r=%1.6f, v=%1.6f, t=%1.6f, call=%1.6f, put=%1.6f, completed = %d}, "
                   "expected: {s=%1.6f, strike=%1.6f, r=%1.6f, v=%1.6f, t=%1.6f, call=%1.6f, put=%1.6f}\n"
                   , i
                   , options_data_ptr->at(i).addr().raw()
                   , actual.s
                   , actual.strike
                   , actual.r
                   , actual.v
                   , actual.t
                   , actual.call
                   , actual.put
                   , actual.completed
                   , expected.s
                   , expected.strike
                   , expected.r
                   , expected.v
                   , expected.t
                   , expected.call
                   , expected.put
                   );

        }
#endif        
        float err = 0.0f;
        for (int i = 0; i < num_options; i++) {
            float actual_call = options_data->at(i).call;
            float expected_call = options[i].call;
#ifdef TRACE
            printf("call %3d: actual=%2.6f, expected=%02.6f\n", i, actual_call, expected_call);
#endif
            float diff = actual_call - expected_call;
            err += (diff*diff);
            // Put;
            float actual_put = options_data->at(i).put;
            float expected_put = options[i].put;
#ifdef TRACE
            printf("put  %3d: actual=%2.6f, expected=%02.6f\n", i, actual_put, expected_put);
#endif
            diff = actual_put - expected_put;
            err += (diff*diff);
        }
        float threshold = 0.01f;
        if (err > threshold) {
            printf("Blackscholes: FAIL: err(%f) > %f\n", err, threshold);
        } else {
            printf("Blackscholes: PASS: err(%f) < %f\n", err, threshold);
        }
        return 0;
    }

    std::vector<OptionData> options;
    bsg_global_pointer::pointer<vector_type> options_data_ptr;
    std::unique_ptr<vector_type::mirror_type> options_data;
    int num_options = 0;
    char *input_path = nullptr;
};

cello::program *make_program() {
    return new program();
}
