#include <cello/cello.hpp>
#include <util/statics.hpp>
#include <util/test_eq.hpp>

//#define TRACE

struct test_mask {
    std::atomic<unsigned> tile;
    std::atomic<unsigned> pod;
    void update() {
        bsg_global_pointer::pod_address podaddr(0,0);
        bsg_global_pointer::pod_address_guard grd(podaddr);
        tile |= 1 << cello::my::tile_id();
        pod  |= 1 << cello::my::pod_id();
    }
};

DRAM(test_mask) delegate_scheduler_mask;

void delegate_scheduler(int *to_ids, int n) {
    using namespace cello;
    using joiner = one_child_joiner;
    unsigned expect_tile = 0;
    unsigned expect_pod = 0;
    for (int i = 0; i < n; i++) {
        int to_id = to_ids[i];
        expect_tile |= 1 << (to_id % my::num_tiles());
        expect_pod  |= 1 << (to_id / my::num_tiles());
        joiner j;
        joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
        task *t = new_task([]() {
#ifdef TRACE
            bsg_printf("delegate_scheduler: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_scheduler_mask.update();
        }, *jp);
        delegate(to_id, t);
        wait(&j);
    }
    TEST_EQ(INT, delegate_scheduler_mask.tile, expect_tile);
    TEST_EQ(INT, delegate_scheduler_mask.pod, expect_pod);
}

DRAM(test_mask) delegate_scheduler_multi_mask;

void delegate_scheduler_multi(int *to_ids, int n) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    unsigned expect_tile = 0;
    unsigned expect_pod = 0;    
    for (int i = 0; i < n; i++) {
        expect_tile |= 1 << (to_ids[i] % my::num_tiles());
        expect_pod  |= 1 << (to_ids[i] / my::num_tiles());
        task *t = new_task([]() {
#ifdef TRACE
            bsg_printf("delegate_scheduler_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_scheduler_multi_mask.update();
        }, *jp);
        delegate(to_ids[i], t);
    }
    wait(&j);
    TEST_EQ(INT, delegate_scheduler_multi_mask.tile, expect_tile);
    TEST_EQ(INT, delegate_scheduler_multi_mask.pod, expect_pod);
}

DRAM(test_mask) delegate_on_tile_mask;

void delegate_on_tile(int *to_ids, int n) {
    using namespace cello;
    unsigned expect_tile = 0;
    unsigned expect_pod = 0;
    for (int i = 0; i < n; i++) {
        expect_tile |= 1 << (to_ids[i] % my::num_tiles());
        expect_pod  |= 1 << (to_ids[i] / my::num_tiles());
        int to_id = to_ids[i];
        on_tile(to_id, []() {
#ifdef TRACE
            bsg_printf("delegate_on_tile: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_on_tile_mask.update();            
        });
    }
    TEST_EQ(INT, delegate_on_tile_mask.tile, expect_tile);
    TEST_EQ(INT, delegate_on_tile_mask.pod, expect_pod);
}

DRAM(test_mask) delegate_on_tile_multi_mask;

void delegate_on_tile_multi(int *to_tiles, int n) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    unsigned expect_tile = 0;
    unsigned expect_pod = 0;
    for (int i = 0; i < n; i++) {
        expect_tile |= 1 << (to_tiles[i] % my::num_tiles());
        expect_pod  |= 1 << (to_tiles[i] / my::num_tiles());
        on_tile(to_tiles[i], jp, []() {
#ifdef TRACE
            bsg_printf("delegate_on_tile_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_on_tile_multi_mask.update();
        });
    }
    wait(&j);
    TEST_EQ(INT, delegate_on_tile_multi_mask.tile, expect_tile);
    TEST_EQ(INT, delegate_on_tile_multi_mask.pod, expect_pod);
}

DRAM(test_mask) delegate_pod_scheduler_mask;

int delegate_pod_scheduler() {
    using namespace cello;
    using joiner = one_child_joiner;
    unsigned expect_pod = 0;    
    for (int to_pod = 0; to_pod < my::num_pods(); to_pod++) {
        joiner j;
        joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
        expect_pod |= 1 << to_pod;
        task *t = new_task([]() {
#ifdef TRACE
            bsg_printf("delegate_pod_scheduler: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_pod_scheduler_mask.update();
        }, *jp);
        delegate_pod(to_pod, t);
        wait(&j);
    }
    TEST_EQ(INT, delegate_pod_scheduler_mask.pod, expect_pod);
    return 0;
}

DRAM(test_mask) delegate_pod_scheduler_multi_mask;

int delegate_pod_scheduler_multi() {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    unsigned expect_pod = 0;
    for (int i = 0; i < my::num_pods(); i++) {
        expect_pod |= 1 << i;
        task *t = new_task([]() {
#ifdef TRACE
            bsg_printf("delegate_pod_scheduler_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_pod_scheduler_multi_mask.update();
        }, *jp);
        delegate_pod(i, t);
    }
    wait(&j);
    TEST_EQ(INT, delegate_pod_scheduler_multi_mask.pod, expect_pod);
    return 0;
}

DRAM(test_mask) delegate_on_pod_mask;

int delegate_on_pod() {
    using namespace cello;
    unsigned expect_pod = 0;
    for (int to_pod = 0; to_pod < my::num_pods(); to_pod++) {
        expect_pod |= 1 << to_pod;
        on_pod(to_pod, []() {
#ifdef TRACE
            bsg_printf("delegate_on_pod: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_on_pod_mask.update();
        });
    }
    TEST_EQ(INT, delegate_on_pod_mask.pod, expect_pod);
    return 0;
}

DRAM(test_mask) delegate_on_pod_multi_mask;

int delegate_on_pod_multi() {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    unsigned expect_pod = 0;
    for (int i = 0; i < my::num_pods(); i++) {
        expect_pod |= 1 << i;
        on_pod(i, jp, []() {
#ifdef TRACE
            bsg_printf("delegate_on_pod_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
#endif
            delegate_on_pod_multi_mask.update();
        });
    }
    wait(&j);
    TEST_EQ(INT, delegate_on_pod_multi_mask.pod, expect_pod);
    return 0;
}

int cello_main(int argc, char **argv) {
    using namespace cello;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
    
    int to_ids[] = {my::id(), my::id()+1, my::num_tiles()-1, my::num_tiles(), my::num_tiles_total()-1};

    delegate_scheduler(to_ids, ARRAY_SIZE(to_ids));
    delegate_scheduler_multi(to_ids, ARRAY_SIZE(to_ids));
    delegate_on_tile(to_ids, ARRAY_SIZE(to_ids));
    delegate_on_tile_multi(to_ids, ARRAY_SIZE(to_ids));
    delegate_pod_scheduler();
    delegate_pod_scheduler_multi();
    delegate_on_pod();
    delegate_on_pod_multi();
    return 0;
}
