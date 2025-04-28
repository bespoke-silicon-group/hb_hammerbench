#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"

DRAM(point_vector) points_in;
DRAM(point_vector) points_out;
DRAM(FP32Complex)  w [32];

unsigned reverse(unsigned x, unsigned len)
{
    for (unsigned b = 0; b < len/2; b++) {
        unsigned m = 1 << b;
        unsigned mp = 1 << (len - b);
        unsigned v = x & m;
        unsigned vp = x & mp;
        x = x & ~(m | mp);
        x = x | (v | vp);
    }
    return x;
}

unsigned log2(unsigned x) {
    unsigned y = 0, z = 1 << y;
    while (z < x) {
        y = y + 1;
        z <<= 1;
    }
    return y ;
}

FP32Complex fast_pow_w(unsigned exp) {
    unsigned b = 1;
    FP32Complex r = w[0]
    while (b < 32) {
        if ((b << 1) & exp) {
            r = r * w[b];
        }
    }
    return r;
}

int cello_main(int argc, char *argv[])
{
    // setup points of unity
    on_every_pod([](){
        float theta = (2 * M_PI)/points_in.size();
        FP32Complex t = FP32Complex{cosf(theta), sinf(theta));
        w[0] = FP32Complex{1.0, 0.0);
        for (unsigned i = 1; i < 32; i++) {
            w[i] = t;
            t = t * t;
        }
    });
    
    // bit-reverse copy
    unsigned lg2 = log2(points_in.size());
    bsg_print_unsigned(lg2);    
    cello::foreach_multipod(0u, points_in.size()/2, 1u, [=](unsigned i){
        unsigned inv = reverse(i, lg2);
        FP32Complex pi = points_in[i];
        points_out[inv] = pi;
    });

    for (unsigned scale = 1; scale < lg2; scale++) {
        unsigned m = 1 << scale;
        cello::foreach_multipod(0, points_out.size(), m, [=](unsigned k){
            FP32Complex wm = fast_pow_w(lg2 - scale);
            cello::foreach(0, m/2, [m, k](unsigned j){
                
            });
        });
    }
    
    return 0;
}
