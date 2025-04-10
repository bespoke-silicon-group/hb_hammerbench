#ifndef CELLO_INIT_HPP
#define CELLO_INIT_HPP
namespace cello
{
typedef void (*global_constructor)();
typedef void (*global_destructor)();

/**
 * called by all threads before entering main loop
 */
void call_global_constructors();

/**
 * called by all threads after exiting main loop
 */
void call_global_destructors();
}

#define cello_constructor(function_name)                                \
    __attribute__((section("cello_constructor"), used))                 \
    static cello::global_constructor __ ## function_name ## _constructor = function_name;

#define cello_destructor(function_name)                                 \
    __attribute__((section("cello_destructor"), used))                  \
    static cello::global_destructor __ ## function_name ## _destructor = function_name;

extern int __start_cello_constructor, __stop_cello_constructor;
extern int __start_cello_destructor, __stop_cello_destructor;
#endif
