
#include <cstdint>
#include <cstdio>
#include <cassert>

#include <algorithm>
#include <unordered_map>
#include <vector>

#define MAX_N 3000


struct LineSeg {
    int beg_idx, end_idx;

    bool operator==(const LineSeg & other) const {
        return beg_idx == other.beg_idx && end_idx == other.end_idx;
    }
};


namespace std {

    template <>
    struct hash<LineSeg> {

        std::size_t operator() (const LineSeg &seg) const {
            uint64_t bh = (((uint64_t) seg.beg_idx) * 2654435761LU) & 0xffffffffLU;
            uint64_t eh = (((uint64_t) seg.end_idx) * 2654435761LU) & 0xffffffffLU;
            return bh | (eh << 32);
        }
    };

}


class SegSet {
private:

    /*
     *
     * recursively divide space like
     *
     * +-------------/
     * | 1  /  | 3 /
     * | /  2  | /
     * |-------/
     * |  4  /
     * |   /
     * | /
     * /
     */

    //constexpr int32_t min_val = 1;
    //constexpr int32_t max_val = 200000;
    static constexpr int32_t max_val = 0x40000;


    class Node {
    public:
        
    };

public:

    SegSet() {}
    ~SegSet() {}

    void insert(const LineSeg & seg) {
    }
};


typedef std::unordered_map<LineSeg, int32_t> map_t;


struct LineSegBegin {
    int32_t l, r;

    // sort first by starting pos, then in reverse order by ending pos
    bool operator<(const LineSegBegin & o) const {
        return l == o.l ? r > o.r : l < o.l;
    }
};


/*
 * find index of first segment that begins at or after given location
 */
int32_t find_first_at(int32_t loc, int32_t n, LineSegBegin * seg_begins) {
    int32_t l = 0, r = n;

    while (l < r) {
        int32_t m = (l + r) >> 1;
        int32_t begin = seg_begins[m].l;

        if (begin < loc) {
            l = m + 1;
        }
        else {
            r = m;
        }
    }
    return l;
}


/*
 * l and r are the bounds we are searching in, i.e [l, r]
 */
int32_t find_starting_at(int32_t l, int32_t r, int32_t fidx, int32_t n, map_t &m,
        LineSegBegin * seg_begins, int allow_same) {

    int32_t max_n_possible = 0;

    auto it = m.find({ l, r });
    if (it != m.end()) {
        return it->second;
    }

    int32_t min_covered_to = r;

    // iterate through all possible starting points
    int32_t first_idx = fidx == -1 ? find_first_at(l, n, seg_begins) : fidx;

    while (first_idx < n) {
        const LineSegBegin &seg = seg_begins[first_idx];
        if (seg.l > min_covered_to) {
            // we are done, is always optimal to just take interval
            // before this one that ended before it began and also take
            // this one, or don't take at all
            break;
        }
        if (seg.r > r) {
            first_idx++;
            continue;
        }
        if (!allow_same && (seg.l == l && seg.r == r)) {
            first_idx++;
            continue;
        }
        //printf(" %d %d\n", seg.l, seg.r);
        int32_t n_possible_after = find_starting_at(seg.r + 1, r, -1, n, m, seg_begins, 1);
        int32_t n_possible_within = find_starting_at(seg.l, seg.r, first_idx, n, m, seg_begins, 0);
        max_n_possible = std::max(max_n_possible, n_possible_after + n_possible_within + 1);
        min_covered_to = std::min(min_covered_to, seg.r);

        // now skip all segments starting at this location
        while (++first_idx < n && seg_begins[first_idx].l == seg.l);
        //first_idx = find_first_at(seg.l + 1, n, seg_begins);
    }

    m.insert(std::make_pair(LineSeg{ l, r }, max_n_possible));

    //printf("%d, %d (%d)\n", l, r, max_n_possible);

    return max_n_possible;
}


int main(int argc, char * argv[]) {

    int test_cases;
    int n;

    // to track results so no recomputation is done, starting_at holds
    // the number of sets we can fit starting from this one (in terms of
    // l), including only all intervals with l >= this one's l
    int32_t starting_at[MAX_N];
    // within holds number of sets we can fit within this one (including this one)
    int32_t within[MAX_N];

    LineSegBegin seg_begins[MAX_N];

    assert(fscanf(stdin, "%d", &test_cases) == 1);

    for (int t = 0; t < test_cases; t++) {
        assert(fscanf(stdin, "%d", &n) == 1);

        // initialize all to -1 (not calculated)
        __builtin_memset(starting_at, -1, MAX_N * sizeof(int32_t));
        __builtin_memset(within, -1, MAX_N * sizeof(int32_t));

        for (int i = 0; i < n; i++) {
            int32_t l, r;
            fscanf(stdin, "%d %d", &l, &r);

            seg_begins[i] = { l, r };
        }

        std::sort(seg_begins, seg_begins + n);

        /*
        printf("begins\n");
        for (int i = 0; i < n; i++) {
            printf("(%u, %u) %d\n", seg_begins[i].l, seg_begins[i].r, seg_begins[i].seg_idx);
        }
        */

        map_t m;
        int32_t res = find_starting_at(1, 200000, -1, n, m, seg_begins, 1);

        printf("%d\n", res);
    }
}

