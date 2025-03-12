#ifndef GLOBAL_POINTER_POD_ADDRESS_HPP
#define GLOBAL_POINTER_POD_ADDRESS_HPP
#include <bitmanip/bitmanip.hpp>
#include <stdint.h>
#ifdef HOST
#include <global_pointer/host/device.hpp>
#endif
namespace bsg_global_pointer
{
#ifndef HOST
/**
 * need to be defined at compile time
 */
#ifndef BSG_COORD_X_WIDTH
#error "BSG_COORD_X_WIDTH is not defined"
#endif
#ifndef BSG_COORD_Y_WIDTH
#error "BSG_COORD_Y_WIDTH is not defined"
#endif
#ifndef BSG_POD_TILES_X
#error "BSG_POD_X is not defined"
#endif
#ifndef BSG_POD_TILES_Y
#error "BSG_POD_Y is not defined"
#endif
#ifndef BSG_PODS_X
#error "BSG_PODS_X is not defined"
#endif
#ifndef BSG_PODS_Y
#error "BSG_PODS_Y is not defined"
#endif

/**
 * @brief pod address class
 */
class pod_address
{
public:
    static constexpr unsigned tile_x_width = bitmanip::bits<BSG_POD_TILES_X-1>();
    static constexpr unsigned tile_y_width = bitmanip::bits<BSG_POD_TILES_Y-1>();
    static constexpr unsigned pod_x_width = BSG_COORD_X_WIDTH - tile_x_width;
    static constexpr unsigned pod_y_width = BSG_COORD_Y_WIDTH - tile_y_width;
    static constexpr unsigned bits = pod_x_width + pod_y_width;

    static pod_address readPodAddrCSR() {
        unsigned raw;
        asm volatile ("csrr %0, 0x360" : "=r"(raw) :: "memory");
        pod_address addr(raw);
        return addr;
    }

    static void writePodAddrCSR(pod_address addr) {
        unsigned raw = addr.raw_;
        asm volatile ("fence; "
                      "csrw 0x360, %0"
                      :: "r"(raw) : "memory");
        return;
    }

    pod_address(unsigned raw)
        : raw_(raw) {
    }

    pod_address(unsigned x, unsigned y) : raw_(0) {
        set_pod_x(x);
        set_pod_y(y);
    }

    pod_address()
        : raw_(0) {
        *this = readPodAddrCSR();
    }

    pod_address(const pod_address & other)
        : raw_(other.raw_) {
    }

    pod_address(pod_address && other)
        : raw_(other.raw_) {
    }

    pod_address & operator=(const pod_address & other) {
        raw_ = other.raw_;
        return *this;
    }

    pod_address & operator=(pod_address && other) {
        raw_ = other.raw_;
        return *this;
    }
    
    unsigned pod_x() const {
        return phys_pod_x()-1;
    }
    unsigned pod_y() const {
        return (phys_pod_y()-1)>>1;
    }
    unsigned phys_pod_x() const {
        return px_;
    }
    unsigned phys_pod_y() const {
        return py_;
    }
    pod_address & set_phys_pod_x(unsigned x) {
        px_ = x;
        return *this;
    }
    pod_address & set_phys_pod_y(unsigned y) {
        py_ = y;
        return *this;
    }
    pod_address & set_pod_x(unsigned x) {
        return set_phys_pod_x(x+1);
    }
    pod_address & set_pod_y(unsigned y) {
        return set_phys_pod_y((y<<1)+1);
    }

    bool operator==(const pod_address & other) const {
        return raw_ == other.raw_;
    }

    union {
        unsigned short raw_; //!< raw pod address
        struct {
            unsigned short px_ : pod_x_width; //!< physical pod x
            unsigned short py_ : pod_y_width; //!< physical pod y
        };
    };
};
#else

/**
 * @brief pod address class
 */
class pod_address
{
public:
    pod_address(hb_mc_coordinate_t coord)
        : pod_(coord) {
    }

    pod_address(unsigned x, unsigned y) {
        set_pod_x(x);
        set_pod_y(y);
    }

    pod_address() {}

    pod_address(const pod_address & other)
        : pod_(other.pod_) {
    }

    pod_address(pod_address && other)
        : pod_(other.pod_) {
    }

    pod_address & operator=(const pod_address & other) {
        pod_ = other.pod_;
        return *this;
    }

    pod_address & operator=(pod_address && other) {
        pod_ = other.pod_;
        return *this;
    }

    unsigned pod_x() const {
        return pod_.x;
    }

    unsigned pod_y() const {
        return pod_.y;
    }

    pod_address & set_pod_x(unsigned x) {
        pod_.x = x;
        return *this;
    }

    pod_address & set_pod_y(unsigned y) {
        pod_.y = y;
        return *this;
    }

    bool operator==(const pod_address & other) const {
        return pod_.x == other.pod_.x && pod_.y == other.pod_.y;
    }

    hb_mc_coordinate_t pod_ = {0,0}; //!< pod coordinate
};
#endif

/**
 * @brief
 * sets the pod address register, saving the old value
 * restores when this goes out of scope.
 */
class pod_address_guard
{
public:
    /**
     * save the old pod address
     * set the new pod address
     */
    pod_address_guard(pod_address set) {
        save_ = get_pod_addr();
        set_pod_addr(set);
    }

    /**
     * restore the old pod address
     */
    ~pod_address_guard() {
        set_pod_addr(save_);
    }

    /**
     * @brief gets the pod address in the CSR
     */
#ifndef HOST
    pod_address get_pod_addr() {
        return pod_address::readPodAddrCSR();
    }

    /**
     * @brief sets the pod address in the CSR
     */
    void set_pod_addr(pod_address addr) {
        return pod_address::writePodAddrCSR(addr);
    }
#else
    pod_address get_pod_addr() {
        return pod_address{hb_mc_index_to_coordinate(the_device->default_pod_id, the_device->mc->config.pods)};
    }

    /**
     * @brief sets the pod address in the CSR
     */
    void set_pod_addr(pod_address addr) {
        the_device->default_pod_id = hb_mc_coordinate_to_index(addr.pod_, the_device->mc->config.pods);
    }
#endif
    pod_address save_;
};

}

#endif
