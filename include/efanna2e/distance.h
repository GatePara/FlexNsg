//
// Motified by WuTong on 2024/8/8.
//



#ifndef EFANNA2E_DISTANCE_H
#define EFANNA2E_DISTANCE_H
#include <efanna2e/platform.h>
#include <iostream>
namespace efanna2e{
  enum Metric{
    L2 = 0,
    INNER_PRODUCT = 1,
    FAST_L2 = 2,
    PQ = 3
  };
    class Distance {
    public:
        virtual float compare(const float* a, const float* b, unsigned length) const = 0;
        virtual ~Distance() {}
    };

    class DistanceL2 : public Distance{
    public:
        inline float compare(const float* a, const float* b, unsigned size) const {
            float result = 0;

#ifdef __GNUC__
#ifdef USE_AVX

  #define AVX_L2SQR(addr1, addr2, dest, tmp1, tmp2) \
      tmp1 = _mm256_loadu_ps(addr1);\
      tmp2 = _mm256_loadu_ps(addr2);\
      tmp1 = _mm256_sub_ps(tmp1, tmp2); \
      tmp1 = _mm256_mul_ps(tmp1, tmp1); \
      dest = _mm256_add_ps(dest, tmp1);

      __m256 sum;
      __m256 l0, l1;
      __m256 r0, r1;
      unsigned D = (size + 7) & ~7U;
      unsigned DR = D % 16;
      unsigned DD = D - DR;
      const float *l = a;
      const float *r = b;
      const float *e_l = l + DD;
      const float *e_r = r + DD;
      float unpack[8] __attribute__ ((aligned (32))) = {0, 0, 0, 0, 0, 0, 0, 0};

      sum = _mm256_loadu_ps(unpack);
      if(DR){AVX_L2SQR(e_l, e_r, sum, l0, r0);}

      for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16) {
      	AVX_L2SQR(l, r, sum, l0, r0);
      	AVX_L2SQR(l + 8, r + 8, sum, l1, r1);
      }
      _mm256_storeu_ps(unpack, sum);
      result = unpack[0] + unpack[1] + unpack[2] + unpack[3] + unpack[4] + unpack[5] + unpack[6] + unpack[7];

#else
#ifdef USE_SSE2
  #define SSE_L2SQR(addr1, addr2, dest, tmp1, tmp2) \
          tmp1 = _mm_load_ps(addr1);\
          tmp2 = _mm_load_ps(addr2);\
          tmp1 = _mm_sub_ps(tmp1, tmp2); \
          tmp1 = _mm_mul_ps(tmp1, tmp1); \
          dest = _mm_add_ps(dest, tmp1);

  __m128 sum;
  __m128 l0, l1, l2, l3;
  __m128 r0, r1, r2, r3;
  unsigned D = (size + 3) & ~3U;
  unsigned DR = D % 16;
  unsigned DD = D - DR;
  const float *l = a;
  const float *r = b;
  const float *e_l = l + DD;
  const float *e_r = r + DD;
  float unpack[4] __attribute__ ((aligned (16))) = {0, 0, 0, 0};

  sum = _mm_load_ps(unpack);
  switch (DR) {
      case 12:
      SSE_L2SQR(e_l+8, e_r+8, sum, l2, r2);
      case 8:
      SSE_L2SQR(e_l+4, e_r+4, sum, l1, r1);
      case 4:
      SSE_L2SQR(e_l, e_r, sum, l0, r0);
    default:
      break;
  }
  for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16) {
      SSE_L2SQR(l, r, sum, l0, r0);
      SSE_L2SQR(l + 4, r + 4, sum, l1, r1);
      SSE_L2SQR(l + 8, r + 8, sum, l2, r2);
      SSE_L2SQR(l + 12, r + 12, sum, l3, r3);
  }
  _mm_storeu_ps(unpack, sum);
  result += unpack[0] + unpack[1] + unpack[2] + unpack[3];

//nomal distance
#else
#ifdef USE_NEON
  #define NEON_L2SQR(addr1, addr2, dest, tmp1, tmp2) \
      tmp1 = vld1q_f32(addr1);\
      tmp2 = vld1q_f32(addr2);\
      tmp1 = vsubq_f32(tmp1, tmp2);\
      dest = vfmaq_f32(dest, tmp1, tmp1);

  float32x4_t sum_vec = vdupq_n_f32(0);
  float32x4_t vec1,vec2;
  unsigned i = 0;
  const float *l = a;
  const float *r = b;
   for (; i + 4 <= size; i += 4) {
        NEON_L2SQR(l+i,r+i,sum_vec,vec1,vec2);
    }
  float sum = vaddvq_f32(sum_vec);
  for (; i < size; ++i) {
        float diff = l[i] - r[i];
        sum += diff * diff;
    }
  result = sum;
#else

