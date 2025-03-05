#include <cello/joiner.hpp>
#include <cello/thread_id.hpp>

namespace cello
{

single_child one_child_joiner::make_child() {
    global_pointer<one_child_joiner> p = global_pointer<one_child_joiner>::onPodXY(my::pod_x(), my::pod_y(), this);
    // auto c = single_child(p);
    // bsg_print_int(my::pod_x());
    // bsg_fence();
    // bsg_print_int(my::pod_y());
    // bsg_fence();
    // bsg_print_int(p->addr().pod_x());
    // bsg_fence();
    // bsg_print_int(p->addr().pod_y());
    // bsg_fence();
    // bsg_print_int(c.parent().ref().addr().pod_x());
    // bsg_fence();
    // bsg_print_int(c.parent().ref().addr().pod_y());
    // bsg_fence();
    //return c;
    // bsg_printf("%d: make_child:            %u %u %p\n"
    //            , cello::my::id()
    //            , p.ref().addr().pod_x()
    //            , p.ref().addr().pod_y()
    //            , p.ref().addr().raw());
    return single_child(p);
}

triplet_child three_child_joiner::make_child() {
    return triplet_child(global_pointer<char>(&this->child_ready_[children_made_++]));
}

nth_child n_child_joiner::make_child() {
    return nth_child(global_pointer<n_child_joiner>::onPodXY(my::pod_x(), my::pod_y(), this));
}

}

void bsg_global_pointer::reference<cello::one_child_joiner>::increment_ready_count() {
    register cello::one_child_joiner *p = reinterpret_cast<cello::one_child_joiner*>(addr().raw());
    {
        pod_address_guard grd(addr().ext().pod_addr());
        p->increment_ready_count();
    }
}

void bsg_global_pointer::reference<cello::n_child_joiner>::increment_ready_count() {
    register cello::n_child_joiner *p = reinterpret_cast<cello::n_child_joiner*>(addr().raw());
    {
        pod_address_guard grd(addr().ext().pod_addr());
        p->increment_ready_count();
    }
}
