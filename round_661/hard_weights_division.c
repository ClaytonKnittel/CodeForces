
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int64_t min(int64_t a, int64_t b) {
    return a < b ? a : b;
}



typedef int64_t heap_key_t;

#ifdef __LP64__
typedef uint64_t ptr_t;
#else
typedef uint32_t ptr_t;
#endif

#define HEAP_ASSERT(expr)

typedef struct heap_node {
    struct heap_node * lchild;
    struct heap_node * rsib;
    heap_key_t key;
} heap_node;


#define HEAP_NODE_SET(heap_node, k) \
    ((heap_node)->key = ((heap_key_t) k))


typedef struct min_heap {
    heap_node * root;
} heap_t;



int heap_init(heap_t *h);

void heap_destroy(heap_t *h);


/*
 * returns a pointer to the heap node at the top of the heap (having minimum
 * key value of all heap nodes)
 */
heap_node * heap_find_min(heap_t *h);

/*
 * deletes the min heap node from the heap
 */
void heap_delete_min(heap_t *h);

/*
 * combines the effect of find_min and delete_min, returning the deleted min
 */
heap_node * heap_extract_min(heap_t *h);


/*
 * inserts a node into the tree. The node must already be initialized, i.e. its
 * key must be set
 *
 * returns 0 on success, nonzero if fails
 */
int heap_insert(heap_t *h, heap_node * node);


/*
 * joins heaps h1 and h2 into one heap, which is stored in the h1 struct. Both
 * heaps h1 and h2 are corrupted by this operation, i.e. they would need to be
 * cloned if you wanted to access them after doing this operation
 */
int heap_meld(heap_t *h1, heap_t *h2);

/*
 * decreases key value of node to the new key value
 */
int heap_decrease_key(heap_t *h, heap_node * node, heap_key_t new_key);

/*
 * increases key value of node to the new key value
 */
int heap_increase_key(heap_t *h, heap_node * node, heap_key_t new_key);

/*
 * deletes a node from the heap, assuming the node is actually in the heap.
 * returns 0 on success, nonzero if fails
 *
 * note: undefined behavior of node is not in the heap
 */
int heap_delete(heap_t *h, heap_node * node);



#define LEFT_CHILD 0x1

// pointers must be aligned by 2 bytes for this to work (need 1 bit in rsib)
#define ALIGNMENT_REQUIREMENT 2


#define IS_LEFT_CHILD(heap_node) \
    ((((ptr_t) (heap_node)->rsib) & LEFT_CHILD) != 0)

#define IS_RIGHT_CHILD(heap_node) (!IS_LEFT_CHILD(heap_node))


#define SET_LEFT_CHILD(node) \
    ((node)->rsib = (heap_node *) (((ptr_t) (node)->rsib) | LEFT_CHILD))

#define SET_RIGHT_CHILD(node) \
    ((node)->rsib = (heap_node *) (((ptr_t) (node)->rsib) & ~LEFT_CHILD))


// this node has a parent (i.e. is not the root) if it has a right sibling.
// Note that the only child bits will not be set for the root, meaning rsib
// having value (heap_node*) 0x1 or 0x2 is not possible
#define HAS_PARENT(heap_node) \
    (((heap_node)->rsib) != NULL)

#define RSIB(node) \
    ((heap_node*) (((ptr_t) (node)->rsib) & ~LEFT_CHILD))


#define IS_ALIGNED(ptr) \
    ((((ptr_t) (ptr)) & 0x7) == 0)


// returns the right child of node if is has one, NULL otherwise
static heap_node * _right_child(heap_node * node) {
    heap_node * child = node->lchild;
    if (child != node) {
        heap_node * rsib = RSIB(child);
        if (rsib != node) {
            return rsib;
        }
        else if (IS_RIGHT_CHILD(child)) {
            return child;
        }
    }
    return NULL;
}


static void _make_root(heap_t * h, heap_node * root) {
    h->root = root;
    root->rsib = (heap_node *) (((ptr_t) h) | LEFT_CHILD);
}



int heap_init(heap_t *h) {
    // root is an alias for lchild, so when empty, we set it to itself
    h->root = (heap_node *) h;
    return 0;
}


void heap_destroy(heap_t *h) {
}




/*
 * add rchild as a right child of node, assuming node does not currently have
 * a right child
 */
static void __add_right_child(heap_node * node, heap_node * rchild) {
    heap_node * lchild = node->lchild;

    rchild->rsib = node;
    if (lchild == node) {
        // node has no children
        node->lchild = rchild;
    }
    else {
        // node has left child lchild and no right child
        HEAP_ASSERT(IS_LEFT_CHILD(lchild));
        HEAP_ASSERT(RSIB(lchild) == node);

        lchild->rsib = (heap_node *) (((ptr_t) rchild) | LEFT_CHILD);
    }
}


