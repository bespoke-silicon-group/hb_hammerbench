#ifndef CELLO_CONFIG_HPP
#define CELLO_CONFIG_HPP
#include <util/class_field.hpp>
#include <stdint.h>
namespace cello
{
/**
 * @brief config
 */
class config
{
public:
    FIELD(int, pod_x); //!< pod x
    FIELD(int, pod_y); //!< pod y
    FIELD(uint32_t, dram_buffer); //!< dram buffer
    FIELD(uint32_t, dram_buffer_size); //!< dram buffer size
};
}
#endif
