#include <cello/cello.hpp>

void delegate_scheduler(int to_id) {
    using namespace cello;
    using joiner = one_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    task *t = new_task([]() {
        bsg_printf("delegate_scheduler: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                   , my::id()
                   , my::pod_x()
                   , my::pod_y()
                   , my::tile_x()
                   , my::tile_y()
                   );
    }, *jp);
    delegate(to_id, t);
    wait(&j);
}

void delegate_scheduler_multi(int *to_ids, int n) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    for (int i = 0; i < n; i++) {
        task *t = new_task([i]() {
            bsg_printf("delegate_scheduler_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
        }, *jp);
        delegate(to_ids[i], t);
    }
    wait(&j);
}

void delegate_on_tile(int to_tile) {
    using namespace cello;
    on_tile(to_tile, []() {
        bsg_printf("delegate_on_tile: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                   , my::id()
                   , my::pod_x()
                   , my::pod_y()
                   , my::tile_x()
                   , my::tile_y()
                   );
    });
}


void delegate_on_tile_multi(int *to_tiles, int n) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    for (int i = 0; i < n; i++) {
        on_tile(to_tiles[i], jp, []() {
            bsg_printf("delegate_on_tile_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
        });
    }
    wait(&j);
}

int delegate_pod_scheduler(int to_pod) {
    using namespace cello;
    using joiner = one_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    task *t = new_task([]() {
        bsg_printf("delegate_pod_scheduler: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                   , my::id()
                   , my::pod_x()
                   , my::pod_y()
                   , my::tile_x()
                   , my::tile_y()
                   );
    }, *jp);
    delegate_pod(to_pod, t);
    wait(&j);
    return 0;
}

int delegate_pod_scheduler_multi() {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    for (int i = 0; i < my::num_pods(); i++) {
        task *t = new_task([]() {
            bsg_printf("delegate_pod_scheduler_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
        }, *jp);
        delegate_pod(i, t);
    }
    wait(&j);
    return 0;
}

void delegate_on_pod(int to_pod) {
    using namespace cello;
    on_pod(to_pod, []() {
        bsg_printf("delegate_on_pod: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                   , my::id()
                   , my::pod_x()
                   , my::pod_y()
                   , my::tile_x()
                   , my::tile_y()
                   );
    });
}

void delegate_on_pod_multi() {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    for (int i = 0; i < my::num_pods(); i++) {
        on_pod(i, jp, []() {
            bsg_printf("delegate_on_pod_multi: tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                       , my::id()
                       , my::pod_x()
                       , my::pod_y()
                       , my::tile_x()
                       , my::tile_y()
                       );
        });
    }
    wait(&j);
}

int cello_main(int argc, char **argv) {
    using namespace cello;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
    
    int to_ids[] = {my::id(), my::id()+1, my::num_tiles()-1, my::num_tiles(), my::num_tiles_total()-1};

    for (int i = 0; i < ARRAY_SIZE(to_ids); i++) {
        delegate_scheduler(to_ids[i]);
    }

    delegate_scheduler_multi(to_ids, ARRAY_SIZE(to_ids));

    for (int i = 0; i < ARRAY_SIZE(to_ids); i++) {
        delegate_on_tile(to_ids[i]);
    }

    delegate_on_tile_multi(to_ids, ARRAY_SIZE(to_ids));

    for (int i = 0; i < my::num_pods(); i++) {
        delegate_pod_scheduler(i);
    }

    delegate_pod_scheduler_multi();

    for (int i = 0; i < my::num_pods(); i++) {
        delegate_on_pod(i);
    }

    delegate_on_pod_multi();
    return 0;
}