/*
 * add new_child to the chain of right-child nodes starting from the left
 * (any only) child of node
 *
 * i.e.
 *
 *                    5           8                    5
 *                  /           /                   /
 *  _add_child(   10      ,   12     ) =          8
 *               /  \        /  \              /     \
 *              20   15     14  16           12      10
 *                                          /  \    /  \
 *                                         14  16  20  15
 */
static void _add_child(heap_node * node, heap_node * new_child) {
    heap_node * lchild = node->lchild;

    if (lchild == node) {
        // if the node has no children, just add new_child
        node->lchild = new_child;

        // new_child is 
        new_child->rsib = (heap_node *) (((ptr_t) node) | LEFT_CHILD);
    }
    else {
        // node can only have a left child, no right child
        HEAP_ASSERT(IS_LEFT_CHILD(lchild));
        HEAP_ASSERT(RSIB(lchild) == node);

        __add_right_child(new_child, lchild);

        // now just add new_child as a left child of node
        node->lchild = new_child;
        new_child->rsib = (heap_node *) (((ptr_t) node) | LEFT_CHILD);
    }
}




/*
 * links the two subtrees rooted at n1 and n2 by making the node with smaller
 * key the new root, and making the other tree the leftmost child of the new
 * root. Returns the root of the resultant tree
 */
static heap_node * _link(heap_node * n1, heap_node * n2) {

    HEAP_ASSERT(n1 != NULL);
    HEAP_ASSERT(n2 != NULL);

    if (n2->key < n1->key) {
        heap_node * tmp = n1;
        n1 = n2;
        n2 = tmp;
    }

    // n1 is tree with smaller root

    _add_child(n1, n2);

    // return new tree
    return n1;
}



/*
 * removes the right child from the tree rooted at node and returns that
 * right child (i.e. partially removes the tree rootet at node from the
 * right-child chain of trees it's in)
 */
static heap_node * __unlink_from_rchild(heap_node * node) {
    heap_node * lchild = node->lchild;

    if (lchild != node) {
        // this node has at least one child

        // l_rsib is rsib of lchild
        heap_node * l_rsib = RSIB(lchild);

        if (IS_RIGHT_CHILD(lchild)) {
            // lchild is right child of node

            // if lchild is the right child, it must be the only child
            HEAP_ASSERT(l_rsib == node);

            // node has no left children, so it's an empty subtree
            node->lchild = node;
            return lchild;
        }
        else if (l_rsib != node) {
            // l_rsib is the right child
            HEAP_ASSERT(IS_RIGHT_CHILD(l_rsib));

            // need to unlink lchild from l_rsib (lchild now an only child, so
            // need to set LEFT_CHILD bit)
            lchild->rsib = (heap_node *) (((ptr_t) node) | LEFT_CHILD);
            return l_rsib;
        }
    }

    // otherwise, this node does not have a right child and no work needs to
    // be done
    return NULL;
}


/*
 * removes node from the heap
 */
static void _unlink(heap_node * node) {

    // even the root has a parent (the base of the heap)
    HEAP_ASSERT(HAS_PARENT(node));

    // cut the edge connecting node to its parent and link that subtree with
    // the rest of the heap

    int node_is_left_child = IS_LEFT_CHILD(node);
    heap_node * rsib = RSIB(node);

    // regardless of where node is in the heap, we have to unlink it from
    // its right child chain
    heap_node * rchild = __unlink_from_rchild(node);

    if (rsib->lchild == node) {
        // node is an only child, so rsib is actually parent
        heap_node * parent = rsib;

        if (rchild != NULL) {
            // if there was a right child, then it replaces node
            parent->lchild = rchild;

            // pass on the LEFT_CHILD bit of node
            rchild->rsib =
                (heap_node *) (((ptr_t) parent) | node_is_left_child);
        }
        else {
            // otherwise, if this node has no right child, then we need
            // to set lchild of parent to self (meaning it has no
            // children)
            parent->lchild = parent;
        }
    }
    else {
        // node has a sibling, so action depends on whether it was a left or
        // right child

        if (node_is_left_child) {

            HEAP_ASSERT(IS_RIGHT_CHILD(rsib));

            // we know LEFT_CHILD bit of rsib is not set, so we can just
            // access rsib directly without masking
            heap_node * parent = rsib->rsib;

            // now replace node with rchild
            if (rchild != NULL) {
                parent->lchild = rchild;
                rchild->rsib = (heap_node *) (((ptr_t) rsib) | LEFT_CHILD);
            }
            else {
                // if there was no replacement for node, then rsib is now
                // the only child
                parent->lchild = rsib;
            }
        }
        else /* IS_RIGHT_CHILD(node) */ {
            // if node is a right child, then rsib is actually the parent
            heap_node * parent = rsib;
            heap_node * lsib = parent->lchild;

            HEAP_ASSERT(IS_LEFT_CHILD(lsib));

            // now replace node with rchild
            if (rchild != NULL) {
                lsib->rsib = (heap_node *) (((ptr_t) rchild) | LEFT_CHILD);
                rchild->rsib = parent;
            }
            else {
                // if there was no replacement for node, then lsib is now
                // the only child
                lsib->rsib = (heap_node *) (((ptr_t) parent) | LEFT_CHILD);
            }
        }
    }
}



