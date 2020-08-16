
#include <stdio.h>



int main(int argc, char * argv[]) {

    int t, n;

    fscanf(stdin, "%d", &t);
    
    while (t--) {
        fscanf(stdin, "%d", &n);

        int all_same = 1;

        int num;
        fscanf(stdin, "%d", &num);
        for (int i = 1; i < n; i++) {
            int num2;
            fscanf(stdin, "%d", &num2);

            all_same = all_same && (num == num2);
        }

        printf("%d\n", all_same ? n : 1);
    }

    return 0;
}


