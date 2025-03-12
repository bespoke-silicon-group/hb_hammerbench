#include <cello/host/cello.hpp>
#include <global_pointer/global_pointer.hpp>
#include "data.hpp"

class program : public cello::program {
public:
    int input() override {
        cello::program::input();
        data_ptr = bsg_global_pointer::pointer<data_t>(find("data"));
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, mc.mc->config.pods);
            data_ptr.set_pod_x(pod.x);
            data_ptr.set_pod_y(pod.y);
            data_ptr->a() = 1;
            data_ptr->b() = 2;
            data_ptr->c() = 3;
            data_ptr->d() = 4;
            data_ptr->i().x() = 91;
            data_ptr->i().y() = -101;
        }
        return 0;
    }
    int output() override {
        cello::program::output();
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, mc.mc->config.pods);
            printf("data[%d,%d] = { .a() = %2d, .b() = %2d, .c() = %2d, .d() = %2d }\n"
                   , pod.x
                   , pod.y
                   , (int)data_ptr->a()
                   , (int)data_ptr->b()
                   , (int)data_ptr->c()
                   , (int)data_ptr->d()
                   );
        }
        return 0;
    }
    bsg_global_pointer::pointer<data_t> data_ptr;
};

cello::program *make_program() {
    return new program();
}
