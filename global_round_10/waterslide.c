
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int main(int argc, char * argv[]) {

    int t, n;

    fscanf(stdin, "%d", &t);
    
    while (t--) {
        fscanf(stdin, "%d", &n);

        uint64_t tot = 0;

        int prev = 0;

        for (int i = 0; i < n; i++) {
            int num;
            fscanf(stdin, "%d", &num);

            tot += (prev > num ? prev - num : 0);
            prev = num;
        }

        printf("%llu\n", tot);

    }

    return 0;
}


