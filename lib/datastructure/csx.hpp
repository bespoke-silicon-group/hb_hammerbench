#ifndef DATASTRUCTURE_CSX_HPP
#define DATASTRUCTURE_CSX_HPP
#include <datastructure/vector.hpp>
#include <cstddef>

#ifndef HOST
#include <cello/foreach.hpp>
#include <cello/foreach_block.hpp>
#include <cello/delegate.hpp>
#include <vector>
#else
#include <bsg_manycore.h>
#include <bsg_manycore_cuda.h>
#include <Eigen/Sparse>
#endif

namespace datastructure {

/**
 * flags for the CSX data structure
 */
typedef int32_t compressed_sparse_matrix_flag_t;

static constexpr compressed_sparse_matrix_flag_t CSX_FLAG_ROW_MAJOR = 0x1; //!< row major

#ifdef HOST
template <typename ValueType, typename IndexType, compressed_sparse_matrix_flag_t flags>
class compressed_sparse_matrix_mirror;
#endif

/**
 * @brief compressed sparse matrix
 * @tparam ValueType value type
 * @tparam IndexType index type
 * @tparam flags flags
 */
template <typename ValueType, typename IndexType = size_t, compressed_sparse_matrix_flag_t flags = CSX_FLAG_ROW_MAJOR>
class compressed_sparse_matrix {
public:
    using value_type = ValueType;
    using index_type = IndexType;
    using outer_vector_type = datastructure::vector<index_type, 1>;
#ifndef HOST
    using inner_pointer_type = index_type*;
    using value_pointer_type = value_type*;
#else
    using inner_pointer_type = hb_mc_eva_t;
    using value_pointer_type = hb_mc_eva_t;
#endif
#ifdef HOST
    using mirror_type = compressed_sparse_matrix_mirror<ValueType, IndexType, flags>;
#endif

#ifndef HOST
    template <typename F>
    void foreach_outer_index(F &&f) {
        outer_pointers().foreach([this, f](index_type outerIndex, [[maybe_unused]] index_type start) {
            if (outerIndex >= outer_size()) {
                return;
            }
            f(outerIndex);
        });
    }

    template <typename F>
    void foreach_outer_index_inner_indices(F &&f) {
        outer_pointers().foreach([this, f](index_type outerIndex, index_type start) {
            if (outerIndex >= outer_size()) {
                return;
            }
            index_type i = outer_pointers().lcl(start);
            index_type j = i+1;
            index_type end = outer_pointers().data()[j];
            f(outerIndex, inner_indices()+start, end-start);
        });
    }
    template <typename F>
    void foreach_outer_index_inner_indices_values(F &&f) {
        outer_pointers().foreach([this, f](index_type outerIndex, index_type start) {
            if (outerIndex >= outer_size()) {
                return;
            }
            index_type i = outer_pointers().lcl(start);
            index_type j = i+1;
            index_type end = outer_pointers().data()[j];
            f(outerIndex, inner_indices()+start, values()+start, end-start);
        });
    }
#endif

#define CSX_METHODS()                                      \
    index_type nnz(index_type outerIndex) const {          \
        auto offsets = outer_pointers().at_ptr(outerIndex);     \
        index_type start = offsets[0];                     \
        index_type end = offsets[1];                       \
        return end - start;                                \
    }

    CSX_METHODS();

    FIELD(index_type, inner_size);
    FIELD(index_type, outer_size);
    FIELD(outer_vector_type, outer_pointers);
    FIELD(inner_pointer_type, inner_indices);
    FIELD(value_pointer_type, values);
    FIELD(index_type, rows);
    FIELD(index_type, cols);
    FIELD(index_type, nnz);
};
}

/**
 * @brief reference specialization for the compressed sparse matrix
 */
template <typename ValueType, typename IndexType, datastructure::compressed_sparse_matrix_flag_t flags>
class bsg_global_pointer::reference<datastructure::compressed_sparse_matrix<ValueType, IndexType, flags>> {
public:
    using csx_type = datastructure::compressed_sparse_matrix<ValueType, IndexType, flags>;
    using value_type = typename csx_type::value_type;
    using index_type = typename csx_type::index_type;
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(csx_type);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, inner_size);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, outer_size);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, outer_pointers);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, inner_indices);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, values);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, rows);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, cols);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(csx_type, nnz);
    CSX_METHODS();
};