/*
 * returns a pointer to the heap node at the top of the heap (having minimum
 * key value of all heap nodes)
 */
heap_node * heap_find_min(heap_t *h) {
    if (h->root == (heap_node *) h) {
        return NULL;
    }
    return h->root;
}





/*
 * deletes node from its subtree, returning a new subtree without node
 *
 * First, we remove the root, then for each of the remaining k subtrees which
 * were children of node, we link them together in pairs, i.e. subtree 1 links
 * with 2, 3 with 4, and so on. If k is odd, then the last tree remains
 * unlinked. Then, starting from the next to last pair and working to the
 * front, we combine that pair into the last subtree. After this process is
 * over, the last subtree is the heap without node
 */
static heap_node * _delete_root(heap_node * node) {
    heap_node * prev_subtree = NULL;
    heap_node * next_subtree;

    heap_node * child = node->lchild;
    if (child == node) {
        // node does not have any children
        return NULL;
    }

    // to track the list of combined subtree pairs, we link them in a list
    // using the rsib field of each, as this field will be unused
    do {
        heap_node * child2 = _right_child(child);
        if (child2 != NULL) {
            next_subtree = _right_child(child2);

            // need to remove both child and child2 from list
            __unlink_from_rchild(child);
            __unlink_from_rchild(child2);

            // combine child and child2 into one subtree
            child = _link(child, child2);
        }
        else {
            next_subtree = NULL;
        }

        // append the combined tree to the list of combined trees
        child->rsib = prev_subtree;
        prev_subtree = child;

        child = next_subtree;
    } while (child != NULL);

    // now go backwards and combine all subtrees produced by the above loop
    // into one large tree

    // the head pointer to list of subtrees is prev_subtree
    child = prev_subtree->rsib;
    while (child != NULL) {
        HEAP_ASSERT((((ptr_t) child) & 0x1) == 0);
        next_subtree = child->rsib;
        prev_subtree = _link(prev_subtree, child);
        child = next_subtree;
    }

    return prev_subtree;
}


/*
 * deletes the min heap node from the heap
 */
void heap_delete_min(heap_t *h) {
    heap_node * root = h->root;
    root = _delete_root(root);

    if (root != NULL) {
        _make_root(h, root);
    }
    else {
        h->root = (heap_node *) h;
    }
}

/*
 * combines the effect of find_min and delete_min, returning the deleted min
 */
heap_node * heap_extract_min(heap_t *h) {
    heap_node * min = heap_find_min(h);
    if (min != NULL) {
        heap_delete_min(h);
    }
    return min;
}




/*
 * inserts a node into the tree. The node must already be initialized, i.e. its
 * key must be set
 *
 * returns 0 on success, nonzero if fails
 */
int heap_insert(heap_t *h, heap_node * node) {

    HEAP_ASSERT(IS_ALIGNED(node));
    // make node a one-node tree, then link it with the current root
    node->lchild = node;

    heap_node * root = h->root;

    if (root != (heap_node *) h) {
        // pass node as second argument, as it is less likely for node < root
        root = _link(root, node);

        _make_root(h, root);
    }
    else {
        // if the heap is empty, we want to set the node we are inserting as
        // the left child of the heap struct (which acts like a node that isn't
        // actually part of the heap)
        _add_child((heap_node *) h, node);
    }

    return 0;
}


/*
 * joins heaps h1 and h2 into one heap, which is stored in the h1 struct. Both
 * heaps h1 and h2 are corrupted by this operation, i.e. they would need to be
 * cloned if you wanted to access them after doing this operation
 */
