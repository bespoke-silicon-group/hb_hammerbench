#ifndef CELLO_LOOP_INFO_HPP
#define CELLO_LOOP_INFO_HPP
#include <util/class_field.hpp>
#include <cello/threads.hpp>
namespace cello
{
/**
 * @brief Loop information
 */
template <typename IdxT>
class loop_info {
public:
    FIELD(IdxT, start);
    FIELD(IdxT, stop);
    FIELD(IdxT, step);
    FIELD(IdxT, grain);

    /**
     * @brief Constructor
     */
    loop_info(IdxT start, IdxT stop, IdxT step)
        : start_(start), stop_(stop), step_(step) {
        grain() = iters()/(threads()*8);
        if (grain() < 1) {
            grain() = 1;
        }
    }

    /**
     * @brief Constructor
     */
    loop_info(IdxT start, IdxT stop, IdxT step, IdxT grain)
        : start_(start), stop_(stop), step_(step), grain_(grain) {
    }
    
    /**
     * @brief Get the number of iterations
     */
    IdxT iters() const {
        IdxT r = 0;
        if (step() > 0) {
            r = (stop() - start() + step() - 1) / step();
        }
        if (r < 0) {
            r = 0;
        }
        return r;
    }

    /**
     * @brief Get the midpoint
     */
    IdxT mid() const {
        return start() + step() * (iters() / 2);
    }

    /**
     * @brief Leafs
     */
    IdxT leafs() const {
        return iters()/grain();
    }

    /**
     * @brief Get a loop_info for the lower half
     */
    loop_info lower() const {
        return loop_info(start(), mid(), step(), grain());
    }

    /**
     * @brief Get a loop_info for the upper half
     */
    loop_info upper() const {
        return loop_info(mid(), stop(), step(), grain());
    }
};
}
#endif
