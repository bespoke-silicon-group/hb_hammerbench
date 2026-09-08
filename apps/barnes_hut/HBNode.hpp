#include <stdint.h>

typedef struct HBNode_ {
  uint32_t child[8];
  float co_mass;
  float co_pos[3];
  float diamsq;
} HBNode;

// Child EVAs use bit zero as the leaf tag; clear it before conversion to a pointer.
inline uint32_t hb_child_eva(uint32_t tagged_child) {
  return tagged_child & ~uint32_t(1);
}
