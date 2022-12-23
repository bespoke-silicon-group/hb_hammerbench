#include <math.h>
#include "option_data.hpp"
// Cumulative Normal Distribution Function
// See Hull, Section 11.8, P.243-244
#define inv_sqrt_2xPI 0.39894228040143270286f

#define fmadd_asm(rd_p, rs1_p, rs2_p, rs3_p) \
    asm volatile ("fmadd.s %[rd], %[rs1], %[rs2], %[rs3]" \
      : [rd] "=f" (rd_p) \
      : [rs1] "f" ((rs1_p)), [rs2] "f" ((rs2_p)), [rs3] "f" ((rs3_p)))
#define fmsub_asm(rd_p, rs1_p, rs2_p, rs3_p) \
    asm volatile ("fmsub.s %[rd], %[rs1], %[rs2], %[rs3]" \
      : [rd] "=f" (rd_p) \
      : [rs1] "f" ((rs1_p)), [rs2] "f" ((rs2_p)), [rs3] "f" ((rs3_p)))
#define fnmsub_asm(rd_p, rs1_p, rs2_p, rs3_p) \
    asm volatile ("fnmsub.s %[rd], %[rs1], %[rs2], %[rs3]" \
      : [rd] "=f" (rd_p) \
      : [rs1] "f" ((rs1_p)), [rs2] "f" ((rs2_p)), [rs3] "f" ((rs3_p)))


// Using taylor series to approximate ln(x) around x=1
__attribute__ ((always_inline))
float my_logf(float x)
{
  float diff = x - 1.0f;
  const float c5 =  0.20000000f;
  const float c4 = -0.25000000f;
  const float c3 =  0.33333333f;
  const float c2 = -0.50000000f;
  const float c1 =  1.00000000f;
  
  float retval;
  fmadd_asm(retval, diff, c5, c4); 
  fmadd_asm(retval, retval, diff, c3); 
  fmadd_asm(retval, retval, diff, c2); 
  fmadd_asm(retval, retval, diff, c1); 
  return retval*diff;
}

// Approximating expf(x) around x=0
__attribute__ ((always_inline))
float expf_0(float x) {
  const float f5 = 0.0083333338f; // 1/120
  const float f4 = 0.04166667f;   // 1/24
  const float f3 = 0.16666666f;   // 1/6
  const float f2 = 0.5f;
  const float f1 = 1.0f;
  float retval;
  fmadd_asm(retval, x, f5, f4);  
  fmadd_asm(retval, retval, x, f3);  
  fmadd_asm(retval, retval, x, f2);  
  fmadd_asm(retval, retval, x, f1);  
  fmadd_asm(retval, retval, x, f1);  
  return retval;
}


// fast expf for x < 0
__attribute__ ((always_inline))
float fast_expf(float x) {
  // if it's too small, return 0;
  if (x < -15.0f) {
    return 0.0f;
  }

  // calculate the integral part of the exponen;
  float part1 = 1.0f;
  while (x <= -1.0f) {
    part1 *=  0.36787945f; // e^(-1)
    x += 1.0f;
  }

  // calculate the 0.5 part of the exponent;
  if (x <= -0.5f) {
    part1 *= 0.60653066f; // e^(-0.5)
    x += 0.5f;
  }

  // calculate the fractional part of the exponent;
  float part2 = expf_0(x);
  
  // return the combined result;
  return part1 * part2;
}
  

float CNDF (float x)
{
    float OutputX;
    float xNPrimeofX;
    float expValues;
    float xK2_2, xK2_3;
    float xK2_4, xK2_5;
    float x1, x2, x3, x4, x5;
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

    // expValues
    expValues = fast_expf(-0.5f * x_abs * x_abs) * inv_sqrt_2xPI;

    // calculate x1
    fmadd_asm(x1, 0.2316419f, x_abs, 1.0f);
    x1 = 1.0f / x1;

    xK2_2 = x1 * x1;
    xK2_3 = xK2_2 * x1;
    xK2_4 = xK2_3 * x1;
    xK2_5 = xK2_4 * x1;
    
    xLocal_1 = x1 * 0.319381530f;
    xLocal_2 = xK2_2 * (-0.356563782f);
    xLocal_3 = xK2_3 * 1.781477937f;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_4 * (-1.821255978f);
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_5 * 1.330274429f;
    xLocal_2 = xLocal_2 + xLocal_3;

    xLocal_1 = xLocal_2 + xLocal_1;
    //xLocal   = 1.0f - (xLocal_1 * expValues);
    fnmsub_asm(xLocal, xLocal_1, expValues, 1.0f);

    if (sign) {
        xLocal = 1.0f - xLocal;
    }
    
    return xLocal;
} 

void BlkSchlsEqEuroNoDiv_kernel(OptionData* option)
{
    // input reg
    float s_reg = option->s;
    float strike_reg = option->strike;
    float v_reg = option->v;
    float t_reg = option->t;
    float r_reg = option->r;
    // constant
    float halff = 0.5f;
    float onef = 1.0f;


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
      logValues = my_logf(s_reg / strike_reg);
    }

    fmadd_asm(xD1, v_reg*v_reg, halff, r_reg);
    fmadd_asm(xD1, xD1, t_reg, logValues);
    xDen = v_reg * sqrt_time;
    xD1 = xD1 / xDen;
    xD2 = xD1 - xDen;

    
    NofXd1 = CNDF( xD1 );
    NofXd2 = CNDF( xD2 );

    //FutureValueX = strike_reg * ( expf( -(r_reg)*(t_reg) ) );
    FutureValueX = strike_reg * ( expf_0( -(r_reg)*(t_reg) ) );
    option->call = (s_reg * NofXd1) - (FutureValueX * NofXd2);
    NegNofXd1 = (1.0f - NofXd1);
    NegNofXd2 = (1.0f - NofXd2);
    option->put = (FutureValueX * NegNofXd2) - (s_reg * NegNofXd1);
}
