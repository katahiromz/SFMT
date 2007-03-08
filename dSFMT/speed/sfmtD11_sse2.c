/* Hearty Twister Search Code, Makoto Matsumoto 2005/5/6 */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <emmintrin.h>
#include "random.h"
#include "paramsD11.h"

union W128_T {
    __m128i si;
    __m128d sd;
    uint64_t a[2];
    double d[2];
};
typedef union W128_T w128_t;

INLINE static void gen_rand_array(w128_t array[], int size);
INLINE static void gen_rand_all(void);

void fill_array_open_close(double array[], int size);
void fill_array_close_open(double array[], int size);
void fill_array_open_open(double array[], int size);
void fill_array_close1_open2(double array[], int size);
INLINE double genrand_open_close(void);
INLINE double genrand_close_open(void);
INLINE double genrand_open_open(void);
INLINE double genrand_close1_open2(void);

static w128_t sfmt[N + 1];
static double *psfmt = &sfmt[0].d[0];
static int idx;

static __m128i sse2_param_mask;
static __m128i sse2_low_mask;
static __m128i sse2_high_const;
static __m128i sse2_int_one;
static __m128d sse2_double_two;
static __m128d sse2_double_m_one;

static void setup_const(void) {
    static int first = true;
    if (!first) {
	return;
    }
    sse2_param_mask = _mm_set_epi32(MSK32_3, MSK32_4, MSK32_1, MSK32_2);
    sse2_low_mask = _mm_set_epi32(LOW_MASK32_1, LOW_MASK32_2,
				  LOW_MASK32_1, LOW_MASK32_2);
    sse2_int_one = _mm_set_epi32(0, 1, 0, 1);
    sse2_high_const = _mm_set_epi32(HIGH_CONST32, 0, HIGH_CONST32, 0);
    sse2_double_two = _mm_set_pd(2.0L, 2.0L);
    sse2_double_m_one = _mm_set_pd(-1.0L, -1.0L);
    first = false;
}

