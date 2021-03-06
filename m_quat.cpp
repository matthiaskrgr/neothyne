#include "u_misc.h"

#include "m_quat.h"
#include "m_vec.h"
#include "m_mat.h"

namespace m {

void quat::getOrient(vec3 *direction, vec3 *up, vec3 *side) const {
    if (side) {
        side->x = 1.0f - 2.0f * (y * y + z * z);
        side->y = 2.0f * (x * y + w * z);
        side->z = 2.0f * (x * z - w * y);
    }
    if (up) {
        up->x = 2.0f * (x * y - w * z);
        up->y = 1.0f - 2.0f * (x * x + z * z);
        up->z = 2.0f * (y * z + w * x);
    }
    if (direction) {
        direction->x = 2.0f * (x * z + w * y);
        direction->y = 2.0f * (y * z - w * x);
        direction->z = 1.0f - 2.0f * (x * x + y * y);
    }
}

#ifdef __SSE2__
#ifdef __SSE3__
#include <immintrin.h>
#else
static inline __m128 _mm_addsub_ps(__m128 a, __m128 b) {
    alignas(16) static const float kSign[] = { -1.0f, 1.0f, -1.0f, 1.0f };
    const __m128 n = *(const __m128 *)kSign;
    b = _mm_mul_ps(b, n); // 5/4, 1
    a = _mm_add_ps(a, b); // 5/4, 1
    return a;
}
#endif

quat operator*(const quat &l, const quat &r) {
    // ~18 cycles if SSE3 is supported (addsubps)
    // ~21 cycles for SSE2
    __m128 t0 = _mm_pshufd(l.v, _MM_SHUFFLE(3,3,3,3)); // 1, 0.5
    __m128 t1 = _mm_pshufd(r.v, _MM_SHUFFLE(2,3,0,1)); // 1, 0.5
    __m128 t3 = _mm_pshufd(l.v, _MM_SHUFFLE(0,0,0,0)); // 1, 0.5
    __m128 t4 = _mm_pshufd(r.v, _MM_SHUFFLE(1,0,3,2)); // 1, 0.5
    __m128 t5 = _mm_pshufd(l.v, _MM_SHUFFLE(1,1,1,1)); // 1, 0.5
    __m128 t6 = _mm_pshufd(r.v, _MM_SHUFFLE(2,0,3,1)); // 1, 0.5
    // [d,d,d,d]*[z,w,x,y] = [dz,dw,dx,dy]
    __m128 m0 = _mm_mul_ps(t0, t1); // 5/4, 1
    // [a,a,a,a]*[y,x,w,z] = [ay,ax,aw,az]
    __m128 m1 = _mm_mul_ps(t3, t4); // 5/4, 1
    // [b,b,b,b]*[z,x,w,y] = [bz,bx,bw,by]
    __m128 m2 = _mm_mul_ps(t5, t6); // 5/4, 1
    // [c,c,c,c]*[w,z,x,y] = [cw,cz,cx,cy]
    __m128 t7 = _mm_pshufd(l.v, _MM_SHUFFLE(2,2,2,2)); // 1, 0.5
    __m128 t8 = _mm_pshufd(r.v, _MM_SHUFFLE(3,2,0,1)); // 1, 0.5
    __m128 m3 = _mm_mul_ps(t7, t8); // 5/4, 1
    // [dz,dw,dx,dy]+-[ay,ax,sw,az] = [dz+ay,dw-ax,dx+aw,dy-az]
    __m128 e = _mm_addsub_ps(m0, m1); // 3, 1
    // [dx+aw,dz+ay,dy-az,dw-ax]
    e = _mm_pshufd(e, _MM_SHUFFLE(1,3,0,2)); // 1, 0.5
    // [dx+aw,dz+ay,dy-az,dw-ax]+-[bz,bx,bw,by] = [dx+aw+bz,dz+ay-bx,dy-az+bw,dw-ax-by]
    e = _mm_addsub_ps(e, m2); // 3, 1
    // [dz+ay-bx,dw-ax-by,dy-az+bw,dx+aw+bz]
    e = _mm_pshufd(e, _MM_SHUFFLE(2,0,1,3)); // 1, 0.5
    // [dz+ay-bx,dw-ax-by,dy-az+bw,dx+aw+bz]+-[cw,cz,cx,cy] = [dz+ay-bx+cw,dw-ax-by-cz,dy-az+bw+cx,dx+aw+bz-cy]
    e = _mm_addsub_ps(e, m3); // 3, 1
    // [dw-ax-by-cz,dz+ay-bx+cw,dy-az+bw+cx,dx+aw+bz-cy]
    return _mm_pshufd(e, _MM_SHUFFLE(2,3,1,0)); // 1, 0.5
}
#else
quat operator*(const quat &l, const quat &r) {
    const float w = (l.w * r.w) - (l.x * r.x) - (l.y * r.y) - (l.z * r.z);
    const float x = (l.x * r.w) + (l.w * r.x) + (l.y * r.z) - (l.z * r.y);
    const float y = (l.y * r.w) + (l.w * r.y) + (l.z * r.x) - (l.x * r.z);
    const float z = (l.z * r.w) + (l.w * r.z) + (l.x * r.y) - (l.y * r.x);
    return { x, y, z, w };
}
#endif

m::mat4 quat::getMatrix() const {
    // It's possible to construct a rotation matrix as the product of two
    // simple 4x4 matrices. Lets show how this accomplishes just that,
    // (forget everything you've learned about the relationship of columns
    //  and rows with respect to multi-vectors.)
    //
    // Remember the standard formula for vector rotation by a quaternion?
    //
    // q v ~q
    //
    // The trick exploited here is to represent each product above as a
    // matrix transformation and concatenate the results.
    //
    // Lets represent the quaternion as a rotor in geometric algebra -
    // though you can do the same using the standard formula, this tends
    // to show the process better though.
    //
    // Our rotor `q' expressed in a bivector basis:
    //  q1 e2e3 + q2 e3e1 + q3 e1e2 + q4
    //
    // Note: e2e3, e3e1, and e1e2 may correspond to the yz, xz, and xy
    // planes respectively
    //
    // We want to calculate q v ~q - to do that we need to find a matrix
    // to represent the multiplication by a rotor on the left first.
    //
    // let I = e1e2e3
    // (q1 e2e3 + q2 e3e1 + q3 e1e2 + q4) e1    q1 I  +  q2 e3 + -q3 e2 + q4 e1
    //                                    e2 = -q1 e3 +  q2 I  +  q3 e1 + q4 e2
    //                                    e3    q1 e2 + -q2 e1 +  q3 I  + q4 e3
    //                                    I    -q1 e1 + -q2 e2 + -q3 e3 + q4 I
    //
    // which is the matrix:
    // |  q4 -q3  q2 q1 |
    // |  q3  q4 -q1 q2 |
    // | -q2  q1  q4 a3 |
    // | -q1 -q2 -q3 14 |
    //
    // If we follow the same process for right-multiplication by the
    // rotor's inverse (-q1 e2e3 + -q2 e3e1 + -q3 e1e2 + q4) - we get
    // the following:
    //
    // e1 (-q1 e2e3 + -q2 e3e1 + -q3 e1e2 + q4)     -q1 I  +  q2 e3 + -q3 e2 + q4 e1
    // e2                                        =  -q1 e3 + -q2 I  +  q3 e1 + q4 e2
    // e3                                            q1 e2 + -q2 e1 + -q3 I  + q4 e3
    // I                                             q1 e1 +  q2 e2 +  q3 e3 + q4 I
    //
    // which is the matrix:
    // | q4 -q3  q2 -q1 |
    // | q3  q4 -q1 -q2 |
    // |-q2  q1  q4 -q3 |
    // | q1  q2  q3  q4 |
    //
    // Now we have a matrix for the left-multiplication (q v) and for the
    // right-multiplication by the conjugate (v ~q) which needs to be
    // applied sequentially - so concatenate them like so:
    //
    //                             | q4 -q3  q2  q1|   | q4 -q3  q2 -q1|
    // q v ~q =   | v1 v2 v3 0 | * | q3  q4 -q1  q2| * | q3  q4 -q1 -q2|
    //                             |-q2  q1  q4  q3|   |-q2  q1  q4 -q3|
    //                             |-q1 -q2 -q3  q4|   | q1  q2  q3  q4|
    //
    // and now we have our rotation matrix!
    const m::vec4 C0 = swizzle<3, 2, 1, 0>();
    const m::vec4 C1 = swizzle<2, 3, 0, 1>();
    const m::vec4 C2 = swizzle<1, 0, 3, 2>();
    const m::vec4 C3 = *this;

    static const m::vec4 kL0( 1.0f, -1.0f,  1.0f, -1.0f);
    static const m::vec4 kL1( 1.0f,  1.0f, -1.0f, -1.0f);
    static const m::vec4 kL2(-1.0f,  1.0f,  1.0f, -1.0f);
    static const m::vec4 kR0( 1.0f, -1.0f,  1.0f,  1.0f);
    static const m::vec4 kR1( 1.0f,  1.0f, -1.0f,  1.0f);
    static const m::vec4 kR2(-1.0f,  1.0f,  1.0f,  1.0f);
    static const m::vec4 kR3(-1.0f, -1.0f, -1.0f,  1.0f);

    return m::mat4(C0 * kL0, C1 * kL1, C2 * kL2, C3) *
           m::mat4(C0 * kR0, C1 * kR1, C2 * kR2, C3 * kR3);
}

quat operator*(const quat &q, const vec3 &v) {
    return m::vec4(q.asVec3 - v, - (q.x * v.x) - (q.y * v.y) - (q.z * v.z) );
}

quat operator*(const quat &l, float k) {
    return (vec4 &)l * k;
}

}
