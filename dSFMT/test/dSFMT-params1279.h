#define POS1	5
#define SL1	33
#define SL2	1
#define SR1	4
#define SR2	8
#define MSK1	UINT64_C(0xfbaffbde7bffef7f)
#define MSK2	UINT64_C(0xf7ffbcbff5ffffff)
#define MSK32_1	0xfbaffbdeU
#define MSK32_2	0x7bffef7fU
#define MSK32_3	0xf7ffbcbfU
#define MSK32_4	0xf5ffffffU
#define PCV1	UINT64_C(0x0007f1de08000001)
#define PCV2	UINT64_C(0x000d303ee3092a2b)
#define IDSTR	"dSFMT-1279:5-33-1-4-8:fbaffbde7bffef7f-f7ffbcbff5ffffff"


/* PARAMETERS FOR ALTIVEC */
#define ALTI_SL1 1
#define SL1_PERM \
(vector unsigned char)(4,5,6,7,28,28,28,28,12,13,14,15,0,1,2,3)
#define SL1_MSK \
(vector unsigned int)(0xfffffffeU,0x00000000U,0xfffffffeU,0x00000000U)
#define SL2_PERM \
(vector unsigned char)(1,2,3,4,5,6,7,31,9,10,11,12,13,14,15,0)
#define ALTI_SR1 4
#define SR1_MSK \
(vector unsigned int)(0x0baffbdeU,0x7bffef7fU,0x07ffbcbfU,0xf5ffffffU)
#define SR2_PERM \
(vector unsigned char)(17,0,1,2,3,4,5,6,17,8,9,10,11,12,13,14)
#define ALTI_PERM (vector unsigned char) \
  (8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7)
#define ALTI_LOW_MSK (vector unsigned int) \
  (LOW_MASK32_1, LOW_MASK32_2, LOW_MASK32_1, LOW_MASK32_2)
#define ALTI_HIGH_CONST (vector unsigned int)(HIGH_CONST32, 0, HIGH_CONST32, 0)