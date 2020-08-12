
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>


#include <algorithm>
#include <stdexcept>
#include <vector>
#include <stack>

template <class _Ty>
struct _Node
{
    typedef _Node* _Nodeptr;
    _Node(const _Ty& right) : _Val(right)
    {
        _Left = _Next = _Parent = nullptr;
        _Rank = 0;
    }
    _Node(_Ty&& _Val) : _Val(std::move(_Val))
    {
        _Left = _Next = _Parent = nullptr;
        _Rank = 0;
    }
    _Ty _Val;
    _Nodeptr _Left, _Next, _Parent;
    int _Rank;
private:
    _Node& operator=(const _Node&);
};

template <class _Myheap>
class _Iterator
{
public:
    friend _Myheap;
    typedef typename _Myheap::_Nodeptr _Nodeptr;
    typedef typename _Myheap::value_type value_type;
    typedef typename _Myheap::difference_type difference_type;
    typedef typename _Myheap::const_reference const_reference;
    typedef typename _Myheap::const_pointer const_pointer;

    _Iterator(_Nodeptr _Ptr = nullptr)
    {
        this->_Ptr = _Ptr;
    }
    const_reference operator*() const
    {
        return this->_Ptr->_Val;
    }
    const_pointer operator->() const
    {
        return &(operator*());
    }
    _Nodeptr _Ptr;
};

template <class _Ty, class _Pr = std::less<_Ty>, class _Alloc = std::allocator<_Ty>>
class rp_heap
{
public:
    typedef rp_heap<_Ty, _Pr, _Alloc> _Myt;
    typedef _Node<_Ty> _Node;
    typedef _Node* _Nodeptr;

    typedef _Pr key_compare;

    typedef _Alloc allocator_type;
    typedef typename _Alloc::template rebind<_Node>::other _Alty;
    typedef typename _Alloc::value_type value_type;
    typedef typename _Alloc::pointer pointer;
    typedef typename _Alloc::const_pointer const_pointer;
    typedef typename _Alloc::reference reference;
    typedef typename _Alloc::const_reference const_reference;
    typedef typename _Alloc::difference_type difference_type;
    typedef typename _Alloc::size_type size_type;

    typedef _Iterator<_Myt> const_iterator;

    rp_heap(const _Pr& _Pred = _Pr()) : comp(_Pred)
    {
        _Mysize = 0;
        _Myhead = nullptr;
    }

    ~rp_heap()
    {
        clear();
    }

    bool empty() const
    {
        return _Mysize == 0;
    }

    size_type size() const
    {
        return _Mysize;
    }

    const_reference top() const
    {
        return _Myhead->_Val;
    }

    const_iterator push(const value_type& _Val)
    {
        _Nodeptr _Ptr = _Alnod.allocate(1);
        _Alnod.construct(_Ptr, _Val);
        _Insert_root(_Ptr);
        _Mysize++;
        return const_iterator(_Ptr);
    }

    const_iterator push(value_type&& x)
    {
        _Nodeptr _Ptr = _Alnod.allocate(1);
        _Alnod.construct(_Ptr, std::forward<value_type>(x));
        _Insert_root(_Ptr);
        _Mysize++;
        return const_iterator(_Ptr);
    }

    void pop() //delete min
    {
        if (empty())
            throw std::runtime_error("pop error: empty heap");
        std::vector<_Nodeptr> _Bucket(_Max_bucket_size(), nullptr);
        // assert_children(_MinRoot);
        for (_Nodeptr _Ptr = _Myhead->_Left; _Ptr; )
        {
            _Nodeptr _NextPtr = _Ptr->_Next;
            _Ptr->_Next = nullptr;
            _Ptr->_Parent = nullptr;
            _Multipass(_Bucket, _Ptr);
            _Ptr = _NextPtr;
        }
        for (_Nodeptr _Ptr = _Myhead->_Next; _Ptr != _Myhead; )
        {
            _Nodeptr _NextPtr = _Ptr->_Next;
            _Ptr->_Next = nullptr;
            _Multipass(_Bucket, _Ptr);
            _Ptr = _NextPtr;
        }
        _Freenode(_Myhead);
        _Myhead = nullptr;
        std::for_each(_Bucket.begin(), _Bucket.end(), [&](_Nodeptr _Ptr)
        {
            if (_Ptr)
                _Insert_root(_Ptr);
        });
    }

