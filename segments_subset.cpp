
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

    class Node {
    private:

        typedef std::unordered_set<NodePtr, NodePtrHash> hashset_t;

        std::set<NodePtr> children;

        LineSeg seg;

        void insert(NodePtr node, hashset_t & visited) {
            bool add = true;
            for (auto it = children.begin(); it != children.end();) {
                NodePtr o = *it;

                if (*o == *node) {
                    it++;
                    continue;
                }

                auto it2 = visited.find(o);
                bool seen = (it2 != visited.end());
                if (!seen) {
                    visited.insert(o);
                }

                if (node->seg.contains(o->seg)) {
                    //printf("Erase (%d, %d) from (%d, %d)\n", o->seg.beg_idx, o->seg.end_idx, seg.beg_idx, seg.end_idx);
                    it = children.erase(it);
                    hashset_t visited2;
                    node->insert(o, visited2);
                }
                else if (o->seg.contains(node->seg)) {
                    add = false;
                    if (!seen) {
                        o->insert(node, visited);
                    }
                    it++;
                }
                else {
                    it++;
                }
            }

            if (add) {
                children.insert(node);
            }
        }

        void _print(int depth, hashset_t & visited, int & tot, size_t & n_childs) const {
            tot++;
            n_childs += children.size();
            printf("%*s(%d, %d)\n", depth * 2, "", seg.beg_idx, seg.end_idx);
            for (const NodePtr p : children) {

                auto it2 = visited.find(p);
                if (it2 != visited.end()) {
                    printf("\033[0;36m%*s(%d, %d)\033[0;39m\n", (depth + 1) * 2, "", p->seg.beg_idx, p->seg.end_idx);
                    continue;
                }
                else {
                    visited.insert(p);
                }

                p->_print(depth + 1, visited, tot, n_childs);
            }
        }

    public:

        Node() : seg{ LineSeg::min_val, LineSeg::max_val } {}
        Node(const LineSeg & seg) : seg(seg) {}

        void insert(const LineSeg & seg) {
            hashset_t visited;
            NodePtr node(seg);
            this->insert(node, visited);
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

        void validate() {
            for (NodePtr p : children) {
                assert(!(*p == *this));
                for (auto it = children.upper_bound(p); it != children.end(); it++) {
                    if (p->seg.contains((*it)->seg)) {
                        printf("seg (%d, %d) contains (%d, %d)\n", p->seg.beg_idx, p->seg.end_idx, (*it)->seg.beg_idx, (*it)->seg.end_idx);
                    }
                    if ((*it)->seg.contains(p->seg)) {
                        printf("seg (%d, %d) contains (%d, %d)\n", (*it)->seg.beg_idx, (*it)->seg.end_idx, p->seg.beg_idx, p->seg.end_idx);
                    }
                    assert(!p->seg.contains((*it)->seg));
                    assert(!(*it)->seg.contains(p->seg));
                }

                p->validate();
            }
        }
    };

    Node root;

public:

    void insert(const LineSeg & seg) {
        root.insert(seg);
    }

    void print() const {
        root.print();
    }

    void validate() {
        root.validate();
    }

};





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

            printf("\non %d, insert (%d, %d)\n", i, l, r);

            s.insert({ l, r });
            //s.print();
            //s.validate();
        }

        printf("go\n");
        s.print();

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

