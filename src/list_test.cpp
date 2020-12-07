#include <iostream>
#include "../include/orange_list.h"

using namespace std;
using namespace orange_stl;

int main()
{
    int a[] = { 1,2,3,4,5 };
    orange_stl::list<int> l1;
    orange_stl::list<int> l2(5);
    orange_stl::list<int> l3(5, 1);
    orange_stl::list<int> l4(a, a + 5);
    orange_stl::list<int> l5(l2);
    orange_stl::list<int> l6(std::move(l2));
    orange_stl::list<int> l7{ 1,2,3,4,5,6,7,8,9 };
    orange_stl::list<int> l8;
    l8 = l3;
    orange_stl::list<int> l9;
    l9 = std::move(l3);
    orange_stl::list<int> l10;
    l10 = { 1, 2, 2, 3, 5, 6, 7, 8, 9 };

    return 0;
}
