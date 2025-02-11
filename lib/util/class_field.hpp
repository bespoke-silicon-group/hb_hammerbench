#ifndef UTIL_CLASS_FIELD_HPP
#define UTIL_CLASS_FIELD_HPP
#define FIELD_INTERNAL(type, name, internal_name)       \
    private:                                    \
    type internal_name;                         \
public:                                         \
 type get_##name() const                        \
 {                                              \
     return internal_name;                      \
 }                                              \
 void set_##name(type name)                     \
 {                                              \
     internal_name = name;                      \
 }                                              \
 type & name()                                  \
 {                                              \
     return internal_name;                      \
 }                                              \
 const type & name() const                      \
 {                                              \
     return internal_name;                      \
 }                                              \

#define FIELD(type, name) FIELD_INTERNAL(type, name, name##_)


#endif
