#include <util/list.hpp>
#include <util/test_eq.hpp>

#define SIZE unsigned, "%u"
#define BOOL int, "%d"
#define PTR void*, "%p"

void empty_list()
{
    util::list l;
    TEST_EQ(BOOL, l.empty(), true);
    return;
}

void push_front()
{
    util::list l;
    util::list_item i;
    l.push_front(&i);
    TEST_EQ(PTR, l.fast_front(), &i);
    TEST_EQ(PTR, l.front(), &i);
    TEST_EQ(PTR, l.fast_back(), &i);
    TEST_EQ(PTR, l.back(), &i);

    util::list_item j;
    l.push_front(&j);
    TEST_EQ(PTR, l.fast_front(), &j);
    TEST_EQ(PTR, l.front(), &j);
    TEST_EQ(PTR, l.fast_back(), &i);
    TEST_EQ(PTR, l.back(), &i);
    return;
}

void push_back()
{
    util::list l;
    util::list_item i;
    l.push_back(&i);
    TEST_EQ(PTR, l.fast_front(), &i);
    TEST_EQ(PTR, l.front(), &i);
    TEST_EQ(PTR, l.fast_back(), &i);
    TEST_EQ(PTR, l.back(), &i);

    util::list_item j;
    l.push_back(&j);
    TEST_EQ(PTR, l.fast_front(), &i);
    TEST_EQ(PTR, l.front(), &i);
    TEST_EQ(PTR, l.fast_back(), &j);
    TEST_EQ(PTR, l.back(), &j);
    return;    
}

void pop_front()
{
    util::list l;
    util::list_item i;
    l.push_front(&i);
    l.pop_front();
    TEST_EQ(BOOL, l.empty(), true);
    TEST_EQ(PTR, l.front(), nullptr);
    util::list_item j;
    l.push_front(&i);
    l.push_front(&j);
    l.pop_front();
    TEST_EQ(PTR, l.front(), &i);
    TEST_EQ(PTR, l.back(), &i);
    l.pop_front();
    TEST_EQ(BOOL, l.empty(), true);
    TEST_EQ(PTR, l.front(), nullptr);
    TEST_EQ(PTR, i.next(), &i);
    TEST_EQ(PTR, i.prev(), &i);
    TEST_EQ(PTR, j.next(), &j);
    TEST_EQ(PTR, j.prev(), &j);
    return;
}

void pop_back()
{
    util::list l;
    util::list_item i;
    l.push_back(&i);
    l.pop_back();
    TEST_EQ(BOOL, l.empty(), true);
    TEST_EQ(PTR, l.back(), nullptr);
    util::list_item j;
    l.push_back(&i);
    l.push_back(&j);
    l.pop_back();
    TEST_EQ(PTR, l.front(), &i);
    TEST_EQ(PTR, l.back(), &i);
    l.pop_back();
    TEST_EQ(BOOL, l.empty(), true);
    TEST_EQ(PTR, l.back(), nullptr);
    TEST_EQ(PTR, i.next(), &i);
    TEST_EQ(PTR, i.prev(), &i);
    TEST_EQ(PTR, j.next(), &j);
    TEST_EQ(PTR, j.prev(), &j);    
    return;
}

extern "C" int kernel()
{
    empty_list();
    push_front();
    push_back();
    pop_front();
    pop_back();
    return 0;
}