#ifdef HOST
namespace datastructure {
/**
 * @brief mirror of the compressed sparse matrix
 * @tparam ValueType value type
 * @tparam IndexType index type
 * @tparam flags flags
 */
template <typename ValueType, typename IndexType, compressed_sparse_matrix_flag_t flags>
class compressed_sparse_matrix_mirror {
public:
    using csx_type = compressed_sparse_matrix<ValueType, IndexType, flags>;
    using value_type = typename csx_type::value_type;
    using index_type = typename csx_type::index_type;
    using outer_vector_type = typename csx_type::outer_vector_type;
    using eigen_sparse_matrix_type = Eigen::SparseMatrix<value_type, flags & CSX_FLAG_ROW_MAJOR ? Eigen::RowMajor : Eigen::ColMajor, index_type>;

    /**
     * @brief return true if row major
     */
    bool is_row_major() const {
        return flags & CSX_FLAG_ROW_MAJOR;
    }
    
    /**
     * @brief get the host pod index of the outer index
     * @param outer_idx outer index
     * @return host pod index
     */
    hb_mc_pod_id_t outer_host_pod_index(size_t outer_idx) {
        return outer_pointers.host_pod_index(outer_idx);
    }

    /**
     * @brief get the local index of the outer index
     * @param outer_idx outer index
     * @return local index
     */
    size_t outer_local_indexr(size_t outer_idx) {
        return outer_pointers.lcl(outer_idx);
    }

    /**
     * @brief get the coordinate of the pod id
     * @param pod_id pod id
     * @return coordinate
     */
    hb_mc_coordinate_t pod_id_to_coordinate(hb_mc_pod_id_t pod_id) {
        return hb_mc_index_to_coordinate(pod_id, bsg_global_pointer::the_device->mc->config.pods);
    }

    /**
     * @brief constructor
     */
    compressed_sparse_matrix_mirror(const bsg_global_pointer::pointer<csx_type>& ptr_)
        : ptr(ptr_)
        , outer_pointers(bsg_global_pointer::pointer<outer_vector_type>(ptr->outer_pointers().addr())) {
        inner_indices.resize(bsg_global_pointer::the_device->num_pods);
        values.resize(bsg_global_pointer::the_device->num_pods);
    }

    /**
     * @brief initialize from a sparse matrix
     * initialize
     */
    void init_host_from(const eigen_sparse_matrix_type & mat) {
        std::vector<index_type> outer_indices;
        for (int i = 0; i < mat.outerSize(); i++) {
            int j = i+1;
            int start = mat.outerIndexPtr()[i];
            int end = mat.outerIndexPtr()[j];
            int size = end - start;
            printf("outerIndexPtr[%3d] = %3d - %3d: %3d\n", i, start, end, size);
        }
        const index_type *outerIndexPtr = mat.outerIndexPtr();
        const index_type *innerIndexPtr = mat.innerIndexPtr();
        const value_type *valuePtr = mat.valuePtr();
        // sort nonzeros into their pods by their outer index
        index_type i = 0;
        for (i = 0; i < mat.outerSize(); i++) {
            hb_mc_pod_id_t pod_id = outer_host_pod_index(i);
            outer_indices.push_back(inner_indices[pod_id].size());
            for (index_type j = outerIndexPtr[i]; j < outerIndexPtr[i + 1]; j++) {
                inner_indices[pod_id].push_back(innerIndexPtr[j]);
                values[pod_id].push_back(valuePtr[j]);
            }
        }
        // each pod, set last index to nonzeros assigned        
        for (index_type j = 0; j < bsg_global_pointer::the_device->num_pods; j++) {
            hb_mc_pod_id_t pod_id = outer_host_pod_index(i+j);
            outer_indices.push_back(inner_indices[pod_id].size());
        }
        outer_pointers.init_host_from(outer_indices);

        // debug
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(bsg_global_pointer::the_device, pod_id) {
            printf("mat.nonZeros() = %d\n", mat.nonZeros());
            printf("mat.outerSize() = %d\n", mat.outerSize());
            printf("outer_indices[outerSize() + %d] = %d\n", pod_id, outer_indices[mat.outerSize() + pod_id]);
        }
        rows = mat.rows();
        cols = mat.cols();
        nnz = mat.nonZeros();
    }

