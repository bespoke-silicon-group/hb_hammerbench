#ifndef UTIL_LIST_HPP
#define UTIL_LIST_HPP
#include <util/class_field.hpp>
#include <util/container_of.hpp>
namespace util
{

class list_item {
public:
    typedef list_item* item_ptr;

    list_item()
        : next_(this)
        , prev_(this) {
    }

    void insert_in_front_of(item_ptr before_this) {
        this->next() = before_this;
        this->prev() = before_this->prev();
        this->next()->prev() = this;
        this->prev()->next() = this;
    }

    void insert_in_back_of(item_ptr after_this) {
        this->prev() = after_this;
        this->next() = after_this->next();
        this->next()->prev() = this;
        this->prev()->next() = this;
    }

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

    FIELD(list_item, head);
};

}
#endif
