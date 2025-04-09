 #include <cello/cello.hpp>
#include <vector>

template <typename index_type>
inline index_type floor_log2(index_type x)
{
    index_type i = -1;
    index_type j = i+1;
    while  ((1 << j) <= x) {
        i = j;
        j = j+1;
    }
    return i;
}

template <typename index_type>
inline index_type ceil_log2(index_type x)
{
    index_type j = 0;
    while (x > (1<<j)) {
        j = j+1;
    }
    return j;
}

/*
 * A node is either red or black
 *
 * The root and leaf (nil)  nodes are black
 *
 * If a node is red, then its children are black
 *
 * All paths from a node to its nil descendents
 * contain the same number of black nodes.
 */
template <typename Key, typename Value>
class rb_node {
public:
    static constexpr Key RED = 0, BLACK = 1;
    Key     color: 1;    
    Key     key  :31;
    Value   val;    
    rb_node *p = nullptr;
    rb_node *l = nullptr;
    rb_node *r = nullptr;
    /**
     * @brief allocates a new task
     */
    void *operator new(size_t size) {
        return cello::allocate(size);
    }

    /**
     * @brief deallocates a task
     */
    void operator delete(void *p) {
        cello::deallocate(p, sizeof(rb_node));
    }

    /**
     * static assertions
     */
    void assertions() const {
        static_assert(std::is_integral<Key>::value && "Key must be an integer type");
        static_assert(sizeof(Key) == 4 && "Key must be word size of the machine");
    }

    /**
     * @brief return true if n is nill
     */
    static bool is_nill(rb_node *n) { return n == nullptr; }

    /**
     * @brief return true if n is black
     */
    static bool is_black(rb_node *n) {
        if (is_nill(n))
            return true;
        return n->color;
    }

    /**
     * @brief return true is is red
     */
    static bool is_red(rb_node *n) {
        return !is_black(n);
    }
};

template <typename Key, typename Value>
class rb_iterator;

template<typename Key, typename Value>
class rb_tree {
public:
    using node = rb_node<Key, Value>;
    using iterator = rb_iterator<Key, Value>;
    static constexpr Key BLACK = node::BLACK, RED = node::RED;

    /**
     * @brief return true if n is nill
     */
    static bool is_nill(node *n) { return node::is_nill(n); }

    /**
     * @brief return true if n is black
     */
    static bool is_black(node *n) { return node::is_black(n); }

    /**
     * @brief return true is is red
     */
    static bool is_red(node *n) { return node::is_red(n); }
    
    /**
     * @brief Check that  the root and leaf (nil)  nodes are black
     */
    void check_root_and_leafs() {
        if (!is_black(root)) {
            bsg_printf("Root @ {key = %d} is not black\n", root->key);
        }
        // leafs are black by definition
    }

    /**
     * @brief Check that red nodes have black children
     */
    void check_red_nodes_have_black_children() {
        for (iterator it(this); it; it.next()) {
            if (is_red(&*it)) {
                bool l_is_black = is_black(it->l);
                bool r_is_black = is_black(it->r);
                bool bad = !(l_is_black && r_is_black);
                if (bad) {
                    bsg_printf("Red node @ {key = %d} has a red child: %s\n",
                               it->key,
                               l_is_black
                               ? (r_is_black ? "l+r" : "l")
                               : "r");
                }
            }
        }
    }

    /**
     * @brief Check that constraints of an rb tree hold
     */
    void check() {
        check_root_and_leafs();
        check_red_nodes_have_black_children();        
    }

    /**
     * insert a key value pair
     */
    void insert(const Key &key, const Value &val) {
        node *z = new node;
        z->key = key;
        z->val = val;
        z->color = RED;        
        insert_internal(z);
        fixup_after_insertion(z);
    }

    /**
     * insert a key value pair
     */
    void insert_internal(node *z) {
        node **np = &root;
        node  *p = nullptr;
        while (!is_nill(*np)) {
            p = *np;
            np = (z->key < p->key) ? &p->l : &p->r;
        }
        z->p = p;
        *np = z;
    }

    /**
     * left rotation
     */
    void left_rotate(node *z) {
        // set y
        node *zp = z->p;
        node *y = z->r;
        node *zpl = zp->l;
        node *zpr = zp->r;
        // turn y's left subtree into z's right subtree
        z->r = y->l;
        if (!is_nill(z->r)) {
            z->r->p = z;
        }
        // link z's parent to y
        y->p = zp;
        if (!is_nill(z)) {
            if (zpl == z) {
                zp->l = y;
            } else {
                zp->r = y;
            }
        } else {
            root = y;
        }
        // put z on y's left
        y->l = z;
        z->p = y;
    }

