#pragma once
#include <HammerBlade.hpp>
#include <RuntimeError.hpp>
#include <type_traits>
#include <vector>

namespace hammerblade {
    namespace host {

        template <typename T>
        class Vector {
        private:
            HammerBlade::Ptr _HB;
            size_t _size;
            hb_mc_eva_t _v_dev;

        public:
            static constexpr hb_mc_eva_t NullPtr = static_cast<T>(-1);

            ///////////////////////////////////////////////////////
            // constructors/destructors and assignment operators //
            ///////////////////////////////////////////////////////
            Vector(size_t n = 0) :
                _HB(HammerBlade::Get()),
                _size(n),
                _v_dev(NullPtr) {

                // static_assert(std::is_trivially_copyable<T>::value,
                //               "Vector class requires T be trivially copyable");

                _v_dev = _HB->alloc(n * sizeof(T));
            }

            virtual ~Vector() {
                _HB->free(_v_dev);
                _HB = nullptr;
            }

            Vector(const Vector &other)=delete;
            Vector& operator=(const Vector &other)=delete;

            Vector(Vector &&other) :
                _HB(HammerBlade::Get()),
                _size(other._size),
                _v_dev(other._v_dev) {

                other._v_dev = NullPtr;
                other._size  = 0;
            }

            Vector& operator=(Vector &&other) {
                _size = other._size;
                _v_dev = other._v_dev;

                other._v_dev = NullPtr;
                other._size = 0;
                return *this;
            }

            ///////////////////////
            // trivial accessors //
            ///////////////////////
            size_t size() const { return _size; }

            hb_mc_eva_t address() const { return _v_dev; }

            std::vector<T> get_async() {
                std::vector<T> v(_size);
                _HB->push_read(_v_dev, &v[0], _size * sizeof(T));
                return v;
            }

            std::vector<T> get() {
                auto v = get_async();
                _HB->sync_rw();
                return v;
            }

            void set_async(const std::vector<T> &data) {
                _HB->push_write(_v_dev, &data[0], sizeof(T) * _size);
            }

            void set(const std::vector<T> &data) {
                set_async(data);
                _HB->sync_rw();
            }
        };

    }
}
