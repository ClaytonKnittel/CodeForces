
#include <cstdint>
#include <cstdio>
#include <cassert>

#include <algorithm>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

#define MAX_N 3000


struct LineSeg {
    int beg_idx, end_idx;

    static constexpr int32_t min_val = 1;
    static constexpr int32_t max_val = 200000;


    bool contains(const LineSeg & o) const {
        return beg_idx <= o.beg_idx && end_idx >= o.end_idx;
    }

    bool operator==(const LineSeg & other) const {
        return beg_idx == other.beg_idx && end_idx == other.end_idx;
    }

    bool operator!=(const LineSeg & other) const {
        return beg_idx != other.beg_idx || end_idx != other.end_idx;
    }

    bool operator<(const LineSeg & o) const {
        return beg_idx < o.beg_idx;
    }
};


class SegTree {
private:

    class Node;

    class NodePtr {
    public:
        std::shared_ptr<Node> node;

        explicit NodePtr(const Node & n) {
            node = std::make_shared<Node>(n);
        }

        Node &operator*() {
            return *node;
        }

        const Node &operator*() const {
            return *node;
        }

        Node * operator->() {
            return &(*node);
        }

        const Node * operator->() const {
            return &(*node);
        }

        bool operator<(const NodePtr & o) const {
            return (*node) < (*o.node);
        }

        bool operator==(const NodePtr & o) const {
            return (**this) == (*o);
        }
    };

    struct NodePtrHash {
        std::size_t operator()(const NodePtr & ptr) const {
            size_t val = reinterpret_cast<uint64_t>(&(*ptr));
            val = (~val) + (val << 18);
            val ^= (val >> 31);
            val *= 21;
            val &= (val >> 11);
            val += val << 6;
            val ^= val >> 22;
            return val;
        }
    };
    typedef std::unordered_set<NodePtr, NodePtrHash> hashset_t;

    class Node {
    private:

        uint64_t visited;
        uint64_t count_cache;

        std::vector<NodePtr> children;

        LineSeg seg;

        void insert(NodePtr node, uint64_t tag) {
            bool add = true;
            for (auto it = children.begin(); it != children.end();) {
                NodePtr o = *it;

                if (*o == *node) {
                    add = false;
                    it++;
                    continue;
                }

                bool seen = (o->visited == tag);
                if (!seen) {
                    o->visited = tag;
                }

                if (node->seg.contains(o->seg)) {
                    it = children.erase(it);
                    if (add) {
                        children.push_back(node);
                        add = false;
                    }
                    node->insert(o, vcount++);
                }
                else if (o->seg.contains(node->seg)) {
                    add = false;
                    if (!seen) {
                        o->insert(node, tag);
                    }
                    it++;
                }
                else {
                    it++;
                }
            }

            if (add) {
                children.push_back(node);
            }
        }

        void _print(int depth, hashset_t & visited, int & tot, size_t & n_childs) const {
            tot++;
            n_childs += children.size();
            printf("%*s(%d, %d) (%llu)\n", depth * 2, "", seg.beg_idx, seg.end_idx, count_cache);
            for (const NodePtr p : children) {

                auto it2 = visited.find(p);
                if (it2 != visited.end()) {
                    printf("\033[0;36m%*s(%d, %d) (%llu)\033[0;39m\n", (depth + 1) * 2, "", p->seg.beg_idx, p->seg.end_idx, p->count_cache);
                    continue;
                }
                else {
                    visited.insert(p);
                }

                p->_print(depth + 1, visited, tot, n_childs);
            }
        }

    public:

        Node() : visited(0), seg{ LineSeg::min_val - 1, LineSeg::max_val + 1 } {}
        Node(const LineSeg & seg) : seg(seg) {}

        void insert(const LineSeg & seg, uint64_t tag) {
            NodePtr node(seg);
            this->insert(node, tag);
        }

        bool operator<(const Node & n) const {
            return seg < n.seg;
        }

        bool operator==(const Node & n) const {
            return seg == n.seg;
        }
        void print() const {
            int tot = 0;
            size_t n_children = 0;
            hashset_t visited;
            this->_print(0, visited, tot, n_children);

            printf("Total: %d\nTotal num children: %zu\navg n children: %f\n", tot, n_children, n_children / (static_cast<double>(tot)));
        }