INLINE static
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
__m128i 
mm_recursion(__m128i *a, __m128i *b, __m128i c, __m128i d) {
    __m128i v, w, x, y, z;
    
    z = _mm_load_si128(a);
    y = _mm_srli_epi64(*b, SR1);
    y = _mm_and_si128(y, sse2_param_mask);
    w = _mm_slli_epi64(c, SL1);
    x = _mm_srli_epi64(c, SR2);
    v = _mm_shuffle_epi32(d, SSE2_SHUFF);
    w = _mm_xor_si128(w, x);
    v = _mm_xor_si128(v, z);
    z = _mm_slli_si128(z, SL2);
    w = _mm_xor_si128(w, y);
    v = _mm_xor_si128(v, z);
    v = _mm_xor_si128(v, w);
    v = _mm_and_si128(v, sse2_low_mask);
    v = _mm_or_si128(v, sse2_high_const);
    return v;
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    static void convert_oc(w128_t array[], int size) {
    int i;

    for (i = 0; i < size; i++) {
	array[i].sd = _mm_sub_pd(sse2_double_two, array[i].sd);
    }
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    static void convert_co(w128_t array[], int size) {
    int i;

    for (i = 0; i < size; i++) {
	array[i].sd = _mm_add_pd(array[i].sd, sse2_double_m_one);
    }
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    static void convert_oo(w128_t array[], int size) {
    int i;
    for (i = 0; i < size; i++) {
	array[i].si = _mm_or_si128(array[i].si, sse2_int_one);
	array[i].sd = _mm_add_pd(array[i].sd, sse2_double_m_one);
    }
}

INLINE static
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    void gen_rand_all(void) {
    int i;
    __m128i r, lung;

    lung = _mm_load_si128(&sfmt[N].si);
    r = _mm_load_si128(&sfmt[N - 1].si);
    for (i = 0; i < N - POS1; i++) {
	r = mm_recursion(&sfmt[i].si, &sfmt[i + POS1].si, r, lung);
	_mm_store_si128(&sfmt[i].si, r);
	lung = _mm_xor_si128(lung, r);
    }
    for (; i < N; i++) {
	r = mm_recursion(&sfmt[i].si, &sfmt[i + POS1 - N].si, r, lung);
	_mm_store_si128(&sfmt[i].si, r);
	lung = _mm_xor_si128(lung, r);
    }
    _mm_store_si128(&sfmt[N].si, lung);
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    static void gen_rand_array(w128_t array[], int size) {
    int i, j;
    __m128i r, lung;

    lung = _mm_load_si128(&sfmt[N].si);
    r = _mm_load_si128(&sfmt[N - 1].si);
    for (i = 0; i < N - POS1; i++) {
	r = mm_recursion(&sfmt[i].si, &sfmt[i + POS1].si, r, lung);
	_mm_store_si128(&array[i].si, r);
	lung = _mm_xor_si128(lung, r);
    }
    for (; i < N; i++) {
	r = mm_recursion(&sfmt[i].si, &array[i + POS1 - N].si, r, lung);
	_mm_store_si128(&array[i].si, r);
	lung = _mm_xor_si128(lung, r);
    }
    /* main loop */
    for (; i < size - N; i++) {
	r = mm_recursion(&array[i - N].si, &array[i + POS1 - N].si, r, lung);
	_mm_store_si128(&array[i].si, r);
	lung = _mm_xor_si128(lung, r);
    }
    for (j = 0; j < 2 * N - size; j++) {
	r = _mm_load_si128(&array[j + size - N].si);
	_mm_store_si128(&sfmt[j].si, r);
    }    
    for (; i < size; i++, j++) {
	r = mm_recursion(&array[i - N].si, &array[i + POS1 - N].si, r, lung);
	_mm_store_si128(&array[i].si, r);
	_mm_store_si128(&sfmt[j].si, r);
	lung = _mm_xor_si128(lung, r);
    }
    _mm_store_si128(&sfmt[N].si, lung);
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    double genrand_close_open(void)
{
    double r;

    if (idx >= N * 2) {
	gen_rand_all();
	idx = 0;
    }
    r = psfmt[idx++];
    return r - 1.0L;
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    double genrand_open_close(void) {
    double r;

    if (idx >= N * 2) {
	gen_rand_all();
	idx = 0;
    }
    r = psfmt[idx++];
    return 2.0L - r;
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    double genrand_open_open(void) {
    union w64_t {
	uint64_t u;
	double d;
    } conv;

    if (idx >= N * 2) {
	gen_rand_all();
	idx = 0;
    }
    conv.d = psfmt[idx++];
    conv.u |= 1;
    return conv.d -1.0L;
}

INLINE
#if defined(__GNUC__) && (!defined(DEBUG))
__attribute__((always_inline)) 
#endif
    double genrand_close1_open2(void) {
    double r;

    if (idx >= N * 2) {
	gen_rand_all();
	idx = 0;
    }
    r = psfmt[idx++];
    return r;
}

void fill_array_open_close(double array[], int size)
{
    assert(size % 2 == 0);
    assert(size >= 2 * N * 2);
    gen_rand_array((w128_t *)array, size / 2);
    convert_oc((w128_t *)array, size / 2);
}

void fill_array_close_open(double array[], int size)
{
    assert(size % 2 == 0);
    assert(size >= 2 * N * 2);
    gen_rand_array((w128_t *)array, size / 2);
    convert_co((w128_t *)array, size / 2);
}

void fill_array_open_open(double array[], int size)
{
    assert(size % 2 == 0);
    assert(size >= 2 * N * 2);
    gen_rand_array((w128_t *)array, size / 2);
    convert_oo((w128_t *)array, size / 2);
}

void fill_array_close1_open2(double array[], int size)
{
    assert(size % 2 == 0);
    assert(size >= 2 * N * 2);
    gen_rand_array((w128_t *)array, size / 2);
}

void init_gen_rand(uint64_t seed)
{
    int i;
    uint64_t *psfmt;

    psfmt = (uint64_t *)&sfmt[0];
    psfmt[0] = (seed & LOW_MASK) | HIGH_CONST;
    for (i = 1; i < (N + 1) * 2; i++) {
        psfmt[i] = 1812433253UL * (psfmt[i - 1] ^ (psfmt[i - 1] >> 30)) + i;
        psfmt[i] = (psfmt[i] & LOW_MASK) | HIGH_CONST;
    }
    idx = N * 2;
    setup_const();
}

#include "test_time3.c"