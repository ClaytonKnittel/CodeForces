
#include <cassert>

#include <iostream>
#include <vector>


#define MAX_N 50

int main(int argc, char *argv[]) {
    int t, n;

    std::cin >> t;

    int cnts[MAX_N];

    for (; t > 0; t--) {
        std::cin >> n;

        assert(n <= MAX_N);

        __builtin_memset(cnts, 0, n * sizeof(int));

        for (int i = 0; i < n; i++) {
            int w;
            std::cin >> w;
            cnts[w - 1]++;
        }

        /*for (int w = 0; w < n; w++) {
            printf("w = %d, cnt = %d\n", w + 1, cnts[w]);
        }*/

        int max_n_teams = 0;
        for (int tot = 0; tot < 2 * n - 1; tot++) {
            int n_teams = 0;
            for (int i = std::max(0, tot + 1 - n); i < std::min(n, (tot + 1) / 2); i++) {
                int j = tot - i;
                assert(j >= 0);
                assert(i < j);
                assert(j < n);
                n_teams += std::min(cnts[i], cnts[j]);
            }
            n_teams += (!(tot & 1) ? cnts[tot / 2] / 2 : 0);
            //printf("tot: %d\tmin: %d\tmax: %d\tn_teams: %d\n", tot, std::max(0, tot + 1 - n), std::min(n, (tot + 1) / 2) - 1, n_teams);
            max_n_teams = std::max(n_teams, max_n_teams);
        }

        printf("%d\n", max_n_teams);

    }
}

