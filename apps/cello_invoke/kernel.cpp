#include <cello/cello.hpp>
#include <bsg_manycore.h>
#include <atomic>
#include <util/test_eq.hpp>
#include <util/statics.hpp>


DRAM(std::atomic<int>) sched_ctr, invoke_ctr, lvalue_ctr;
DRAM(std::atomic<int>) sched_mask, invoke_mask, lvalue_mask;

void recurse_scheduler(int depth)
{
    using namespace cello;
    bsg_print_int(depth);
    if (depth == 0) {
        bsg_global_pointer::pod_address paddr;
        paddr.set_pod_x(0);
        paddr.set_pod_y(0);
        bsg_global_pointer::pod_address_guard grd(paddr);
        sched_ctr++;
        sched_mask |= (1 << my::id());
        return;
    } else {
        using joiner = one_child_joiner;
        joiner j;
        joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
        task *t = new_task([=]() { recurse_scheduler(depth - 1); }, *jp);
        spawn(t);
        recurse_scheduler(depth - 1);
        wait(jp);
    }
}

void recurse_invoke(int depth)
{
    using namespace cello;
    //bsg_print_int(depth);
    if (depth == 0) {
        invoke_ctr++;
        invoke_mask |= (1 << my::id());
        return;
    } else {
        parallel_invoke([=]() { recurse_invoke(depth - 1); },
                        [=]() { recurse_invoke(depth - 1); });
    }
}

struct recurser
{
    recurser(int depth) : depth_(depth) {}
    void operator()()
    {
        using namespace cello;
        //bsg_print_int(depth_);
        if (depth_ == 0) {
            lvalue_ctr++;
            lvalue_mask |= (1 << my::id());
            return;
        } else {
            recurser c(depth_ - 1);
            parallel_invoke(c, c);
        }
    }
    FIELD(int, depth);
};

void recurse_invoke_lvalue(int depth)
{
    recurser r(depth);
    r();
}

DRAM(std::atomic<int>) ctr4, mask4;

void recurse4(int depth)
{
    using namespace cello;
    //bsg_print_int(depth);
    if (depth == 0) {
        ctr4++;
        mask4 |= (1 << my::id());
        return;
    } else {
        parallel_invoke([=]() { recurse4(depth - 1); },
                        [=]() { recurse4(depth - 1); },
                        [=]() { recurse4(depth - 1); },
                        [=]() { recurse4(depth - 1); });
    }
}

int cello_main(int argc, char *argv[])
{
    using namespace cello;
    sched_ctr = 0;
    invoke_ctr = 0;
    lvalue_ctr = 0;
    sched_mask = 0;
    invoke_mask = 0;
    lvalue_mask = 0;
    ctr4 = 0;
    mask4 = 0;
    recurse_scheduler(3);
    //recurse_invoke(3);
    //recurse_invoke_lvalue(3);
    // check that the right number of leafs were reached
    TEST_EQ(INT, sched_ctr  ,8);
    //TEST_EQ(INT, invoke_ctr ,8);
    //TEST_EQ(INT, lvalue_ctr ,8);
    // check that more than this thread participated
    TEST_NEQ(INT, sched_mask  ,(1 << my::id()));
    //TEST_NEQ(INT, invoke_mask ,(1 << __bsg_id));
    //TEST_NEQ(INT, lvalue_mask ,(1 << __bsg_id));
    // recurse4(3);
    // TEST_EQ(INT, ctr4, 4*4*4);
    // TEST_NEQ(INT, mask4, (1 << __bsg_id));
    return 0;
}