    void pop(value_type& _Val)
    {
        if (empty())
            throw std::runtime_error("pop error: empty heap");
        _Val = std::move(_Myhead->_Val);
        pop();
    }

    void clear()
    {
        // while (!empty())
        //     pop();
        // post order traversal using two stacks
        if (!empty())
        {
            std::stack<_Nodeptr> _Stack_in, _Stack_out;
            _Stack_in.push(_Myhead);
            while (!_Stack_in.empty())
            {
                _Nodeptr _Ptr = _Stack_in.top();
                _Stack_in.pop();
                _Stack_out.push(_Ptr);
                if (_Ptr->_Left)
                    _Stack_in.push(_Ptr->_Left);
                if (_Ptr->_Next && _Ptr->_Next != _Myhead)
                    _Stack_in.push(_Ptr->_Next);
            }
            while (!_Stack_out.empty())
            {
                _Nodeptr _Ptr = _Stack_out.top();
                _Freenode(_Ptr);
                _Stack_out.pop();
            }
        }
        _Myhead = nullptr;
        //assert(empty());
    }

    void decrease(const_iterator _It, const value_type& _Val)
    {
        _Nodeptr _Ptr = _It._Ptr;
        if (comp(_Val, _Ptr->_Val))
            _Ptr->_Val = _Val;
        if (_Ptr == _Myhead)
            return;
        if (_Ptr->_Parent == nullptr) //one of the roots
        {
            if (comp(_Ptr->_Val, _Myhead->_Val))
                _Myhead = _Ptr;
        }
        else
        {
            _Nodeptr _ParentPtr = _Ptr->_Parent;
            if (_Ptr == _ParentPtr->_Left)
            {
                _ParentPtr->_Left = _Ptr->_Next;
                if (_ParentPtr->_Left)
                    _ParentPtr->_Left->_Parent = _ParentPtr;
            }
            else
            {
                _ParentPtr->_Next = _Ptr->_Next;
                if (_ParentPtr->_Next)
                    _ParentPtr->_Next->_Parent = _ParentPtr;
            }
            // assert_children(_ParentPtr);
            _Ptr->_Next = _Ptr->_Parent = nullptr;
            _Ptr->_Rank = (_Ptr->_Left) ? _Ptr->_Left->_Rank + 1 : 0;
            // assert_half_tree(_Ptr);
            _Insert_root(_Ptr);
            //type-2 rank reduction
            if (_ParentPtr->_Parent == nullptr) // is a root
                _ParentPtr->_Rank = (_ParentPtr->_Left) ? _ParentPtr->_Left->_Rank + 1 : 0;
            else
            {
                while (_ParentPtr->_Parent)
                {
                    int i = _ParentPtr->_Left ? _ParentPtr->_Left->_Rank : -1;
                    int j = _ParentPtr->_Next ? _ParentPtr->_Next->_Rank : -1;
#ifdef TYPE1_RANK_REDUCTION
                    int k = (i != j) ? std::max(i, j) : i + 1; //type-1 rank reduction
#else
                    int k = (abs(i - j) > 1) ? std::max(i, j) : std::max(i, j) + 1; //type-2 rank reduction
#endif // TYPE1_RANK_REDUCTION
                    if (k >= _ParentPtr->_Rank)
                        break;
                    _ParentPtr->_Rank = k;
                    _ParentPtr = _ParentPtr->_Parent;
                }
            }
        }
    }

private:

    void _Freenode(_Nodeptr _Ptr)
    {
        _Alnod.destroy(_Ptr);
        _Alnod.deallocate(_Ptr, 1);
        _Mysize--;
    }

    // void assert_half_tree(_Nodeptr _Ptr)
    // {
    //     assert(_Ptr->_Next == nullptr && _Ptr->_Parent == nullptr);
    // }

    // void assert_parent(_Nodeptr _Ptr)
    // {
    //     assert(_Ptr->_Parent->_Left == _Ptr || _Ptr->_Parent->_Next == _Ptr);
    // }

    // void assert_children(_Nodeptr _Ptr)
    // {
    //     if (_Ptr->_Left)
    //         assert(_Ptr->_Left->_Parent == _Ptr);
    //     if (_Ptr->_Next)
    //         assert(_Ptr->_Next->_Parent == _Ptr);
    // }

