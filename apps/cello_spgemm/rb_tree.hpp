#ifndef RB_TREE_HPP
#define RB_TREE_HPP
#include <vector>
#include <cello/cello.hpp>
#include <array>
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

#ifdef RB_DBG
#define rb_dbg(fmt, ...)                        \
    do { bsg_printf(fmt, ##__VA_ARGS__); } while (0)
#else
#define rb_dbg(fmt, ...)
#endif

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
     * @brief find a key
     * @return pointer to node if found, o/w nullptr
     */
    node *find(const Key &key) {
        node *z = root;
        while (!is_nill(z) and (z->key != key)) {
            z = (key < z->key ? z->l : z->r);
        }
        return z;
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
        size++;
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
        if (!is_nill(zp)) {
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
        curr = tree->root;
        while (curr && !is_nill(curr->l)) {
            stack[end++] = curr;
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
                if (end != 0) {
                    curr = stack[(end--)-1];
                } else {
                    curr = nullptr;
                }
            } else {
                curr = curr->r;
                while (curr && !is_nill(curr->l)) {
                    stack[end++] = curr;
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
    std::array<node *, 20> stack;
    size_t end = 0;
};
#endif
