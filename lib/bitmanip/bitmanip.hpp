#ifndef BITMANIP_BITMANIP_HPP
#define BITMANIP_BITMANIP_HPP
namespace bitmanip
{
/**
 * the number of bits to represent positive integer x
 */
template <unsigned x>
static inline constexpr unsigned bits() {
        constexpr unsigned y = x >> 1;
        return x == 0 ? 0 : 1 + bits<y>();
}
/**
 * the mask of width bits
 */
template <unsigned width>
static inline constexpr unsigned mask() {
        return (1 << width) - 1;
}

/**
 * settable/gettable handle to a bit range
 */
template <typename UINT, unsigned HI, unsigned LO, unsigned TAG=0>
struct bitrange_handle {
public:
    typedef UINT uint_type;
    static constexpr unsigned HI_BIT = HI;
    static constexpr unsigned LO_BIT = LO;

    bitrange_handle(UINT &i) : i(i) {}
    ~bitrange_handle() = default;
    bitrange_handle(bitrange_handle &&o) = default;
    bitrange_handle &operator=(bitrange_handle &&o) = default;
    bitrange_handle(const bitrange_handle &o) = default;
    bitrange_handle &operator=(const bitrange_handle &o) = default;

    static constexpr UINT lo() {
        return LO;
    }
    static constexpr UINT hi() {
        return HI;
    }
    static constexpr UINT bits() {
        return HI - LO + 1;
    }

    static constexpr UINT mask()
    {
        return ((1ull << (HI - LO + 1)) - 1) << LO;
    }

    static UINT getbits(UINT in)
    {
        return (in & mask()) >> LO;
    }

    static void setbits(UINT &in, UINT val)
    {
        in &= ~mask();
        in |=  mask() & (val << LO);
    }

    operator UINT() const { return getbits(i); }

    bitrange_handle &operator=(UINT val) {
        setbits(i, val);
        return *this;
    }

    UINT &i;
};

}
#endif
