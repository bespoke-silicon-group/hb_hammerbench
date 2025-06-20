#include <cello/cello.hpp>
#include "common.hpp"
#include <util/statics.hpp>
#include <cmath>

#include "HBBody.hpp"

//#define TRACE
#ifdef TRACE
#define trace(i)                                \
    bsg_print_int(i)
#else
#define trace(i)
#endif


template <typename T>
using gref = bsg_global_pointer::reference<T>;
template <typename T>
using gptr = bsg_global_pointer::pointer<T>;
using guard = bsg_global_pointer::pod_address_guard;

#define GADDRESSOF(gref)                        \
    bsg_global_pointer::addressof(gref)

#ifndef GRAIN_SCALE
#define GRAIN_SCALE 8
#endif

// Constants;
#define itolsq  (1.0f/(0.5f*0.5f))
#define epssq   (0.05f*0.05f)
#define dthf    (0.25f)

inline void updateForce(float* force, float* delta, float distsq, float mass) {
  CELLO_STAT_ADDM(cello_flops, 9);
  float idr =  1.0f / sqrtf(distsq + epssq);
  float scale = mass * idr * idr * idr;
  force[0] = delta[0] * scale;
  force[1] = delta[1] * scale;
  force[2] = delta[2] * scale;
}

inline float dist2(float x, float y, float z) {
  CELLO_STAT_ADDM(cello_flops, 5);
  return (x*x) + (y*y) + (z*z);
}

DRAM(body_vector) bodies;
DRAM(node_vector) nodes;
DRAM(int*) nodestack;

