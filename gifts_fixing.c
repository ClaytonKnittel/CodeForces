
#include <stdint.h>
#include <stdio.h>
#include <assert.h>


#define MAX_N 50

int main(int argc, char * argv[]) {

    int test_cases;
    int n;

    uint32_t as[MAX_N];
    uint32_t bs[MAX_N];
    uint32_t mina, minb;

    assert(fscanf(stdin, "%d", &test_cases) == 1);

    for (int t = 0; t < test_cases; t++) {
        assert(fscanf(stdin, "%d", &n) == 1);

        mina = 0xffffffffu;
        minb = 0xffffffffu;

        for (int i = 0; i < n; i++) {
            fscanf(stdin, "%u", &as[i]);
            mina = (mina < as[i]) ? mina : as[i];
        }

        for (int i = 0; i < n; i++) {
            fscanf(stdin, "%u", &bs[i]);
            minb = (minb < bs[i]) ? minb : bs[i];
        }

        uint64_t tot = 0;
        for (int i = 0; i < n; i++) {
            uint32_t a = as[i] - mina;
            uint32_t b = bs[i] - minb;
            tot += a > b ? a : b;
        }

        printf("%lld\n", tot);
    }
}


