#ifndef UTIL_LIST_HPP
#define UTIL_LIST_HPP
#include <util/class_field.hpp>
#include <util/container_of.hpp>
#include <cstdint>
namespace util
{

class list_item {
public:
    typedef list_item* item_ptr;

    list_item()
        : next_(this)
        , prev_(this) {
    }

    /**
     * @brief insert this in front of before_this
     */
    void insert_in_front_of(item_ptr before_this) {
        this->next() = before_this;
        this->prev() = before_this->prev();
        this->next()->prev() = this;
        this->prev()->next() = this;
    }

    /**
     * @brief insert this before after_this
     */
    void insert_in_back_of(item_ptr after_this) {
        this->prev() = after_this;
        this->next() = after_this->next();
        this->next()->prev() = this;
        this->prev()->next() = this;
    }

    /**
     * @brief remove from whichever list this is in
     */
    void remove() {
        item_ptr n = this->next();
        item_ptr p = this->prev();
        n->prev() = p;
        p->next() = n;
        this->next() = this;
        this->prev() = this;
    }
    
    FIELD(item_ptr, next);
    FIELD(item_ptr, prev);
};

class list {
public:
    typedef list_item* item_ptr;
    /**
     * @brief true if list is empty
     */
    bool empty() const {
        return head().next() == &head();
    }

    /**
     * @brief true if list is empty - cannot be compiled out
     */
    bool empty_volatile() const {
        item_ptr n;
        asm volatile ("lw %0, %1" : "=r" (n) : "m" (head().next()) : "memory");
        return n == &head();
    }

    /**
     * @brief clear the list
     */
    void clear() {
        head().next() = head().prev() = &head();
    }

    /**
     * @brief push an item to front of the list
     */
    void push_front(item_ptr ip) {
        ip->insert_in_back_of(&head());
    }
    /**
     * @brief push an item to back of the list
     */
    void push_back(item_ptr ip) {
        ip->insert_in_front_of(&head());
    }
    /**
     * return the front of the list, head if empty
     */
    item_ptr fast_front() const {
        return head().next();
    }
    /**
     * return the back of the list, head if empty
     */
    item_ptr fast_back() const {
        return head().prev();
    }    
    /**
     * return the front of the list, null if empty
     */
    item_ptr front() const {
        return empty() ? nullptr : fast_front();
    }    
    /**
     * return the back of the list, null if empty
     */
    item_ptr back() const {
        return empty() ? nullptr : fast_back();
    }
    /**
     * pop from the front of the list, null if empty
     */
    item_ptr pop_front() const {
        if (!empty()) {
            item_ptr f = fast_front();
            f->remove();
            return f;
        }
        return nullptr;
    }
    /**
     * pop from the back of the list, null if empty
     */
    item_ptr pop_back() const  {
        if (!empty()) {
            item_ptr f = fast_back();
            f->remove();
            return f;
        }
        return nullptr;
    }

    /**
     * extend entire list to back
     */
    void extend_back(item_ptr first, item_ptr last) {
        // link first with head.prev()
        first->prev() = this->head().prev();
        this->head().prev()->next() = first;
        // link last with head
        last->next() = &this->head();
        this->head().prev() = last;
    }

    /**
     * extend entire list to front
     */
    void extend_front(item_ptr first, item_ptr last) {
        // link last with head.next()
        last->next() = this->head().next();
        this->head().next()->prev() = last;
        // link first with head
        first->prev() = &this->head();
        this->head().next() = first;
    }
    

    /**
     * extend entire list to back
     * assumes l is non-empty
     * assumes l will not be used after
     */
    void extend_back_fast(list *l) {
        extend_back(l->fast_front(), l->fast_back());
    }

    /**
     * extend entire list to front
     * assumes l is non-empty
     * assumes l will not be used after     
     */    
    void extend_front_fast(list *l) {
        extend_front(l->fast_front(), l->fast_back());
    }

    /**
     * extend entire list to back
     * l is cleared after call
     */
    void extend_back(list *l) {
        if (!l->empty()) {
            extend_back(l->fast_front(), l->fast_back());
            l->clear();
        }
    }

    /**
     * extend entire list to front
     * l is cleared after call
     */
    void extend_front(list *l) {
        if (!l->empty()) {
            extend_front(l->fast_front(), l->fast_back());
            l->clear();
        }
    }

    /**
     * link an array of items together
     * @param first - v vector of items to link
     * @param last - n size of the vector
     * @param stride - distance between items in vector
     */    
    static void linkv_fwd(item_ptr v, size_t n, size_t stride) {
        item_ptr curr = v, next;
        for (size_t i = 0; i < n-1; i++) {
            next = reinterpret_cast<item_ptr>(reinterpret_cast<uintptr_t>(curr) + stride);
            curr->next() = next;
            next->prev() = curr;
            curr = next;
        }
    }

    /**
     * link an array of items together
     * @param first - v vector of items to link
     * @param last - n size of the vector
     * @param stride - distance between items in vector
     */    
    static void linkv_rev(item_ptr v, size_t n, size_t stride) {
        item_ptr curr = v, prev;
        for (size_t i = 0; i < n-1; i++) {
            prev = reinterpret_cast<item_ptr>(reinterpret_cast<uintptr_t>(curr) + stride);
            curr->prev() = prev;
            prev->next() = curr;
            curr = prev;
        }
    }

    FIELD(list_item, head);
};

}
#endif
