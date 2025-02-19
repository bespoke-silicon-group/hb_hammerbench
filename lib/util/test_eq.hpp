#ifndef UTIL_TEST_EQ_HPP
#define UTIL_TEST_EQ_HPP
#include <util/stringify.hpp>
#include <util/print_str.hpp>
#define _TEST_EQ(type, fmt, expr, expect)                               \
    do {                                                                \
        type e = (expr);                                                \
        if (e != expect) {                                              \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": FAIL:  "#expr " != " #expect "\n"); \
        }  else {                                                       \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": PASS:  "#expr " == " #expect "\n"); \
        }                                                               \
    } while (0)

#define _TEST_NEQ(type, fmt, expr, expect)                               \
    do {                                                                \
        type e = (expr);                                                \
        if (e == expect) {                                              \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": FAIL:  "#expr " == " #expect "\n"); \
        }  else {                                                       \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": PASS:  "#expr " != " #expect "\n"); \
        }                                                               \
    } while (0)

#define _TEST_EQ_SAVE(type, fmt, save, expr, expect)                    \
    do {                                                                \
        type e = (expr);                                                \
        save = e;                                                       \
        if (e != expect) {                                              \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": FAIL:  "#expr " != " #expect "\n"); \
        } else {                                                        \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": PASS:  "#expr " == " #expect "\n"); \
        }                                                               \
    } while (0)

#define _TEST_NEQ_SAVE(type, fmt, save, expr, expect)                    \
    do {                                                                \
        type e = (expr);                                                \
        save = e;                                                       \
        if (e == expect) {                                              \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": FAIL:  "#expr " == " #expect "\n"); \
        } else {                                                        \
            util::print_str(__FILE__ ":" STRINGIFY(__LINE__) ": PASS:  "#expr " != " #expect "\n"); \
        }                                                               \
    } while (0)

#define TEST_EQ(typefmt, expr, expect) _TEST_EQ(typefmt, expr, expect)
#define TEST_NEQ(typefmt, expr, expect) _TEST_NEQ(typefmt, expr, expect)
#define TEST_EQ_SAVE(typefmt, save, expr, expect) _TEST_EQ_SAVE(typefmt, save, expr, expect)
#define TEST_NEQ_SAVE(typefmt, save, expr, expect) _TEST_NEQ_SAVE(typefmt, save, expr, expect)

#define PTR void*        ,"%p"
#define INT int          ,"%d"
#define UINT unsigned int,"%u"
#define HINT unsigned int,"0x%x"

#endif
