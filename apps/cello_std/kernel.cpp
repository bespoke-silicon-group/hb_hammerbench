#include <cello/cello.hpp>
#include <vector>
#include <unordered_map>
#include <map>

int cello_main(int argc, char *argv[])
{
    std::vector<bool, cello::allocator<bool>> data2;
    for (int i = 0; i < 100; i++) {
        data2.push_back(i % 2);
    }
    for (auto i : data2) {
        bsg_print_int(1000+i);
    }
    //std::map<int, int, std::less<int>, cello::allocator<std::pair<const int, int>>> data;
    std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, cello::allocator<std::pair<const int, int>>> data;
    data[1] = 2;
    data[2] = 3;
    //bsg_printf("Returning from cello_main\n");
    for (auto i : data) {
        bsg_print_int(2000+i.first);
        bsg_print_int(3000+i.second);
    }
    bsg_printf("sizeof(data) = %d\n", sizeof(data));
    return 0;
}
