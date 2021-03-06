#include <float.h>
#include <math.h>

#include "m_trig.h"
#include "m_const.h"

#include "u_assert.h"
#include "u_traits.h"

#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

namespace m {

static constexpr floatShape kNaN(~0u);

static const double kC1PIO2 = 1*kPiHalf;
static const double kC2PIO2 = 2*kPiHalf;
static const double kC3PIO2 = 3*kPiHalf;
static const double kC4PIO2 = 4*kPiHalf;

// R(x^2): a rational approximation of (asin(x)-x)/x^3
// Remez error is bounded by:
//  |(asin(x)-x)/x^3 - R(x^2)| < 2^(-58.75)
static const float kPS0 =  1.6666586697e-01;
static const float kPS1 = -4.2743422091e-02;
static const float kPS2 = -8.6563630030e-03;
static const float kQS1 = -7.0662963390e-01;
static float R(float z) {
    const float p = z*(kPS0+z*(kPS1+z*kPS2));
    const float q = 1.0f+z*kQS1;
    return p/q;
}

// On i386 the x87 FPU does everything in 80 bits of precision. Using a double
// for the reduced range approximations would cause the compiler to generate
// additional load and stores to reduce the 80bit results to 64bit ones. By using
// the effective type floating-point arithmetic is evaluated on in hardware we
// can eliminate those.
#if FLOAT_EVAL_METHOD == 2
typedef long double f64;
typedef f64 f32;
#else
typedef double f64;
typedef float f32;
#endif

// |cos(x) - c(x)| < 2**-34.1 (~[-5.37e-11, 5.295e-11])
static const double kC0 = -4.99999997251031003e-01;
static const double kC1 = 4.16666233237390632e-02;
static const double kC2 = -1.38867637746099295e-03;
static const double kC3 = 2.43904487962774091e-05;

static inline float cosdf(double x) {
    const f64 z = x*x;
    const f64 w = z*z;
    const f64 r = kC2+z*kC3;
    return ((1.0+z*kC0) + w*kC1) + (w*z)*r;
}

// |sin(x)/x - s(x)| < 2**-37.5 (~[-4.89e-12, 4.824e-12])
static const double kS0 = -1.66666666416265236e-01;
static const double kS1 = 8.33332938588946318e-03;
static const double kS2 = -1.98393348360966317e-04;
static const double kS3 = 2.71831149398982191e-06;

static inline float sindf(double x) {
    const f64 z = x*x;
    const f64 w = z*z;
    const f64 r = kS2+z*kS3;
    const f64 s = z*x;
    return (x + s*(kS0 + z*kS1)) + s*w*r;
}

// |cos(x) - c(x)| < 2**-34.1 (~[-5.37e-11, 5.295e-11])]
// |sin(x)/x - s(x)| < 2**-37.5 (~[-4.89e-12, 4.824e-12])
template <int E>
static inline void sincosdf(double x, float &sin, float &cos) {
    const f64 z = x*x;
    // polynomial reduction into independent terms for parallel evaluation
    const f64 rc = kC2+z*kC3;
    const f64 rs = kS2+z*kS3;
    const f64 w = z*z;
    const f64 s = z*x;
    const f64 ss = (x + s*(kS0 + z*kS1)) + s*w*rc;
    const f64 cc = ((1.0+z*kC0) + w*kC1) + (w*z)*rs;
    // Compiler synthesizes away branches
    sin = E & 1 ? -ss : ss;
    cos = E & 2 ? -cc : cc;
}

// |tan(x)/x - t(x)| < 2**-25.5 (~[-2e-08, 2e-08])
static const double kT0 = 3.33331395030791400e-01;
static const double kT1 = 1.33392002712976743e-01;
static const double kT2 = 5.33812378445670394e-02;
static const double kT3 = 2.45283181166547279e-02;
static const double kT4 = 2.97435743359967305e-03;
static const double kT5 = 9.46564784943673167e-03;
static inline float tandf(double x, bool odd) {
    const f64 z = x*x;
    // polynomial reduction into independent terms for parallel evaluation
    const f64 r = kT4 + z*kT5;
    const f64 t = kT2 + z*kT3;
    const f64 w = z*z;
    const f64 s = z*x;
    const f64 u = kT0 + z*kT1;
    // add up small terms from lowest degree up for efficiency on non-sequential
    // systems (lower terms tend to be ready earlier.)
    const f64 v = (x + s*u) + (s*w)*(t + w*r);
    return odd ? -1.0/v : v;
}

#if FLT_EVAL_METHOD == 0 || FLT_EVAL_METHOD == 1
static const double kToInt = 1.5 / DBL_EPSILON;
#elif FLT_EVAL_METHOD == 2 || defined(_MSC_VER)
static const double kToInt = 1.5 / LDBL_EPSILON;
#endif
static const double kInvPio2 = 6.36619772367581382433e-01;
static const double kPIO2H = 1.57079631090164184570e+00;
static const double kPIO2T = 1.58932547735281966916e-08;

static inline int rempio2(float x, uint32_t ix, double &y) {
    // 25+53 bit pi is good enough for median size
    if (ix < 0x4DC90FDB) { // |x| ~< 2^28*(pi/2)
        const f64 fn = (f64)x*kInvPio2 + kToInt - kToInt;
        const int n = int32_t(fn);
        y = x - fn*kPIO2H - fn*kPIO2T;
        return n;
    }

    U_ASSERT(ix < 0x7F800000 && "NaN");

    // value is far too large, something is pathologically wrong
    U_ASSERT(0 && "function called with a huge value");
    return 0;
}

float cos(float x) {
    const floatShape shape = { x };
    const uint32_t ix = shape.asInt & 0x7FFFFFFF;
    const uint32_t sign = shape.asInt >> 31;
    if (ix <= 0x3F490FDA) // |x| ~<= pi/4
        return ix < 0x39800000 ? 1.0f : cosdf(x); // |x| < 2**-12
    if (ix <= 0x407B53D1) { // |x| ~<= 5*pi/4
        if (ix > 0x4016CBE3) // |x| ~> 3*pi/4
            return -cosdf(sign ? x+kC2PIO2 : x-kC2PIO2);
        return sindf(sign ? x+kC1PIO2 : kC1PIO2-x);
    }
    if (ix <= 0x40E231D5) { // |x| ~<= 9*pi/4
        if (ix > 0x40AFEDDF) // |x| ~> 7*pi/4
            return cosdf(sign ? x+kC4PIO2 : x-kC4PIO2);
        return sindf(sign ? -x-kC3PIO2 : x-kC3PIO2);
    }
    U_ASSERT(ix < 0x7F800000 && "NaN");
    double y = 0.0;
    const int n = rempio2(x, ix, y);
    switch (n&3) {
    case 0: return cosdf(y);
    case 1: return sindf(-y);
    case 2: return -cosdf(y);
    }
    return sindf(y);
}

static const float kPIO2Hi = 1.5707962513e+00; // 0x3FC90FDA
static const float kPIO2Lo = 7.5497894159e-08; // 0x33A22168
float acos(float x) {
    const floatShape shape = { x };
    const uint32_t ix = shape.asInt & 0x7FFFFFFF;
    const uint32_t sign = shape.asInt >> 31;
    if (ix >= 0x3F800000) { // |x| >= 1 or NaN
        if (ix == 0x3F800000)
            return sign ? 2*kPIO2Hi + 7.52316384526264005e-37f : 0.0f;
        U_ASSERT(0 && "NaN");
    }
    if (ix < 0x3F000000) { // |x| < 0.5
        return ix <= 0x32800000
            ? kPIO2Hi + 7.52316384526264005e-37f // |x| < 2**-26
            : kPIO2Hi - (x - (kPIO2Lo-x*R(x*x)));
    }
    if (sign) { // x < -0.5
        const float z = (1.0f+x)*0.5f;
        const float s = m::sqrt(z);
        const float w = R(z)*s-kPIO2Lo;
        return 2*(kPIO2Hi - (s+w));
    }
    // x > 0.5
    const float z = (1-x)*0.5f;
    const float s = m::sqrt(z);
    const float f = (floatShape { (floatShape { s }).asInt & 0xFFFFF000 }).asFloat;
    const float c = (z-f*f)/(s+f);
    const float w = R(z)*s+c;
    return 2*(f+w);
}

float sin(float x) {
    const floatShape shape = { x };
    const uint32_t ix = shape.asInt & 0x7FFFFFFF;
    const uint32_t sign = shape.asInt >> 31;
    if (ix <= 0x3F490FDA) // |x| ~<= pi/4
        return ix < 0x39800000 ? x : sindf(x); // |x| < 2**-12
    if (ix <= 0x407B53D1) { // |x| ~<= 5*pi/4
        if (ix <= 0x4016CBE3) // |x| ~<= 3pi/4
            return sign ? -cosdf(x + kC1PIO2) : cosdf(x - kC1PIO2);
        return sindf(sign ? -(x + kC2PIO2) : -(x - kC2PIO2));
    }
    if (ix <= 0x40E231D5) { // |x| ~<= 9*pi/4
        if (ix <= 0x40AFEDDF) // |x| ~<= 7*pi/4
            return sign ? cosdf(x + kC3PIO2) : -cosdf(x - kC3PIO2);
        return sindf(sign ? x+kC4PIO2 : x-kC4PIO2);
    }
    U_ASSERT(ix < 0x7F800000 && "NaN");
    double y = 0.0;
    const int n = rempio2(x, ix, y);
    switch (n&3) {
    case 0: return sindf(y);
    case 1: return cosdf(y);
    case 2: return sindf(-y);
    }
    return -cosdf(y);
}

static const double kPIO2 = 1.570796326794896558e+00;

float asin(float x) {
    const floatShape shape = { x };
    const uint32_t ix = shape.asInt & 0x7FFFFFFF;
    const uint32_t sign = shape.asInt >> 31;
    if (ix >= 0x3F800000) { // |x| >= 1
        if (ix == 0x3F800000)
            return x*kPIO2 + 7.52316384526264005e-37f; // asin(+-1) = +-pi/2 with inexact
        U_ASSERT(0 && "NaN"); // asin(|x|>1) is NaN
    }
    if (ix < 0x3F000000) { // |x| < 0.5
        // if 0x1p-126 <= |x| < 0x1p-120
        if (ix < 0x39800000 && ix >= 0x00800000)
            return x;
        return x + x*R(x*x);
    }
    // 1 > |x| >= 0.5
    const float z = (1 - m::abs(x)) * 0.5f;
    const float s = m::sqrt(z);
    const float m = kPIO2 - 2*(s+s*R(z));
    return sign ? -m : m;
}

float tan(float x) {
    const floatShape shape = { x };
    const uint32_t ix = shape.asInt & 0x7FFFFFFF;
    const uint32_t sign = shape.asInt >> 31;
    if (ix < 0x3F490FDa) // |x| ~<= pi/4
        return ix < 0x39800000 ? x : tandf(x, 0); // |x| < 2**-12
    if (ix <= 0x407B53D1) { // |x| ~<= 5*pi/4
        if (ix <= 0x4016CBE3) // |x| ~<= 3pi/4
            return tandf((sign ? x+kC1PIO2 : x-kC1PIO2), true);
        return tandf((sign ? x+kC2PIO2 : x-kC2PIO2), false);
    }
    if (ix <= 0x40E231D5) { // |x| ~<= 9*pi/4
        if (ix <= 0x40AFEDDF) // |x| ~<= 7*pi/4
            return tandf((sign ? x+kC3PIO2 : x-kC3PIO2), true);
        return tandf((sign ? x+kC4PIO2 : x-kC4PIO2), false);
    }
    U_ASSERT(ix < 0x7F800000);
    double y = 0.0;
    const int n = rempio2(x, ix, y);
    return tandf(y, n&1);
}

static const float kATanHi[] = {
    4.6364760399e-01, // atan(0.5)hi 0x3EED6338
    7.8539812565e-01, // atan(1.0)hi 0x3F490FDA
    9.8279368877e-01, // atan(1.5)hi 0x3F7b985E
    1.5707962513e+00, // atan(inf)hi 0x3FC90FDA
};

static const float kATanLo[] = {
    5.0121582440e-09, // atan(0.5)lo 0x31AC3769
    3.7748947079e-08, // atan(1.0)lo 0x33222168
    3.4473217170e-08, // atan(1.5)lo 0x33140FB4
    7.5497894159e-08, // atan(inf)lo 0x33A22168
};

static const float kAT[] = {
     3.3333328366e-01,
    -1.9999158382e-01,
     1.4253635705e-01,
    -1.0648017377e-01,
     6.1687607318e-02,
};

float atan(float x) {
    const floatShape shape = { x };
    const uint32_t ix = shape.asInt & 0x7FFFFFFF;
    const uint32_t sign = shape.asInt >> 31;

    int i;
    if (ix >= 0x4c800000) { // if |x| >= 2**26
        if (x != x)
            return x;
        const float z = kATanHi[3] + 7.52316384526264005e-37f;
        return sign ? -z : z;
    }
    if (ix < 0x3ee00000) { // |x| < 0.4375
        if (ix < 0x39800000) // |x| < 2**-12
            return x;
        i = -1;
    } else {
        x = m::abs(x);
        if (ix < 0x3f980000) { // |x| < 1.1875
            if (ix < 0x3f300000) { // 7/16 <= |x| < 11/16
                i = 0;
                x = (2.0f*x - 1.0f)/(2.0f + x);
            } else { // 11/16 <= |x| < 19/16
                i = 1;
                x = (x - 1.0f)/(x + 1.0f);
            }
        } else {
            if (ix < 0x401c0000) { // |x| < 2.4375
                i = 2;
                x = (x - 1.5f)/(1.0f + 1.5f*x);
            } else { // 2.4375 <= |x| < 2**26
                i = 3;
                x = -1.0f/x;
            }
        }
    }
    const float z = x*x;
    const float w = z*z;

    // break sum from i=0..10 kAT[i]z**(i+1) into odd and even polynomials
    const float s1 = z*(kAT[0]+w*(kAT[2]+w*kAT[4]));
    const float s2 = w*(kAT[1]+w*kAT[3]);
    if (i < 0)
        return x - x*(s1+s2);

    const float m = kATanHi[i] - ((x*(s1+s2) - kATanLo[i]) - x);
    return sign ? -m : m;
}

void sincos(float x, float &s, float &c) {
    const floatShape shape = { x };
    const uint32_t ix = shape.asInt & 0x7FFFFFFF;
    const uint32_t sign = shape.asInt >> 31;
    if (ix <= 0x3F490FDA) { // |x| ~<= pi/4
        if (ix < 0x39800000) { // |x| < 2**-12
            s = x;
            c = 1.0f;
            return;
        }
        sincosdf<0>(x, s, c);
        return;
    }
    if (ix <= 0x407B53D1) { // |x| ~<= 5*pi/4
        if (ix <= 0x4016CBE3) { // |x| ~<= 3*pi/4
            if (sign)
                sincosdf<2>(x + kC1PIO2, c, s);
            else
                sincosdf<0>(kC1PIO2 - x, c, s);
            return;
        }
        // -sin(x+c) is not correct if x+c could be 0: -0 vs 0
        sincosdf<3>(sign ? x+kC2PIO2 : x-kC2PIO2, s, c);
        return;
    }
    if (ix <= 0x40E231D5) { // |x| ~<= 9*pi/4
        if (ix <= 0x40AFEDDF) { // |x| ~<= 7*pi/4
            if (sign)
                sincosdf<1>(x+kC3PIO2, c, s);
            else
                sincosdf<2>(x-kC3PIO2, c, s);
            return;
        }
        sincosdf<0>(sign ? x+kC4PIO2 : x-kC4PIO2, s, c);
        return;
    }
    U_ASSERT(ix < 0x7F800000);
    double y = 0.0;
    // argument reduction
    const int n = rempio2(x, ix, y);
    switch (n&3) {
    // sin = s, cos = c
    case 0:
        sincosdf<0>(y, s, c);
        return;
    // sin = c, cos = -s
    case 1:
        sincosdf<1>(y, c, s);
        return;
    // sin = -s, cos = -c
    case 2:
        sincosdf<3>(y, s, c);
        return;
    // sin = -c, cos = s
    default:
        sincosdf<2>(y, c, s);
        return;
    }
}


// |(log(1+s)-log(1-s))/s - Lg(s)| < 2**-34.24 (~[-4.95e-11, 4.97e-11])
static const float kIvLn2Hi = 1.4428710938e+00; // 0x3fb8b000
static const float kIvLn2Lo = -1.7605285393e-04; // 0xb9389ad4
static const float kLg1 = 6.66666626930236816e-01;
static const float kLg2 = 4.00009721517562866e-01;
static const float kLg3 = 2.84987866878509521e-01;
static const float kLg4 = 2.42790788412094116e-01;

float log2(float x) {
    floatShape shape = { x };
    uint32_t ix = shape.asInt;
    int k = 0;
    if (ix < 0x00800000 || (ix >> 31)) { // x <2**-16
        if ((ix << 1) == 0)
            return -1/(x*x); // log(+-0) = -inf
        if ((ix >> 31))
            return kNaN.asFloat;
        // scale up subnormal number
        k -= 25;
        x *= 3.35544320000000000e+07f;
        shape.asFloat = x;
        ix = shape.asInt;
    } else if (ix >= 0x7F800000) {
        return x;
    } else if (ix == 0x3F800000) {
        return 0.0f;
    }
    // reduce x into [sqrt(2)/2, sqrt(2)]
    ix += 0x3F800000 - 0x3F3504F3;
    k += int(ix>>23) - 0x7f;
    ix = (ix & 0x007FFFFF) + 0x3F3504F3;
    shape.asInt = ix;
    x = shape.asFloat;
    const f32 f = x - 1.0f;
    const f32 s = f/(2.0f + f);
    const f32 z = s*s;
    const f32 w = z*z;
    const f32 t1 = w*(kLg2+w*kLg4);
    const f32 t2 = z*(kLg1+w*kLg3);
    const f32 R = t2 + t1;
    const f32 hfsq = 0.5f*f*f;
    f32 hi = f - hfsq;
    shape.asFloat = hi;
    shape.asInt &= 0xFFFFF000;
    hi = shape.asFloat;
    const f32 lo = f - hi - hfsq + s*(hfsq+R);
    return (lo+hi)*kIvLn2Lo + lo*kIvLn2Hi + hi*kIvLn2Hi + k;
}

float pow(float a, float b) {
    union {
        double asDouble;
        int64_t asBits;
    } shape = { a }, reshape;
    const int32_t t1 = (int32_t)(shape.asBits >> 32);
    const int32_t t2 = (int32_t)(b * (t1 - 1072632447) + 1072632447);
    reshape.asBits = ((int64_t)t2) << 32;
    return reshape.asDouble;
}

#ifdef __SSE2__
float sqrt(float x) {
    // The use of inverse square root is actually faster than a full
    // sqrtss instruction which would be issued if we called sqrtf(x)
    // like we do below. This only has ~11 bits of precision and has
    // 1/4th the latency.
    //
    // We can actually survive with ~11 bits of precision without much
    // concern for accuracy since we favor techniques which avoid sqrt
    // where precision matters.
    const __m128 a = _mm_set1_ps(x);   // shufps $0x0,%xmm0,%xmm0
                                       // movaps %xmm0,%xmm1
    const __m128 b = _mm_rsqrt_ss(a);  // rsqrtss %xmm0,%xmm1
    const __m128 c = _mm_mul_ps(a, b); // mulps %xmm1, %xmm0
    return _mm_cvtss_f32(c);           // retq
}

float floor(float in) {
    const __m128 x = _mm_set1_ps(in);
#ifdef __SSE4_1__
    const __m128 ret = _mm_floor_ps(x);
#else
    const __m128i v0 = _mm_setzero_si128();
    const __m128i v1 = _mm_cmpeq_epi32(v0, v0);
    const __m128i ji = _mm_srli_epi32(v1, 25);
    const __m128i z1 = _mm_slli_epi32(ji, 23);
    __m128 j = *(__m128*)&z1;
    const __m128i i = _mm_cvttps_epi32(x);
    const __m128 fi = _mm_cvtepi32_ps(i);
    const __m128 igx = _mm_cmpgt_ps(fi, x);
    j = _mm_and_ps(igx, j);
    const __m128 ret = _mm_sub_ps(fi, j);
#endif
    return _mm_cvtss_f32(ret);
}

float ceil(float in) {
    const __m128 x = _mm_set1_ps(in);
#ifdef __SSE4_1__
    const __m128 ret = _mm_ceil_ps(x);
#else
    const __m128i v0 = _mm_setzero_si128();
    const __m128i v1 = _mm_cmpeq_epi32(v0, v0);
    const __m128i ji = _mm_srli_epi32(v1, 25);
    const __m128i z1 = _mm_slli_epi32(ji, 23);
    __m128 j = *(__m128*)&z1;
    const __m128i i = _mm_cvttps_epi32(x);
    const __m128 fi = _mm_cvtepi32_ps(i);
    const __m128 igx = _mm_cmplt_ps(fi, x);
    j = _mm_and_ps(igx, j);
    const __m128 ret = _mm_add_ps(fi, j);
#endif
    return _mm_cvtss_f32(ret);
}

float fmod(float x, float y) {
    const __m128 a = _mm_set1_ps(x);
    const __m128 div = _mm_set1_ps(y);
    const __m128 c = _mm_div_ps(a, div);
    const __m128i i = _mm_cvttps_epi32(c);
    const __m128 trunc = _mm_cvtepi32_ps(i);
    const __m128 base = _mm_mul_ps(trunc, div);
    const __m128 ret = _mm_sub_ps(a, base);
    return _mm_cvtss_f32(ret);
}
#else
float floor(float x) {
    floatShape shape = { x };
    const int e = int(shape.asInt >> 23 & 0xFF) - 0x7F;
    if (e >= 23)
        return x;
    if (e >= 0) {
        const uint32_t m = 0x007FFFFF >> e;
        if ((shape.asInt & m) == 0)
            return x;
        if ((shape.asInt >> 31))
            shape.asInt += m;
        shape.asInt &= ~m;
    } else {
        if ((shape.asInt >> 31) == 0)
            shape.asInt = 0;
        else if ((shape.asInt << 1))
            shape.asFloat = -1.0f;
    }
    return shape.asFloat;
}

float ceil(float x) {
    floatShape shape = { x };
    const int e = int(shape.asInt >> 23 & 0xFF) - 0x7F;
    if (e >= 23)
        return x;
    if (e >= 0) {
        const uint32_t m = 0x007FFFFF >> e;
        if ((shape.asInt & m) == 0)
            return x;
        if ((shape.asInt >> 31) == 0)
            shape.asInt += m;
        shape.asInt &= ~m;
    } else {
        if ((shape.asInt >> 31))
            shape.asFloat = -0.0f;
        else if ((shape.asInt << 1))
            shape.asFloat = 1.0f;
    }
    return shape.asFloat;
}

float fmod(float x, float y) {
    floatShape ux = { x };
    floatShape uy = { y };
    int ex = ux.asInt >> 23 & 0xFF;
    int ey = uy.asInt >> 23 & 0xFF;
    const uint32_t sx = ux.asInt & 0x80000000;
    uint32_t uxi = ux.asInt;
    uint32_t uyi = uy.asInt;

    if (uxi << 1 <= uyi << 1)
        return x;

    // normalize x and y
    auto normalize = [](int &c, uint32_t &v) {
        if (c == 0) {
            for (uint32_t i = v << 9; i >> 31 == 0; c--, i <<= 1)
                ;
            v <<= -c + 1;
        } else {
            v &= -1u >> 9;
            v |= 1u << 23;
        }
    };
    normalize(ex, uxi);
    normalize(ey, uyi);

    // x mod y
    for (; ex > ey; ex--) {
        uint32_t i = uxi - uyi;
        if (i >> 31 == 0) {
            if (i == 0)
                return 0.0f;
            uxi = i;
        }
        uxi <<= 1;
    }
    uint32_t i = uxi - uyi;
    if (i >> 31 == 0) {
        if (i == 0)
            return 0.0f;
        uxi = i;
    }
    for (; uxi >> 23 == 0; uxi <<= 1, ex--)
        ;

    // scale result
    if (ex > 0) {
        uxi -= 1u << 23;
        uxi |= uint32_t(ex) << 23;
    } else {
        uxi >>= -ex + 1;
    }
    uxi |= sx;
    ux.asInt = uxi;
    return ux.asFloat;
}

float sqrt(float x) {
    return sqrtf(x);
}
#endif

}