int heap_meld(heap_t *h1, heap_t *h2) {
    heap_node * r1 = heap_find_min(h1);
    heap_node * r2 = heap_find_min(h2);

    if (r2 == NULL) {
        // do nothing if h2 is empty
    }
    else if (r1 == NULL) {
        _make_root(h1, r1);
    }
    else {
        r1 = _link(r1, r2);
        _make_root(h1, r1);
    }

    return 0;
}



/*
 * decreases key value of node to the new key value
 */
int heap_decrease_key(heap_t *h, heap_node * node, heap_key_t new_key) {

    heap_key_t old_key = node->key;

    // this must strictly decrease the key
    HEAP_ASSERT(old_key > new_key);

    node->key = new_key;

    // remove the subtree rooted at node from the tree, then link it with the
    // root of the tree
    _unlink(node);

    heap_node * root = h->root;
    if (root != (heap_node *) h) {
        root = _link(root, node);

        _make_root(h, root);
    }
    else {
        // if the heap is now empty, then node was the root
        HEAP_ASSERT(IS_LEFT_CHILD(node));
        h->root = node;
    }

    return 0;
}


/*
 * increases key value of node to the new key value
 */
int heap_increase_key(heap_t *h, heap_node * node, heap_key_t new_key) {

    heap_key_t old_key = node->key;

    // this muse strictly increase the key
    HEAP_ASSERT(old_key < new_key);

    node->key = new_key;

    int res = heap_delete(h, node);
    if (res == 0) {
        res = heap_insert(h, node);
    }
    return res;
}



/*
 * deletes a node from the heap, assuming the node is actually in the heap.
 * returns 0 on success, nonzero if fails
 *
 * note: undefined behavior of node is not in the heap
 */
int heap_delete(heap_t *h, heap_node * node) {
    heap_node * root = h->root;

    if (root == node) {
        // if heap_delete is called on the root, then just perform delete min
        // on the whole heap
        heap_delete_min(h);
    }
    else {
        // otherwise, we have to unlink node from the heap, call delete root
        // on the subtree, then recombine the subtree with the remainder of the
        // heap
        _unlink(node);
        heap_node * new_subtree = _delete_root(node);

        if (new_subtree != NULL) {
            // if node had any children, then new subtree will be nonnull, and
            // we have to reinsert them into the heap
            root = _link(root, new_subtree);
        }

        _make_root(h, root);
    }

    return 0;
}





typedef struct edge {
    uint32_t w;
    uint32_t to;
    uint32_t cost;
    uint32_t edge_idx;
} edge_t;

typedef struct val_edge {
    heap_node node;
    uint32_t w;
    // number of children this edge lies on the path to from the root
    uint32_t n_children;
    // either 1 or 2
    uint32_t cost;
} val_edge_t;


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
    if (l->ptr) {
        free(l->ptr);
    }
}


typedef struct int_dylist {
    uint32_t len;
    uint32_t cap;
    uint64_t * ptr;
} int_dylist_t;

static void int_dylist_push(int_dylist_t * l, uint64_t val) {
    uint32_t cap = l->cap;
    if (cap == l->len) {
        cap = (cap << 1) | !cap;
        l->cap = cap;
        uint64_t * new_ptr = (uint64_t *) realloc(l->ptr, cap * sizeof(edge_t));
        l->ptr = new_ptr;
    }
    l->ptr[l->len] = val;
    l->len++;
}

static void int_dylist_delete(int_dylist_t * l) {
    if (l->ptr) {
        free(l->ptr);
    }
}


typedef struct node {
    dylist_t edges;

    // exactly one of these edges is to the parent
    uint32_t par_edge_idx;
} node_t;


static void add_edge(node_t * nodes, uint32_t v, uint32_t u, uint32_t w,
        uint32_t edge_idx, uint32_t cost) {
    edge_t ev = {
        .w = w,
        .to = u,
        .edge_idx = edge_idx,
        .cost = cost
    };
    edge_t eu = {
        .w = w,
        .to = v,
        .edge_idx = edge_idx,
        .cost = cost
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

        edges[e->edge_idx].w = e->w;
        edges[e->edge_idx].n_children = num_paths;
        edges[e->edge_idx].cost = e->cost;

        num_paths_thru += num_paths;
    }

    // return num_paths_thru if this isn't a leaf, otherwise 1
    return num_paths_thru | !num_paths_thru;
}


