//========================================================================
// runtime.inl
//========================================================================

#include "applrts-runtime.hpp"

namespace applrts {

namespace local {

SimpleDeque<Task*> g_taskq = SimpleDeque<Task*>();

} // namespace local

} // namespace applrts