      float diff0, diff1, diff2, diff3;
      const float* last = a + size;
      const float* unroll_group = last - 3;

      /* Process 4 items with each loop for efficiency. */
      while (a < unroll_group) {
          diff0 = a[0] - b[0];
          diff1 = a[1] - b[1];
          diff2 = a[2] - b[2];
          diff3 = a[3] - b[3];
          result += diff0 * diff0 + diff1 * diff1 + diff2 * diff2 + diff3 * diff3;
          a += 4;
          b += 4;
      }
      /* Process last 0-3 pixels.  Not needed for standard vector lengths. */
      while (a < last) {
          diff0 = *a++ - *b++;
          result += diff0 * diff0;
      }
#endif
#endif
#endif
#endif
            return result;
        }
    };

  class DistanceInnerProduct : public Distance{
  public:
    float compare(const float* a, const float* b, unsigned size) const {
      float result = 0;
#ifdef __GNUC__
#ifdef USE_AVX
      #define AVX_DOT(addr1, addr2, dest, tmp1, tmp2) \
          tmp1 = _mm256_loadu_ps(addr1);\
          tmp2 = _mm256_loadu_ps(addr2);\
          tmp1 = _mm256_mul_ps(tmp1, tmp2); \
          dest = _mm256_add_ps(dest, tmp1);

	  __m256 sum;
   	  __m256 l0, l1;
   	  __m256 r0, r1;
      unsigned D = (size + 7) & ~7U;
      unsigned DR = D % 16;
      unsigned DD = D - DR;
   	  const float *l = a;
   	  const float *r = b;
      const float *e_l = l + DD;
   	  const float *e_r = r + DD;
      float unpack[8] __attribute__ ((aligned (32))) = {0, 0, 0, 0, 0, 0, 0, 0};

      sum = _mm256_loadu_ps(unpack);
      if(DR){AVX_DOT(e_l, e_r, sum, l0, r0);}

      for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16) {
	    AVX_DOT(l, r, sum, l0, r0);
	    AVX_DOT(l + 8, r + 8, sum, l1, r1);
      }
      _mm256_storeu_ps(unpack, sum);
      result = unpack[0] + unpack[1] + unpack[2] + unpack[3] + unpack[4] + unpack[5] + unpack[6] + unpack[7];

#else
#ifdef USE_SSE2
      #define SSE_DOT(addr1, addr2, dest, tmp1, tmp2) \
          tmp1 = _mm128_loadu_ps(addr1);\
          tmp2 = _mm128_loadu_ps(addr2);\
          tmp1 = _mm128_mul_ps(tmp1, tmp2); \
          dest = _mm128_add_ps(dest, tmp1);
      __m128 sum;
      __m128 l0, l1, l2, l3;
      __m128 r0, r1, r2, r3;
      unsigned D = (size + 3) & ~3U;
      unsigned DR = D % 16;
      unsigned DD = D - DR;
      const float *l = a;
      const float *r = b;
      const float *e_l = l + DD;
      const float *e_r = r + DD;
      float unpack[4] __attribute__ ((aligned (16))) = {0, 0, 0, 0};

      sum = _mm_load_ps(unpack);
      switch (DR) {
          case 12:
          SSE_DOT(e_l+8, e_r+8, sum, l2, r2);
          case 8:
          SSE_DOT(e_l+4, e_r+4, sum, l1, r1);
          case 4:
          SSE_DOT(e_l, e_r, sum, l0, r0);
        default:
          break;
      }
      for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16) {
          SSE_DOT(l, r, sum, l0, r0);
          SSE_DOT(l + 4, r + 4, sum, l1, r1);
          SSE_DOT(l + 8, r + 8, sum, l2, r2);
          SSE_DOT(l + 12, r + 12, sum, l3, r3);
      }
      _mm_storeu_ps(unpack, sum);
      result += unpack[0] + unpack[1] + unpack[2] + unpack[3];
#else
#ifdef USE_NEON
  #define NEON_DOT(addr1, addr2, dest, tmp1, tmp2) \
      tmp1 = vld1q_f32(addr1);\
      tmp2 = vld1q_f32(addr2);\
      dest = vfmaq_f32(dest, tmp1, tmp2);

  float32x4_t sum_vec = vdupq_n_f32(0);
  float32x4_t vec1,vec2;
  unsigned i = 0;
  const float *l = a;
  const float *r = b;
   for (; i + 4 <= size; i += 4) {
        NEON_DOT(l+i,r+i,sum_vec,vec1,vec2);
    }
  float sum = vaddvq_f32(sum_vec);
  for (; i < size; ++i) {
        float dot = l[i] * r[i];
        sum += dot;
    }
  result = sum;
