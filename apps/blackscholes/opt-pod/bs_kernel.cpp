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
    // Check for negative value of x
    const float zerof = 0.0f;
    const float onef  = 1.0f;
    float x_abs;
    int sign;
    asm volatile ("flt.s %[rd], %[rs1], %[rs2]" \
      : [rd] "=r" (sign)
      : [rs1] "f" (x), [rs2] "f" (zerof));
    asm volatile ("fsgnj.s %[rd], %[rs1], %[rs2]" \
      : [rd] "=f" (x_abs) \
      : [rs1] "f" (x), [rs2] "f" (zerof));

    // expValues
    float expValues = fast_expf(-0.5f * x_abs * x_abs) * inv_sqrt_2xPI;

    // calculate x1
    float x1;
    fmadd_asm(x1, 0.2316419f, x_abs, 1.0f);
    x1 = 1.0f / x1;

    const float c1 =  0.319381530f;
    const float c2 = -0.356563782f;
    const float c3 =  1.781477937f;
    const float c4 = -1.821255978f;
    const float c5 =  1.330274429f;
    float fx;
    fmadd_asm(fx, x1, c5, c4);
    fmadd_asm(fx, fx, x1, c3);
    fmadd_asm(fx, fx, x1, c2);
    fmadd_asm(fx, fx, x1, c1);
    fx = fx * x1;

    float xLocal;
    fnmsub_asm(xLocal, fx, expValues, onef);

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

    
    float NofXd1 = CNDF( xD1 );
    float NofXd2 = CNDF( xD2 );

    FutureValueX = strike_reg * ( expf_0( -(r_reg)*(t_reg) ) );
    option->call = (s_reg * NofXd1) - (FutureValueX * NofXd2);
    NegNofXd1 = (1.0f - NofXd1);
    NegNofXd2 = (1.0f - NofXd2);
    option->put = (FutureValueX * NegNofXd2) - (s_reg * NegNofXd1);
}