        void validate(hashset_t & visited) {
            for (size_t i = 0; i < children.size(); i++) {
                NodePtr p = children[i];
                for (size_t j = i + 1; j < children.size(); j++) {
                    NodePtr q = children[j];
                    assert(p->seg.beg_idx < q->seg.beg_idx);
                    assert(p->seg.end_idx < q->seg.end_idx);
                    if (p->seg.contains(q->seg)) {
                        printf("seg (%d, %d) contains (%d, %d)\n", p->seg.beg_idx, p->seg.end_idx, q->seg.beg_idx, q->seg.end_idx);
                    }
                    if (q->seg.contains(p->seg)) {
                        printf("seg (%d, %d) contains (%d, %d)\n", q->seg.beg_idx, q->seg.end_idx, p->seg.beg_idx, p->seg.end_idx);
                    }
                    assert(!p->seg.contains(q->seg));
                    assert(!q->seg.contains(p->seg));
                }
                auto it = visited.find(p);
                if (it == visited.end()) {
                    visited.insert(p);
                    p->validate(visited);
                }
            }
        }

        void sort(uint64_t tag) {
            std::sort(children.begin(), children.end());
            for (size_t i = 0; i < children.size(); i++) {
                NodePtr p = children[i];
                if (p->visited != tag) {
                    p->visited = tag;
                    p->sort(tag);
                }
            }
        }

        uint64_t count(uint64_t tag) {
            if (visited == tag) {
                return count_cache;
            }
            visited = tag;

            uint32_t csize = children.size();

            if (csize == 0) {
                count_cache = 1;
            }
            else {
                uint64_t *cnts = new uint64_t[csize];
                int32_t j = -1;
                for (int32_t i = 0; i < csize; i++) {
                    uint32_t beg_idx = children[i]->seg.beg_idx;
                    while (j + 1 < i && children[j + 1]->seg.end_idx < beg_idx) {
                        j++;
                    }
                    uint64_t with_cnt = children[i]->count(tag) + (j >= 0 ? cnts[j] : 0);
                    uint64_t sin_cnt  = (i > 0 ? cnts[i - 1] : 0);
                    cnts[i] = std::max(with_cnt, sin_cnt);
                }

                count_cache = 1 + cnts[csize - 1];

                delete [] cnts;
            }
            return count_cache;
        }
    };

    Node root;

public:

    static uint64_t vcount;

    void insert(const LineSeg & seg) {
        uint64_t tag = vcount;
        vcount++;
        root.insert(seg, tag);
    }

    void print() const {
        root.print();
    }

    void validate() {
        hashset_t visited;
        root.validate(visited);
    }

    void sort_all() {
        uint64_t tag = vcount;
        vcount++;
        root.sort(tag);
    }

    uint64_t get_count() {
        uint64_t tag = vcount;
        vcount++;
        // subtract one for imaginary node [0, 200001] at root
        return root.count(tag) - 1;
    }

};


uint64_t SegTree::vcount = 1;





int main(int argc, char * argv[]) {

    int test_cases;
    int n;

    assert(fscanf(stdin, "%d", &test_cases) == 1);

    for (int t = 0; t < test_cases; t++) {
        assert(fscanf(stdin, "%d", &n) == 1);

        SegTree s;

        for (int i = 0; i < n; i++) {
            int32_t l, r;
            fscanf(stdin, "%d %d", &l, &r);
            assert(l <= r);

            //printf("\non %d, insert (%d, %d)\n", i, l, r);

            s.insert({ l, r });
            //s.print();
        }


        s.sort_all();
        s.get_count();
        s.print();
        s.validate();

        //printf("Count: %llu\n", s.get_count());
        printf("%llu\n", s.get_count());

        /*
        printf("size: %zu\n", s.size());
        for (const LineSeg & seg : s) {
            printf("(%d, %d)\n", seg.beg_idx, seg.end_idx);
        }

        int tot = 0;
        for (const LineSeg & s : all) {
            for (const LineSeg & s2 : all) {
                int x = 1;
                tot += x;
            }
            int cnt = 0;
            for (auto it = v.begin(); it != v.end(); it++) {
                if (it->beg_idx <= s.beg_idx && it->end_idx >= s.end_idx) {
                    //printf("  (%d, %d) in (%d, %d)\n", s.beg_idx, s.end_idx, it->beg_idx, it->end_idx);
                    cnt++;
                }
            }
            assert(cnt >= 1);
        }*/

        //std::sort(seg_begins, seg_begins + n);

        /*
        printf("begins\n");
        for (int i = 0; i < n; i++) {
            printf("(%u, %u) %d\n", seg_begins[i].l, seg_begins[i].r, seg_begins[i].seg_idx);
        }
        */

        //map_t m;
        //int32_t res = find_starting_at(1, 200000, -1, n, m, seg_begins, 1);

        //printf("%d\n", res);
    }
}

