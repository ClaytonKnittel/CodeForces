
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define MAX_N 100

// mod must be a power of 2
#define ROUND_UP(num, mod) (((num) + ((mod) - 1)) & (~((mod) - 1)))

#ifdef __LP64__

typedef uint64_t bitv_t;
#define BITV_IDX_SHIFT 6
#define BITV_BMASK 0x3fLU
#define BITV_LEN 2

#else

typedef uint32_t bitv_t;
#define BITV_IDX_SHIFT 5
#define BITV_BMASK 0x1f
#define BITV_LEN 4

#endif

int main(int argc, char * argv[]) {

    int test_cases;
    int n;

    bitv_t bitv[BITV_LEN];

    assert(fscanf(stdin, "%d", &test_cases) == 1);

    for (int t = 0; t < test_cases; t++) {
        assert(fscanf(stdin, "%d", &n) == 1);

        for (int i = 0; i < BITV_LEN; i++) {
            bitv[i] = 0;
        }

        for (int i = 0; i < n; i++) {
            uint32_t a;
            assert(fscanf(stdin, "%d", &a) == 1);

            uint32_t idx = a >> BITV_IDX_SHIFT;
            uint32_t bit = a & BITV_BMASK;
            bitv[idx] |= (bitv_t) (1LU << bit);
        }

        //printf("%08x%08x%08x%08x\n", bitv[3], bitv[2], bitv[1], bitv[0]);
        //printf("%016llx%016llx\n", bitv[1], bitv[0]);

        int rem = 0;
        int8_t bitcnt = 0;

        for (int i = 0; i < BITV_LEN; i++) {
            int32_t loc = __builtin_ffsl(bitv[i]);
            bitv_t add = rem ? rem : (loc ? (1LU << (loc - 1)) : 0);
            bitv_t sum = bitv[i] + add;
            rem = (add != 0LU) && (sum == 0LU);

            bitv[i] = sum;

            bitcnt += sum ? ((sum & (sum - 1)) ? 2 : 1) : 0;
        }

        printf("%s\n", bitcnt == 1 ? "YES" : "NO");
    }
}

