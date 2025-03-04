#include <cello/cello.hpp>
#include <util/statics.hpp>

int write_back(int depth)
{
    if (depth <= 0) {
        return depth;
    }

    return write_back(depth - 1) + 1;
}

// template <>
// class bsg_global_pointer::reference<int> {
// public:
//     /**
//      * @brief base constructor
//      */
//     explicit reference(address addr): addr_(addr) {
//         //bsg_print_int(100000 + 1000*cello::my::pod_y() + addr_.pod_y());
//     }

//     /**
//      * @brief constructor from address with default extended address
//      */
//     explicit reference(const void* raw): addr_(raw) {}

//     /**
//      * @brief default constructor
//      */
//     reference() : addr_() {}

//     /**
//      * @brief copy constructor for argument passing
//      */
//     reference(const reference& other) : addr_(other.addr_) {
//         //bsg_print_int(1000000 + 1000*cello::my::pod_y() + addr().pod_y());
//     }

//     /**
//      * @brief move constructor for return values
//      */
//     reference(reference&& other) : addr_(other.addr_) {
//         //bsg_print_int(2000000 + 1000*cello::my::pod_y() + addr().pod_y());        
//     }

//     /**
//      * @brief destructor
//      */
//     ~reference() {}

//     /**
//      * @brief copy assignment operator
//      * this updates the value POINTED TO... NOT the reference object
//      */
//     reference& operator=(const reference& other) {
//         *this = (int)other;
//         return *this;
//     }

//     /**
//      * @brief move assignment operator
//      * this updates the value POINTED TO... NOT the reference object
//      */
//     reference& operator=(reference&& other) {
//         *this = (int)other;
//         return *this;
//     }

//     /**
//      * @brief assignment operator
//      * used as syntactic sugar for writes
//      * this updates the value POINTED TO... NOT the reference object
//      */
//     reference& operator=(const int& other) {
//         // problem, what if stack is in dram???
//         // maybe we make this only valid for scalar types...
//         write(other);
//         return *this;
//     }

//     /**
//      * @brief cast operator
//      * used as syntactic sugar for reads
//      * this reads the value POINTED TO... NOT the reference object
//      */
//     operator int() const {
//         return read();
//     }

//     /**
//      * @brief updates the value pointed to by the reference
//      */
//     void write(const int& other) {
//         // problem, what if stack is in dram???
//         // maybe we make this only valid for scalar types...
//         addr_.write(other);
//     }

//     /**
//      * @brief reads the value pointed to by the reference
//      */
//     int read() const {
//         return addr_.read<int>();
//     }

//     /**
//      * @brief set the pod x of the reference
//      */
//     reference& set_pod_x(unsigned x) {
//         addr().set_pod_x(x);
//         return *this;
//     }

//     /**
//      * @brief get the pod x of the reference
//      */
//     unsigned pod_x() const {
//         return addr().pod_x();
//     }

//     /**
//      * @brief set the pod y of the reference
//      */
//     reference& set_pod_y(unsigned y) {
//         addr().set_pod_y(y);
//         return *this;
//     }

//     /**
//      * @brief get the pod y of the reference
//      */
//     unsigned pod_y() const {
//         return addr().pod_y();
//     }


//     FIELD(address, addr); //!< the address information
// };

int spawn_and_write_back(int depth)
{
    using namespace cello;
    using joiner = one_child_joiner;

    if (depth <= 0) {
        return depth;
    }

    int wb = 0;

    //#define USE_POINTER
#ifdef  USE_POINTER
    bsg_global_pointer::pointer<int> wb_ptr = bsg_global_pointer::pointer<int>::onPodXY
        (my::pod_x(), my::pod_y(), (bsg_tile_group_remote_pointer<int>(my::tile_x(), my::tile_y(), &wb)));
#else
    bsg_global_pointer::pod_address pod_addr(my::pod_x(), my::pod_y());
    bsg_global_pointer::address_ext addr_ext(pod_addr);
    bsg_global_pointer::address addr(addr_ext, bsg_tile_group_remote_pointer<int>(my::tile_x(), my::tile_y(), &wb));
    bsg_global_pointer::reference<int> wb_ref(addr);
    //bsg_print_int(1000000000 + wb_ref.pod_y());
    //bsg_print_hexadecimal(wb_ref.addr().raw());
#endif
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    task *t = new_task
#ifdef USE_POINTER
        ([wb_ptr, depth]()mutable{
            *wb_ptr = spawn_and_write_back(depth - 1);
        },*jp);
#else
        ([wb_ref, depth]()mutable{
            //bsg_global_pointer::pointer<int> p(wb_ref);
            //*p = spawn_and_write_back(depth - 1);
            //wb_ref.addr().write<int>(spawn_and_write_back(depth - 1));
            //bsg_fence();
            wb_ref = spawn_and_write_back(depth - 1);
        },*jp);
#endif
    spawn(t);
    wait(jp);
    return wb + 1;
}

int cello_main(int argc, char *argv[])
{
    using namespace cello;
#if 1
    // bsg_print_int(cello::my::id());
    // bsg_print_int(cello::my::tile_x());
    // bsg_print_int(cello::my::tile_y());
    // bsg_print_int(cello::my::pod_x());
    // bsg_print_int(cello::my::pod_y());
    // bsg_print_int(cello::my::num_pods());
    bsg_fence();
    int n = 20;
    int wb = spawn_and_write_back(n);
    bsg_print_int(-wb);
    bsg_print_int(-write_back(n));
    //bsg_fence();
#endif
    return 0;
}