int main(int argc, char * argv[]) {

    int test_cases;
    int n;
    int64_t S;

    assert(fscanf(stdin, "%d", &test_cases) == 1);

    for (int t = 0; t < test_cases; t++) {
        assert(fscanf(stdin, "%d %lld", &n, &S) == 2);

        node_t * nodes = (node_t *) calloc(n, sizeof(node_t));
        val_edge_t * edges = (val_edge_t *) malloc((n - 1) * sizeof(val_edge_t));

        for (int e = 0; e < n - 1; e++) {
            int v, u;
            uint32_t w, c;
            assert(fscanf(stdin, "%d %d %u %u", &v, &u, &w, &c) == 4);

            add_edge(nodes, v - 1, u - 1, w, e, c);
        }

        // set parent value to n, since no node has index n (node 0 has no parent)
        resolve_edges(edges, nodes, 0, n);

        for (int i = 0; i < n; i++) {
            dylist_delete(&nodes[i].edges);
        }
        free(nodes);

        uint64_t tot_w = 0;
        uint32_t n_actions = 0;
        heap_t h1;
        heap_t h2;
        heap_init(&h1);
        heap_init(&h2);


        for (int i = 0; i < n - 1; i++) {
            //printf("e %d, w=%d, nc=%d\n", i, edges[i].w, edges[i].n_children);
            tot_w += ((uint64_t) edges[i].w) * ((uint64_t) edges[i].n_children);
            // max-heap of difference that moves on edges would make
            edges[i].node.key = -(((heap_key_t) ((edges[i].w + 1) / 2)) * edges[i].n_children);

            if (edges[i].cost == 1) {
                heap_insert(&h1, &edges[i].node);
            }
            else {
                heap_insert(&h2, &edges[i].node);
            }
        }

        //printf("tot: %llu\n", tot_w);

        int_dylist_t l1, l2;
        __builtin_memset(&l1, 0, sizeof(int_dylist_t));
        __builtin_memset(&l2, 0, sizeof(int_dylist_t));

        uint64_t tot_w1 = tot_w;

        while (tot_w1 > S && heap_find_min(&h1) != NULL) {
            val_edge_t * e = (val_edge_t *) heap_extract_min(&h1);

            heap_key_t dif = -e->node.key;

            int_dylist_push(&l1, dif);

            uint64_t w = e->w;
            const uint64_t n_c = e->n_children;

            tot_w1 -= dif;

            w >>= 1;
            dif = ((heap_key_t) (((w + 1) / 2) * n_c));
            n_actions++;

            e->w = w;
            e->n_children = n_c;
            e->node.key = -dif;
            if (dif > 0) {
                heap_insert(&h1, &e->node);
            }
        }

        uint64_t tot_w2 = tot_w;

        while (tot_w2 > S && heap_find_min(&h2) != NULL) {
            val_edge_t * e = (val_edge_t *) heap_extract_min(&h2);

            heap_key_t dif = -e->node.key;

            int_dylist_push(&l2, dif);

            uint64_t w = e->w;
            const uint64_t n_c = e->n_children;

            tot_w2 -= dif;

            w >>= 1;
            dif = ((heap_key_t) (((w + 1) / 2) * n_c));
            n_actions++;

            e->w = w;
            e->n_children = n_c;
            e->node.key = -dif;
            if (dif > 0) {
                heap_insert(&h2, &e->node);
            }
        }

        heap_destroy(&h2);
        heap_destroy(&h1);

        free(edges);

        /*
        printf("l1: [");
        for (int i = 0; i < l1.len; i++) {
            printf("%lld", l1.ptr[i]);
            if (i != l1.len -  1) {
                printf(", ");
            }
        }
        printf("]\n");

        printf("l2: [");
        for (int i = 0; i < l2.len; i++) {
            printf("%lld", l2.ptr[i]);
            if (i != l2.len -  1) {
                printf(", ");
            }
        }
        printf("]\n");*/

        int64_t nec_dif = tot_w - S;

        int64_t tots = (int64_t) (tot_w - tot_w2);
        int64_t min_cost = l1.len + 2 * l2.len;

        uint32_t i1 = 0, i2 = l2.len;
        while (i1 < l1.len && tots < nec_dif) {
            tots += l1.ptr[i1];
            i1++;
        }
        while (1) {
            while (i2 > 0 && (tots - ((int64_t) l2.ptr[i2 - 1])) >= nec_dif) {
                i2--;
                tots -= l2.ptr[i2];
            }
            //printf("(%d, %d) => %d (tot=%lld)\n", i1, i2, i1 + 2 * i2, tots);
            if (tots >= nec_dif) {
                min_cost = min(i1 + 2 * i2, min_cost);
            }
            if (i1 == l1.len) {
                break;
            }
            tots += l1.ptr[i1];
            i1++;
        }

        int_dylist_delete(&l2);
        int_dylist_delete(&l1);

        printf("%lld\n", min_cost);
    }

    return 0;
}

