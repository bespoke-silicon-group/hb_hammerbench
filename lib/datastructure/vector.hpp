#ifndef DATASTRUCTURE_VECTOR_HPP
#define DATASTRUCTURE_VECTOR_HPP
#include <global_pointer/global_pointer.hpp>
#include <datastructure/types.hpp>
#include <datastructure/id.hpp>

#ifndef HOST
#include <cello/foreach.hpp>
#include <cello/foreach_block.hpp>
#include <cello/delegate.hpp>
#include <vector>
#else
#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#endif

namespace datastructure
{
template <typename T, size_t STRIDE = 1>
class vector
{
public:
#ifndef HOST
    using value_type = T;
    using value_pointer = T *;
    using value_reference = T &;
    using value_constpointer = const T *;
    using value_constreference = const T &;
    using value_gpointer = bsg_global_pointer::pointer<T>;
    using value_greference = bsg_global_pointer::reference<T>;
    using value_gconstpointer = bsg_global_pointer::pointer<const T>;
    using value_gconstreference = bsg_global_pointer::reference<const T>;
#else    
    using value_type = T;
    using value_pointer = hb_mc_eva_t;
    //using value_reference = T &;
    //using value_constpointer = const T *;
    //using value_constreference = const T &;
    using value_gpointer = bsg_global_pointer::pointer<T>;
    using value_greference = bsg_global_pointer::reference<T>;
    using value_gconstpointer = bsg_global_pointer::pointer<const T>;
    using value_gconstreference = bsg_global_pointer::reference<const T>;    
#endif
    
#ifndef HOST
    template <typename F>
    void foreach(F && f) {
        using namespace cello;
        using joiner = n_child_joiner;
        joiner j;
        joiner *jp = bsg_tile_group_remote_pointer<joiner>(tile_x(), tile_y(), &j);
        on_every_pod(jp, [this, f](){
            size_t start = STRIDE * pod_id();
            size_t step = STRIDE * num_pods();
            size_t end = size();
            cello::foreach(start, end, step, [this, f](size_t i){
                f(i, this->local(i));
            });
        });
        wait(&j);
    }

    value_reference local(size_t i) {
        using namespace cello;        
        size_t lcl = (i / (STRIDE * num_pods()));
        return data()[lcl];
    }

    value_constreference local(size_t i) const {
        using namespace cello;        
        size_t lcl = (i / (STRIDE * num_pods()));
        return data()[lcl];
    }
#endif

#define VECTOR_INDEX_METHODS(type)                                      \
    size_t pod(size_t i) const {                                        \
        return i % (STRIDE * datastructure::num_pods());                \
    }                                                                   \
    int pod_x(size_t i) const {                                         \
        return pod(i) % datastructure::num_pods_x();                    \
    }                                                                   \
    int pod_y(size_t i) const {                                         \
        return pod(i) / datastructure::num_pods_x();                    \
    }                                                                   \
    size_t lcl(size_t i) const {                                        \
        return i / (STRIDE * datastructure::num_pods());                \
    }                                                                   \
    
#define VECTOR_AT_METHODS(type)                                         \
    VECTOR_INDEX_METHODS(type);                                         \
    typename type::value_gpointer at_ptr(size_t i) {                    \
        uintptr dataptr = data();                                       \
        return type::value_gpointer::onPodXY(pod_x(i), pod_y(i), dataptr + lcl(i)*sizeof(T)); \
    }                                                                   \
    typename type::value_gconstpointer at_ptr(size_t i) const {         \
        uintptr dataptr = data();                                       \
        return type::value_gconstpointer::onPodXY(pod_x(i), pod_y(i), dataptr + lcl(i)*sizeof(T)); \
    }                                                                   \
    typename type::value_greference at(size_t i) {                      \
        return *at_ptr(i);                                              \
    }                                                                   \
    typename type::value_gconstreference at(size_t i) const {           \
        return *at_ptr(i);                                              \
    }                                                                   \
    typename type::value_greference operator[](size_t i) {              \
        return at(i);                                                   \
    }                                                                   \
    typename type::value_gconstreference operator[](size_t i) const {   \
        return at(i);                                                   \
    }

    VECTOR_AT_METHODS(vector);
    FIELD(value_pointer ,data);
    FIELD(uintptr       ,size);
};
}

template <typename T, size_t STRIDE>
class bsg_global_pointer::reference<datastructure::vector<T, STRIDE>>
{
public:
    using type = datastructure::vector<T, STRIDE>;
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(type);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(type, data);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(type, size);
    VECTOR_AT_METHODS(type);
};

#ifdef HOST
namespace datastructure
{
/**
 * @brief A class for initializing a vector
 */
template <typename T, size_t STRIDE = 1>
class vector_initializer
{
public:
    using vector_type = vector<T, STRIDE>;
    /**
     * @brief Initialize the vector
     * @param vptr The vector to initialize
     * @param dptr The device to initialize the vector on
     */
    int init(bsg_global_pointer::pointer<vector_type> vptr,
              const std::vector<T>& data,
              hb_mc_device_t *dptr,
              std::vector<std::vector<hb_mc_dma_htod_t>>& jobs_in) {

        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(dptr, pod_id) {
            hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, dptr->mc->config.pods);
            vptr.set_pod_x(pod.x).set_pod_y(pod.y);
            vptr->size() = data.size();
            hb_mc_eva_t dp;
            BSG_CUDA_CALL(hb_mc_device_pod_malloc(dptr, pod_id, data.size() * sizeof(T), &dp));
            vptr->data() = dp;
        }

        io_data.resize(dptr->num_pods);

        for (size_t i = 0; i < data.size(); i++) {
            bsg_global_pointer::pointer<T> p = vptr->at_ptr(i);
            hb_mc_coordinate_t pod = { .x = p.pod_x(), .y = p.pod_y() };
            pod_id = hb_mc_coordinate_to_index(pod, dptr->mc->config.pods);
            io_data[pod_id].push_back(data[i]);
        }
        
        hb_mc_device_foreach_pod_id(dptr, pod_id) {
            hb_mc_eva_t dst = vptr->data();
            jobs_in[pod_id].push_back({
                    dst,
                    io_data[pod_id].data(),
                    io_data[pod_id].size() * sizeof(T),
            });
        }
        return 0;
    }
    std::vector<std::vector<T>> io_data; //!< The data to initialize the vector with
};
}
#endif
#endif