    /**
     * right rotation
     */
    void right_rotate(node *z) {
        // set x
        node *zp = z->p;
        node *x = z->l;
        node *zpl = zp->l;
        node *zpr = zp->r;
        asm volatile ("" ::: "memory");
        // turn x's right subtree into z's left subtree
        z->l = x->r;
        if (!is_nill(z->l)) {
            z->l->p = z;
        }
        // link z's parent to x
        x->p = zp;
        if (!is_nill(zp)) {
            if (zpl == z) {
                zp->l = x;
            } else {
                zp->r = x;
            }
        } else {
            root = x;
        }
        // put z on x's right
        x->r = z;
        z->p = x;
    }

   /**
    * fixup after an insertion
    */
    void fixup_after_insertion(node *z) {
        node *zp = z->p;
        while (is_red(zp)) {
            // note that zgp is not nill
            // since zp is red and thus is not the root
            node *zpr = zp->r;
            node *zpl = zp->l;
            node *zgp = zp->p;
            asm volatile ("" ::: "memory");            
            node *zgpl = zgp->l;
            node *zgpr = zgp->r;
            asm volatile ("" ::: "memory");
            if (zp == zgpl) {
                // left side of grandparent
                node *u = zgpr; // uncle is right side
                if (is_red(u)) {
                    // recolor parent, grandparent, and uncle
                    // set z to grandparent
                    zp->color = BLACK;
                    zgp->color = RED;
                    u->color = BLACK;
                    z = zgp;
                } else {
                    // elbow
                    if (z == zpr) {
                        z = zp;
                        left_rotate(z);
                        zp = z->p;
                        zgp = zp->p;                        
                    }                   
                    zp->color = BLACK;
                    zgp->color = RED;
                    right_rotate(zgp);
                    
                }
            } else {
                // right side of grandparent
                node *u = zgpl; // uncle is left side
                if (is_red(u)) {
                    // recolor parent, grandparent, and uncle
                    // set z to grandparent
                    zp->color = BLACK;
                    zgp->color = RED;
                    u->color = BLACK;
                    z = zgp;
                } else {
                    // elbow
                    if (z == zpl) {
                        z = zp;
                        right_rotate(z);
                        zp = z->p;
                        zgp = zp->p;
                    }
                    zp->color = BLACK;
                    zgp->color = RED;
                    left_rotate(zgp);
                }
            }
            zp = z->p;
        }
        root->color = BLACK;
    }

    node  *root = nullptr;
    size_t size = 0;    
};


template <typename Key, typename Value>
class rb_iterator {
public:
    using tree = rb_tree<Key, Value>;
    using node = typename tree::node;
    /**
     * @brief default constructor
     */
    rb_iterator(tree *tree) {
        stack.reserve(ceil_log2(tree->size));
        curr = tree->root;
        while (curr && !is_nill(curr->l)) {
            stack.push_back(curr);
            curr = curr->l;
        }
    }
    /**
     * @brief arrow operator
     */
    node *operator->() { return curr; }

    /**
     * @brief const arrow operator
     */
    const node *operator->() const { return curr; }

    /**
     * @brief dereference operator
     */
    node &operator*() {
        return *curr;
    }

    /**
     * @brief const dereference operator
     */
    const node &operator*() const {
        return *curr;
    }

    /**
     * iterate
     */
    void next() {
        if (!is_nill(curr)) {
            if (is_nill(curr->r)) {
                if (!stack.empty()) {
                    curr = stack.back();
                    stack.pop_back();
                } else {
                    curr = nullptr;
                }
            } else {
                curr = curr->r;
                while (curr && !is_nill(curr->l)) {
                    stack.push_back(curr);
                    curr = curr->l;
                }
            }
        }
    }

    /**
     * @brief bool value of the iterator
     */
    operator bool() const { return !is_nill(curr); }

    /**
     * @brief return true if the iterator is good
     */
    bool good() const { return !is_nill(curr); }
    
    /**
     * @brief return true if n is nill
     */
    static bool is_nill(node *n) { return node::is_nill(n); }

    /**
     * @brief return true if n is black
     */
    static bool is_black(node *n) { return node::is_black(n); }

     /**
     * @brief return true is is red
     */
    static bool is_red(node *n) { return node::is_red(n); }

    node *curr = nullptr;
    std::vector<node*, cello::allocator<node*>> stack;
};

int cello_main(int argc, char *argv[])
{
    rb_tree<int,int> tree;
    tree.insert(2, 0);
    tree.check();    
    tree.insert(1, 0);
    tree.check();    
    tree.insert(0, 0);
    tree.check();
    for (rb_iterator<int,int> it(&tree); it; it.next()) {
        bsg_printf("%3d %3d\n", it->key, it->val);
        bsg_fence();        
    }
    return 0;
}
