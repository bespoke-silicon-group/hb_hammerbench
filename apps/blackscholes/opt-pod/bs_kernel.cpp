#include <math.h>
#include "option_data.hpp"
// Cumulative Normal Distribution Function
// See Hull, Section 11.8, P.243-244
#define inv_sqrt_2xPI 0.39894228040143270286f
float CNDF (float x)
{
    float OutputX;
    float xNPrimeofX;
    float expValues;
    float xK2;
    float xK2_2, xK2_3;
    float xK2_4, xK2_5;
    float xLocal, xLocal_1;
    float xLocal_2, xLocal_3;

    // Check for negative value of x
    float zerof = 0.0f;
    float x_abs;
    int sign;
    asm volatile ("flt.s %[rd], %[rs1], %[rs2]" \
      : [rd] "=r" (sign)
      : [rs1] "f" (x), [rs2] "f" (zerof));
    asm volatile ("fsgnj.s %[rd], %[rs1], %[rs2]" \
      : [rd] "=f" (x_abs) \
      : [rs1] "f" (x), [rs2] "f" (zerof));
    //if (x < 0.0f) {
    //    x = -x;
    //    sign = 1;
    //} else 
    //    sign = 0;

    // Compute NPrimeX term common to both four & six decimal accuracy calcs
    expValues = expf(-0.5f * x_abs * x_abs) * inv_sqrt_2xPI;

    xK2 = 0.2316419f * x_abs;
    xK2 = 1.0f + xK2;
    xK2 = 1.0f / xK2;
    xK2_2 = xK2 * xK2;
    xK2_3 = xK2_2 * xK2;
    xK2_4 = xK2_3 * xK2;
    xK2_5 = xK2_4 * xK2;
    
    xLocal_1 = xK2 * 0.319381530f;
    xLocal_2 = xK2_2 * (-0.356563782f);
    xLocal_3 = xK2_3 * 1.781477937f;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_4 * (-1.821255978f);
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_5 * 1.330274429f;
    xLocal_2 = xLocal_2 + xLocal_3;

    xLocal_1 = xLocal_2 + xLocal_1;
    xLocal   = xLocal_1 * expValues;
    xLocal   = 1.0f - xLocal;

    OutputX  = xLocal;
    
    if (sign) {
        OutputX = 1.0f - OutputX;
    }
    
    return OutputX;
} 

void BlkSchlsEqEuroNoDiv_kernel(OptionData* option)
{
    float s_reg = option->s;
    float strike_reg = option->strike;
    float v_reg = option->v;
    float t_reg = option->t;
    float r_reg = option->r;

    float xD1; 
    float xD2;
    float xDen;
    float FutureValueX;
    float NofXd1;
    float NofXd2;
    float NegNofXd1;
    float NegNofXd2;    
    
    float sqrt_time = sqrt(t_reg);
    float logValues;
    if (s_reg == strike_reg) {
      logValues = 0.0f;
    } else {
      logValues = logf(s_reg / strike_reg);
    }

    xD1 = (v_reg * v_reg * 0.5f) + r_reg;
    xD1 = xD1 * t_reg;
    xD1 = xD1 + logValues;

    xDen = v_reg * sqrt_time;
    xD1 = xD1 / xDen;
    xD2 = xD1 -  xDen;

    
    NofXd1 = CNDF( xD1 );
    NofXd2 = CNDF( xD2 );

    FutureValueX = strike_reg * ( expf( -(r_reg)*(t_reg) ) );
    option->call = (s_reg * NofXd1) - (FutureValueX * NofXd2);
    NegNofXd1 = (1.0f - NofXd1);
    NegNofXd2 = (1.0f - NofXd2);
    option->put = (FutureValueX * NegNofXd2) - (s_reg * NegNofXd1);
}