int cello_main(int argc, char *argv[])
{
    int grain = bodies.local_size()/(cello::threads()*GRAIN_SCALE);
    if (grain < 1)
        grain = 1;

#ifdef CELLO_GLOBAL_STEALING
    bodies.foreach_unrestricted(grain, [](int curr, gref<HBBody> rcurr_body){
        gptr<HBBody> pcurr_body = GADDRESSOF(rcurr_body);
        HBBody curr_body = *pcurr_body;
#else
    bodies.foreach(grain, [](int curr, HBBody &rcurr_body){
        HBBody *pcurr_body = &rcurr_body;
        HBBody curr_body = *pcurr_body;
#endif
        // delta;
        float delta[3];
        float distsq;

        float prev_acc[3];
        prev_acc[0] = curr_body.acc[0];
        prev_acc[1] = curr_body.acc[1];
        prev_acc[2] = curr_body.acc[2];
        curr_body.acc[0] = 0.0f;
        curr_body.acc[1] = 0.0f;
        curr_body.acc[2] = 0.0f;

        // my stack in DRAM;
        int* mystack = &nodestack[__bsg_id*STACK_SIZE];
        int* max_mystack_ptr = &mystack[STACK_SIZE+1];

        // put the root in the stack
        mystack[0] = 0;
        int* mystack_top = &mystack[1];
        trace(1000000 + curr);
        while (mystack_top != mystack) {
            // take one off the stack;
            mystack_top--;
            int curr_node_idx = *mystack_top;
            //bsg_print_int(2000000 + curr_node_idx);
            global_node_pointer curr_node = bsg_global_pointer::addressof(nodes[curr_node_idx]);

            // distsq;
            float l_co_mass;
            float l_co_pos[3];
            float l_diamsq;
            l_co_mass = curr_node->CoMass();
            l_co_pos[0] = curr_node->CoPos(0);
            l_co_pos[1] = curr_node->CoPos(1);
            l_co_pos[2] = curr_node->CoPos(2);
            l_diamsq = curr_node->DiamSq();
            asm volatile("": : :"memory");

            delta[0] = curr_body.pos[0] - l_co_pos[0];
            delta[1] = curr_body.pos[1] - l_co_pos[1];
            delta[2] = curr_body.pos[2] - l_co_pos[2];
            float curr_diamsq = itolsq * l_diamsq;
            distsq = dist2(delta[0], delta[1], delta[2]);
            CELLO_STAT_ADDM(cello_flops, 1);
            if (distsq >= curr_diamsq) {
                // far away; compute summarized force;
                float node_force[3];
                updateForce(node_force, delta, distsq, l_co_mass);
                CELLO_STAT_ADDM(cello_flops, 3);
                curr_body.acc[0] += node_force[0];
                curr_body.acc[1] += node_force[1];
                curr_body.acc[2] += node_force[2];
            } else {
                //float child_diamsq = curr_diamsq * 0.25f;
                // Move down;
                // Load all child pointers;
                uint32_t children[8];
                uint32_t tmp0 = curr_node->Child(0);
                uint32_t tmp1 = curr_node->Child(1);
                uint32_t tmp2 = curr_node->Child(2);
                uint32_t tmp3 = curr_node->Child(3);
                uint32_t tmp4 = curr_node->Child(4);
                uint32_t tmp5 = curr_node->Child(5);
                uint32_t tmp6 = curr_node->Child(6);
                uint32_t tmp7 = curr_node->Child(7);
                asm volatile("": : :"memory");
                children[0] = tmp0;
                children[1] = tmp1;
                children[2] = tmp2;
                children[3] = tmp3;
                children[4] = tmp4;
                children[5] = tmp5;
                children[6] = tmp6;
                children[7] = tmp7;
                asm volatile("": : :"memory");
                for (int i = 0; i < 8; i++) {
                    if (children[i] == 0) {
                        // skip null pointer;
                        continue;
                    } else {
                        uint32_t is_leaf = children[i] & leaf;
                        if (is_leaf) {
                            // child is leaf;
                            uint32_t body_idx = children[i] & ~leaf;
                            if (body_idx != curr) {
                                bsg_global_pointer::pointer<HBBody> body_ptr = bsg_global_pointer::addressof(bodies[body_idx]);
                                CELLO_STAT_ADDM(cello_flops, 6);
                                // child is not self;
                                float child_pos[3];
                                float child_mass;
                                float child_delta[3];
                                float child_distsq;
                                child_pos[0] = body_ptr->Pos(0);
                                child_pos[1] = body_ptr->Pos(1);
                                child_pos[2] = body_ptr->Pos(2);
                                child_mass = body_ptr->Mass();
                                asm volatile("": : :"memory");
                                child_delta[0] = curr_body.pos[0] - child_pos[0];
                                child_delta[1] = curr_body.pos[1] - child_pos[1];
                                child_delta[2] = curr_body.pos[2] - child_pos[2];
                                child_distsq = dist2(child_delta[0], child_delta[1], child_delta[2]);
                                float child_force[3];
                                updateForce(child_force, child_delta, child_distsq, child_mass);
                                curr_body.acc[0] += child_force[0];
                                curr_body.acc[1] += child_force[1];
                                curr_body.acc[2] += child_force[2];
                            }
                        } else {
                            // child is an internal node;
                            // put it on the stack;
                            *mystack_top =  children[i];
                            mystack_top++;                            
                            if (mystack_top == max_mystack_ptr) {
                                bsg_print_int(0xdeadbeef);
                            }
                        }
                    }
                }
            }  
        }

        // Finished traversal;
        CELLO_STAT_ADDM(cello_flops, 9);
        float new_vel[3]; 
        new_vel[0] = dthf * (curr_body.acc[0] - prev_acc[0]);
        new_vel[1] = dthf * (curr_body.acc[1] - prev_acc[1]);
        new_vel[2] = dthf * (curr_body.acc[2] - prev_acc[2]);
        curr_body.vel[0] += new_vel[0];
        curr_body.vel[1] += new_vel[1];
        curr_body.vel[2] += new_vel[2];
        bodies[curr].Vel(0) = curr_body.vel[0];
        bodies[curr].Vel(1) = curr_body.vel[1];
        bodies[curr].Vel(2) = curr_body.vel[2];
        bodies[curr].Acc(0) = curr_body.acc[0];
        bodies[curr].Acc(1) = curr_body.acc[1];
        bodies[curr].Acc(2) = curr_body.acc[2];
    });

    return 0;
}
