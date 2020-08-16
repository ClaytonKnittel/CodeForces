
#include <cassert>

#include <iostream>
#include <iterator>
#include <queue>
#include <vector>


int main(int argc, char * argv[]) {

    int t, n;

    std::cin >> t;
    while (t--) {
        std::cin >> n;

        std::queue<int> zeros, ones;

        std::vector<size_t> seqs;

        while (n--) {
            char dig;
            std::cin >> dig;

            size_t seq_num;
            if (dig == '1') {
                if (zeros.size() > 0) {
                    seq_num = zeros.front();
                    zeros.pop();
                }
                else {
                    seq_num = zeros.size() + ones.size();
                }
                ones.push(seq_num);
            }
            else {
                if (ones.size() > 0) {
                    seq_num = ones.front();
                    ones.pop();
                }
                else {
                    seq_num = zeros.size() + ones.size();
                }
                zeros.push(seq_num);
            }
            seqs.push_back(seq_num + 1);
        }

        std::cout << zeros.size() + ones.size() << std::endl;
        std::copy(seqs.begin(), seqs.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }
}

