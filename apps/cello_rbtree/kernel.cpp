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
    /**
     * @brief Check that  the root and leaf (nil)  nodes are black
     */
    void check_root_and_leafs() {
        if (!is_black(root)) {
            bsg_printf("Root @ %p is not black\n", root);
        }
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
        bsg_printf("%s\n", __PRETTY_FUNCTION__);
        bsg_fence();        
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
     * iterate
     */
    void next() {
        bsg_printf("%s\n", __PRETTY_FUNCTION__);
        bsg_fence();
        if (!is_nill(curr)) {
            if (is_nill(curr->r)) {
                curr = stack.back();
                stack.pop_back();
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
    operator bool() const { return is_nill(curr); }

    /**
     * @brief return true if the iterator is good
     */
    bool good() const { return is_nill(curr); }
    
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
    rb_iterator<int,int> it(&tree);
    if (it.good()) {
        bsg_printf("%3d %3d\n"        
#if 0
                   , it.curr->key, it.curr->val
#else
                   , it->key, it->val
#endif
                   );
        bsg_fence();
    }
    return 0;
}