    /**
     * @brief zero out the device
     */
    int clear_device() {
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(bsg_global_pointer::the_device, pod_id) {
            hb_mc_coordinate_t pod = pod_id_to_coordinate(pod_id);
            ptr.set_pod_x(pod.x);
            ptr.set_pod_y(pod.y);
            hb_mc_eva_t ii_eva = ptr->inner_indices();
            hb_mc_eva_t v_eva = ptr->values();
            BSG_CUDA_CALL(hb_mc_device_pod_free(bsg_global_pointer::the_device, pod_id, ii_eva));
            BSG_CUDA_CALL(hb_mc_device_pod_free(bsg_global_pointer::the_device, pod_id, v_eva));
            ptr->nnz() = 0;
            ptr->rows() = 0;
            ptr->cols() = 0;
            ptr->values() = 0;
            ptr->inner_indices() = 0;
        }
        //BSG_CUDA_CALL(outer_pointers.clear_device());
        return 0;
    }

    /**
     * @brief set the device metadata from the host
     */
    int init_device_from_host() {
        hb_mc_pod_id_t pod_id;
        index_type max_inner_size = 0;
        hb_mc_device_foreach_pod_id(bsg_global_pointer::the_device, pod_id) {
            max_inner_size = std::max(max_inner_size, (index_type)inner_indices[pod_id].size());
        }
        hb_mc_device_foreach_pod_id(bsg_global_pointer::the_device, pod_id) {
            hb_mc_coordinate_t pod = pod_id_to_coordinate(pod_id);
            ptr.set_pod_x(pod.x).set_pod_y(pod.y);
            hb_mc_eva_t ii_eva, v_eva;
            BSG_CUDA_CALL(hb_mc_device_pod_malloc(bsg_global_pointer::the_device, pod_id, max_inner_size * sizeof(index_type), &ii_eva));
            BSG_CUDA_CALL(hb_mc_device_pod_malloc(bsg_global_pointer::the_device, pod_id, max_inner_size * sizeof(value_type), &v_eva));
            ptr->nnz() = nnz;
            ptr->rows() = rows;
            ptr->cols() = cols;
            ptr->values() = v_eva;
            ptr->inner_indices() = ii_eva;
            ptr->outer_size() = is_row_major() ? rows : cols;
            ptr->inner_size() = is_row_major() ? cols : rows;            
        }
        //BSG_CUDA_CALL(outer_pointers.init_device_from_host());
        return 0;
    }

    /**
     * @brief Synchronize the device with the host
     */
    int sync_device(std::vector<std::vector<hb_mc_dma_htod_t>> &jobs_in) {
        BSG_CUDA_CALL(clear_device());
        BSG_CUDA_CALL(init_device_from_host());
        BSG_CUDA_CALL(outer_pointers.sync_device(jobs_in));
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(bsg_global_pointer::the_device, pod_id) {
            jobs_in[pod_id].push_back({
                    ptr->inner_indices(),
                    inner_indices[pod_id].data(),
                    inner_indices[pod_id].size() * sizeof(index_type),
                });
            jobs_in[pod_id].push_back({
                    ptr->values(),
                    values[pod_id].data(),
                    values[pod_id].size() * sizeof(value_type),
                });
        }
        return 0;
    }

    /**
     * @brief zero out the host
     */
    int clear_host() {
        rows = cols = nnz = 0;
        for (auto &indices : inner_indices) {
            indices.clear();
        }
        for (auto &vals : values) {
            vals.clear();
        }
        //BSG_CUDA_CALL(outer_pointers.clear_host());
        return 0;
    }

    /**
     * @brief set the host metadata from the device
     */
    int init_host_from_device() {
        return 0;
    }
    
    /**
     * @brief Synchronize the host with the device
     */
    int sync_host(std::vector<std::vector<hb_mc_dma_dtoh_t>> &jobs_out) {
        clear_host();
        init_host_from_device();
        BSG_CUDA_CALL(outer_pointers.sync_host(jobs_out));
        return 0;
    }
    
    bsg_global_pointer::pointer<csx_type> ptr;
    typename outer_vector_type::mirror_type outer_pointers;
    std::vector<std::vector<index_type>> inner_indices;
    std::vector<std::vector<value_type>> values;
    index_type rows;
    index_type cols;
    index_type nnz;
};
}
#endif

#endif
