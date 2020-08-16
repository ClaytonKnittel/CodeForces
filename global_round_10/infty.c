
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int main(int argc, char * argv[]) {

    int t, n;
    uint64_t k;

    fscanf(stdin, "%d", &t);
    
    while (t--) {
        fscanf(stdin, "%d %llu", &n, &k);

        // only care about parity, k cannot be 0
        k &= 1;

        int * arr = (int *) malloc(n * sizeof(int));

        int min = 1000000001, max = -1000000001;
        for (int i = 0; i < n; i++) {
            int num;
            fscanf(stdin, "%d", &num);

            arr[i] = num;

            min = (min < num ? min : num);
            max = (max > num ? max : num);
        }

        if (k) {
            // k is odd
            for (int i = 0; i < n; i++) {
                printf("%d ", max - arr[i]);
            }
        }
        else {
            // k is even
            for (int i = 0; i < n; i++) {
                printf("%d ", arr[i] - min);
            }
        }
        printf("\n");



    }

    return 0;
}