    void _Insert_root(_Nodeptr _Ptr)
    {
        if (_Myhead == nullptr)
        {
            _Myhead = _Ptr;
            _Ptr->_Next = _Ptr;
        }
        else
        {
            _Ptr->_Next = _Myhead->_Next;
            _Myhead->_Next = _Ptr;
            if (comp(_Ptr->_Val, _Myhead->_Val))
                _Myhead = _Ptr;
        }
    }

    _Nodeptr _Link(_Nodeptr _Left, _Nodeptr _Right)
    {
        if (_Right == nullptr)
            return _Left;
        // assert_half_tree(_Left);
        // assert_half_tree(_Right);
        _Nodeptr _Winner, _Loser;
        if (comp(_Right->_Val, _Left->_Val))
        {
            _Winner = _Right;
            _Loser = _Left;
        }
        else
        {
            _Winner = _Left;
            _Loser = _Right;
        }
        _Loser->_Parent = _Winner;
        if (_Winner->_Left)
        {
            _Loser->_Next = _Winner->_Left;
            _Loser->_Next->_Parent = _Loser;
        }
        _Winner->_Left = _Loser;
        _Winner->_Rank = _Loser->_Rank + 1;
        // assert_children(_Winner);
        // assert_parent(_Loser);
        // assert_half_tree(_Winner);
        return _Winner;
    }

    inline size_type _Max_bucket_size() //ceil(log2(size)) + 1
    {
        size_type _Bit = 1, _Count = _Mysize;
        while (_Count >>= 1)
            _Bit++;
        return _Bit + 1;
    }

    template <class _Container>
    void _Multipass(_Container& _Bucket, _Nodeptr _Ptr)
    {
        // if ((size_t)_Ptr->_Rank >= _Bucket.size())
        // {
        //     _Bucket.resize(_Ptr->_Rank + 1, nullptr);
        // }
        // else
        // {
        while (_Bucket[_Ptr->_Rank] != nullptr)
        {
            unsigned int _Rank = _Ptr->_Rank;
            _Ptr = _Link(_Ptr, _Bucket[_Rank]);
            // assert_children(_Ptr);
            // assert_half_tree(_Ptr);
            _Bucket[_Rank] = nullptr;
            // if ((size_t)_Ptr->_Rank >= _Bucket.size())
            // {
            //     _Bucket.resize(_Ptr->_Rank + 1, nullptr);
            //     break;
            // }
        }
        // }
        _Bucket[_Ptr->_Rank] = _Ptr;
    }

    _Pr comp;
    _Nodeptr _Myhead;
    size_type _Mysize;
    _Alty _Alnod;
};





#ifdef __LP64__
typedef uint64_t ptr_t;
#else
typedef uint32_t ptr_t;
#endif



typedef struct edge {
    uint32_t w;
    uint32_t to;
    uint32_t edge_idx;
} edge_t;

class val_edge_t {
private:
    uint32_t w;
    // number of children this edge lies on the path to from the root
    uint32_t n_children;

    uint32_t dif;

    int edge_idx;

    void set_dif() {
        dif = ((w + 1) / 2) * n_children;
    }

public:

    val_edge_t() : w(0), n_children(0), dif(0), edge_idx(-1) {}

    int get_edge_idx() const {
        return edge_idx;
    }
    uint32_t get_w() const {
        return w;
    }
    uint32_t get_n_children() const {
        return n_children;
    }
    uint32_t get_dif() const {
        return dif;
    }

    void set_edge_idx(int idx) {
        this->edge_idx = idx;
    }

    void set_w(uint32_t new_w) {
        this->w = new_w;
        this->set_dif();
    }

    void set_n_children(uint32_t new_n) {
        this->n_children = new_n;
        this->set_dif();
    }

    bool operator<(const val_edge_t & other) const {
        if (this->dif == other.dif) {
            return this->edge_idx > other.edge_idx;
        }
        return this->dif > other.dif;
    }

    bool operator==(const val_edge_t & other) const {
        return this->edge_idx == other.edge_idx;
    }
};


typedef struct dylist {
    uint32_t len;
    uint32_t cap;
    edge_t * ptr;
} dylist_t;

static void dylist_push(dylist_t * l, edge_t val) {
    uint32_t cap = l->cap;
    if (cap == l->len) {
        cap = (cap << 1) | !cap;
        l->cap = cap;
        edge_t * new_ptr = (edge_t *) realloc(l->ptr, cap * sizeof(edge_t));
        l->ptr = new_ptr;
    }
    l->ptr[l->len] = val;
    l->len++;
}