#else

      float dot0, dot1, dot2, dot3;
      const float* last = a + size;
      const float* unroll_group = last - 3;

      /* Process 4 items with each loop for efficiency. */
      while (a < unroll_group) {
          dot0 = a[0] * b[0];
          dot1 = a[1] * b[1];
          dot2 = a[2] * b[2];
          dot3 = a[3] * b[3];
          result += dot0 + dot1 + dot2 + dot3;
          a += 4;
          b += 4;
      }
      /* Process last 0-3 pixels.  Not needed for standard vector lengths. */
      while (a < last) {
          result += *a++ * *b++;
      }
#endif
#endif
#endif
#endif
      return result;
    }

  };
  class DistanceFastL2 : public DistanceInnerProduct{
   public:
    float norm(const float* a, unsigned size) const{
      float result = 0;
#ifdef __GNUC__
#ifdef USE_AVX
#define AVX_L2NORM(addr, dest, tmp) \
    tmp = _mm256_loadu_ps(addr); \
    tmp = _mm256_mul_ps(tmp, tmp); \
    dest = _mm256_add_ps(dest, tmp);

    __m256 sum;
   	__m256 l0, l1;
    unsigned D = (size + 7) & ~7U;
    unsigned DR = D % 16;
    unsigned DD = D - DR;
    const float *l = a;
    const float *e_l = l + DD;
    float unpack[8] __attribute__ ((aligned (32))) = {0, 0, 0, 0, 0, 0, 0, 0};

    sum = _mm256_loadu_ps(unpack);
    if(DR){AVX_L2NORM(e_l, sum, l0);}
	for (unsigned i = 0; i < DD; i += 16, l += 16) {
      AVX_L2NORM(l, sum, l0);
      AVX_L2NORM(l + 8, sum, l1);
    }
    _mm256_storeu_ps(unpack, sum);
    result = unpack[0] + unpack[1] + unpack[2] + unpack[3] + unpack[4] + unpack[5] + unpack[6] + unpack[7];
#else
#ifdef USE_SSE2
#define SSE_L2NORM(addr, dest, tmp) \
    tmp = _mm128_loadu_ps(addr); \
    tmp = _mm128_mul_ps(tmp, tmp); \
    dest = _mm128_add_ps(dest, tmp);

    __m128 sum;
    __m128 l0, l1, l2, l3;
    unsigned D = (size + 3) & ~3U;
    unsigned DR = D % 16;
    unsigned DD = D - DR;
    const float *l = a;
    const float *e_l = l + DD;
    float unpack[4] __attribute__ ((aligned (16))) = {0, 0, 0, 0};

    sum = _mm_load_ps(unpack);
    switch (DR) {
        case 12:
        SSE_L2NORM(e_l+8, sum, l2);
        case 8:
        SSE_L2NORM(e_l+4, sum, l1);
        case 4:
        SSE_L2NORM(e_l, sum, l0);
      default:
        break;
    }
    for (unsigned i = 0; i < DD; i += 16, l += 16) {
        SSE_L2NORM(l, sum, l0);
        SSE_L2NORM(l + 4, sum, l1);
        SSE_L2NORM(l + 8, sum, l2);
        SSE_L2NORM(l + 12, sum, l3);
    }
    _mm_storeu_ps(unpack, sum);
    result += unpack[0] + unpack[1] + unpack[2] + unpack[3];
#else
#ifdef USE_NEON
  #define NEON_NORM(addr, dest, tmp) \
      tmp = vld1q_f32(addr);\
      dest = vfmaq_f32(dest, tmp, tmp);

  float32x4_t sum_vec = vdupq_n_f32(0);
  float32x4_t vec;
  unsigned i = 0;
  const float *l = a;

   for (; i + 4 <= size; i += 4) {
        NEON_NORM(l+i,sum_vec,vec);
    }
  float sum = vaddvq_f32(sum_vec);
  for (; i < size; ++i) {
        float norm = l[i] * l[i];
        sum += norm;
    }
  result = sum;
#else
    float dot0, dot1, dot2, dot3;
    const float* last = a + size;
    const float* unroll_group = last - 3;

    /* Process 4 items with each loop for efficiency. */
    while (a < unroll_group) {
        dot0 = a[0] * a[0];
        dot1 = a[1] * a[1];
        dot2 = a[2] * a[2];
        dot3 = a[3] * a[3];
        result += dot0 + dot1 + dot2 + dot3;
        a += 4;
    }
    /* Process last 0-3 pixels.  Not needed for standard vector lengths. */
    while (a < last) {
        result += (*a) * (*a);
        a++;
    }
#endif
#endif
#endif
#endif
      return result;
    }
    using DistanceInnerProduct::compare;
    float compare(const float* a, const float* b, float norm, unsigned size) const {//not implement
      float result = -2 * DistanceInnerProduct::compare(a, b, size);
      result += norm;
      return result;
    }
  };
}



#endif //EFANNA2E_DISTANCE_H
