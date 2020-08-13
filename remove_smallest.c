
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define MAX_N 100

// mod must be a power of 2
#define ROUND_UP(num, mod) (((num) + ((mod) - 1)) & (~((mod) - 1)))

int main(int argc, char * argv[]) {

    int test_cases;
    int n;

    uint64_t bitv[2];

    assert(fscanf(stdin, "%d", &test_cases) == 1);

    for (int t = 0; t < test_cases; t++) {
        assert(fscanf(stdin, "%d", &n) == 1);

        bitv[0] = 0;
        bitv[1] = 0;

        for (int i = 0; i < n; i++) {
            uint32_t a;
            assert(fscanf(stdin, "%d", &a) == 1);

            uint32_t idx = a >> 6;
            uint32_t bit = a & 0x3f;
            bitv[idx] |= (1LU << bit);
        }

        //printf("%016llx%016llx\n", bitv[1], bitv[0]);

        int32_t first_loc = __builtin_ffsl(bitv[0]);
        uint64_t add = first_loc ? (1LU << (first_loc - 1)) : 0;
        //printf("add 1: %llx\n", add);
        uint64_t sum = bitv[0] + add;
        bitv[0] &= sum;
        int32_t rem = (add != 0LU) && (sum == 0LU);

        int32_t sec_loc = __builtin_ffsl(bitv[1]);
        add = rem ? rem : (sec_loc && !sum ? (1LU << (sec_loc - 1)) : 0);
        //printf("add 2: %llx\n", add);
        bitv[1] &= bitv[1] + add;

        //printf("first_loc: %d\nrem: %d\nsec_loc: %d\n", first_loc, rem, sec_loc);

        int is = bitv[0] == 0LU && bitv[1] == 0LU;

        printf("%s\n", is ? "YES" : "NO");
    }
}

