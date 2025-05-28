#ifndef CELLO_HOST_GROUP_POINTER
#define CELLO_HOST_GROUP_POINTER
#include <global_pointer/intptr.hpp>

#define REMOTE_EPA_WIDTH      18
#define REMOTE_X_CORD_WIDTH   6
#define REMOTE_Y_CORD_WIDTH   5
#define REMOTE_X_CORD_SHIFT   (REMOTE_EPA_WIDTH)
#define REMOTE_Y_CORD_SHIFT   (REMOTE_X_CORD_SHIFT+REMOTE_X_CORD_WIDTH)
#define REMOTE_PREFIX_SHIFT   (REMOTE_Y_CORD_SHIFT+REMOTE_Y_CORD_WIDTH)

namespace bsg_global_pointer {
/* 
 * Remote EVA poitner to a local address in a tile within tile group
 * param[in]  x             X coordinate of destination tile
 * param[in]  y             Y coordinate of destination tile
 * param[in]  local_addr    address of varialbe in tile's local dmem
 * @return    EVA address of remote variable
 */ 
inline uintptr to_group_pointer(unsigned char x, unsigned char y, uintptr local_addr) {
    uintptr remote_prefix = (1 << REMOTE_PREFIX_SHIFT);
    uintptr y_bits = ((y) << REMOTE_Y_CORD_SHIFT);
    uintptr x_bits = ((x) << REMOTE_X_CORD_SHIFT);
    uintptr local_bits = reinterpret_cast<uintptr>(local_addr);
    return reinterpret_cast<uintptr>(remote_prefix | y_bits | x_bits | local_bits);
}
}

#undef REMOTE_EPA_WIDTH
#undef REMOTE_X_CORD_WIDTH
#undef REMOTE_Y_CORD_WIDTH
#undef REMOTE_X_CORD_SHIFT
#undef REMOTE_Y_CORD_SHIFT
#undef REMOTE_PREFIX_SHIFT

#endif
