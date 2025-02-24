#include <cello/host/cello.hpp>
#include <bsg_manycore_loader.h>

class fib_program : public cello::program
{
public:
    fib_program() : cello::program() {}
    int init(int argc, char **argv) override;
    int input() override;
    int fibonnaci(int n)
    {
        int a = 0, b = 1, c;
        for (int i = 2; i <= n; i++)
        {
            c = a + b;
            a = b;
            b = c;
        }
        return n > 0 ? b : a;
    }
    int fib_n = 0;
    int fib_expect = 0;
};

int fib_program::init(int argc, char *argv[])
{
    BSG_CUDA_CALL(cello::program::init(argc, argv));
    this->fib_n = atoi(argv[2]);
    this->fib_expect = fibonnaci(this->fib_n);
    printf("FIBONACCI(%d) = %d\n", this->fib_n, this->fib_expect);
    return HB_MC_SUCCESS;
}

int fib_program::input()
{
    BSG_CUDA_CALL(cello::program::input());
    hb_mc_pod_id_t pod_id;
    hb_mc_eva_t fib_in_ptr, fib_expect_ptr;
    fib_in_ptr = find("fib_n");
    fib_expect_ptr = find("fib_expect");
    hb_mc_device_foreach_pod_id(&this->mc, pod_id)
    {
        jobs_in[pod_id].push_back({
                fib_in_ptr,
                (void*)&this->fib_n,
                sizeof(this->fib_n)
            });
        jobs_in[pod_id].push_back({
                fib_expect_ptr,
                (void*)&this->fib_expect,
                sizeof(this->fib_expect)
            });
    }
    return HB_MC_SUCCESS;
}

cello::program* make_program()
{
    return new fib_program;
}
