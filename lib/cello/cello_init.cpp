#include <cello/init.hpp>
#include <cstddef>
namespace cello
{
void dummy_constructor()
{
    // Do nothing
}
cello_constructor(dummy_constructor);

void dummy_destructor()
{
    // Do nothing
}
cello_destructor(dummy_destructor);


/**
 * called by all threads before entering main loop
 */
void call_global_constructors() {
    // Call all global constructors
    global_constructor *start = (global_constructor*)&__start_cello_constructor;
    global_constructor *stop  = (global_constructor*)&__stop_cello_constructor;
    for (global_constructor *curr = start; curr != stop; curr++) {
        (*curr)();
    }
}

/**
 * called by all threads after exiting main loop
 */
void call_global_destructors() {
    // Call all global destructors
    global_destructor *start = (global_destructor*)&__start_cello_destructor;
    global_destructor *stop  = (global_destructor*)&__stop_cello_destructor;
    for (global_destructor *curr = start; curr != stop; curr++) {
        (*curr)();
    }
}
}