static void dylist_delete(dylist_t * l) {
    free(l->ptr);
}


typedef struct node {
    dylist_t edges;

    // exactly one of these edges is to the parent
    uint32_t par_edge_idx;
} node_t;


static void add_edge(node_t * nodes, uint32_t v, uint32_t u, uint32_t w,
        uint32_t edge_idx) {
    edge_t ev = {
        .w = w,
        .to = u,
        .edge_idx = edge_idx
    };
    edge_t eu = {
        .w = w,
        .to = v,
        .edge_idx = edge_idx
    };
    dylist_push(&nodes[v].edges, ev);
    dylist_push(&nodes[u].edges, eu);
}


static uint32_t resolve_edges(val_edge_t * edges, const node_t * nodes, uint32_t idx, uint32_t parent) {
    uint32_t num_paths_thru = 0;

    const node_t * node = &nodes[idx];

    //printf("node idx %u, parent %u\n", idx, parent);

    for (uint32_t i = 0; i < node->edges.len; i++) {
        edge_t * e = &node->edges.ptr[i];
        if (e->to == parent) {
            continue;
        }

        uint32_t num_paths = resolve_edges(edges, nodes, e->to, idx);

        edges[e->edge_idx].set_w(e->w);
        edges[e->edge_idx].set_n_children(num_paths);
        edges[e->edge_idx].set_edge_idx(e->edge_idx);

        num_paths_thru += num_paths;
    }

    // return num_paths_thru if this isn't a leaf, otherwise 1
    return num_paths_thru | !num_paths_thru;
}


int main(int argc, char * argv[]) {

    int test_cases;
    int n, S;

    assert(fscanf(stdin, "%d", &test_cases) == 1);

    for (int t = 0; t < test_cases; t++) {
        assert(fscanf(stdin, "%d %d", &n, &S) == 2);

        node_t * nodes = (node_t *) calloc(n, sizeof(node_t));
        //val_edge_t * edges = (val_edge_t *) malloc((n - 1) * sizeof(val_edge_t));
        val_edge_t * edges = new val_edge_t[n - 1];

        for (int e = 0; e < n - 1; e++) {
            int v, u;
            uint32_t w;
            assert(fscanf(stdin, "%d %d %u", &v, &u, &w) == 3);

            add_edge(nodes, v - 1, u - 1, w, e);
        }

        /*
        for (int i = 0; i < n; i++) {
            printf("%u\t(", i);
            for (int j = 0; j < nodes[i].edges.len; j++) {
                printf("%u (w=%u, to=%u)", nodes[i].edges.ptr[j].edge_idx, nodes[i].edges.ptr[j].w, nodes[i].edges.ptr[j].to);
                if (j != nodes[i].edges.len - 1) {
                    printf(", ");
                }
            }
            printf(")\n");
        }
        */

        // set parent value to n, since no node has index n (node 0 has no parent)
        resolve_edges(edges, nodes, 0, n);

        for (int i = 0; i < n; i++) {
            dylist_delete(&nodes[i].edges);
        }
        free(nodes);

        uint64_t tot_w = 0;
        uint32_t n_actions = 0;

        rp_heap<val_edge_t> h;

        for (int i = 0; i < n - 1; i++) {
            tot_w += (uint64_t) (edges[i].get_w() * edges[i].get_n_children());
            h.push(edges[i]);
        }
        fprintf(stderr, "heaped\n");

        /*for (int i = 0; i < n - 1; i++) {
            printf("(%u)\t%u %u\n", i, edges[i].get_w(), edges[i].get_n_children());
        }*/


        while (tot_w > S) {
            val_edge_t e = h.top();
            h.pop();
            uint64_t p_tot_w = tot_w;
            tot_w -= e.get_dif();

            /*printf("edge (%u -> %u) (%u)\n", e.get_w(), e.get_w() / 2, e.get_dif());
            printf("total from %llu -> %llu\n", p_tot_w, tot_w);*/

            e.set_w(e.get_w() / 2);

            h.push(e);

            n_actions++;
        }
        fprintf(stderr, "calcd\n");

        delete [] edges;

        printf("%u\n", n_actions);
    }

    return 0;
}

