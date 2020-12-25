#ifndef __ORANGE_STL_HEAP_ALGO_H__
#define __ORANGE_STL_HEAP_ALGO_H__

// 这个头文件包含 heap 的四个算法 : push_heap, pop_heap, sort_heap, make_heap

#include "orange_iterator.h"

namespace orange_stl
{

/*****************************************************************************************/
// push_heap
// 该函数接受两个迭代器，表示一个 heap 容器的首尾，并且新元素已经插入到底部容器的最尾端，调整 heap
/*****************************************************************************************/
template <class RandomIter, class Distance, class T>
void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value)
{
    auto parent = (holeIndex - 1) / 2;
    while (holeIndex > topIndex && *(first + parent) < value)
    {
        // 使用 operator<，所以 heap 为 max-heap
        *(first + holeIndex) = *(first + parent);
        holeIndex = parent;
        parent = (holeIndex - 1) / 2;
    }
    *(first + holeIndex) = value;
}

template <class RandomIter, class Distance>
void push_heap_d(RandomIter first, RandomIter last, Distance*)
{
    orange_stl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1));
}

template <class RandomIter>
void push_heap(RandomIter first, RandomIter last)
{ 
    // 新元素应该已置于底部容器的最尾端
    orange_stl::push_heap_d(first, last, distance_type(first));
}

// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class Distance, class T, class Compared>
void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value, Compared comp)
{
    auto parent = (holeIndex - 1) / 2;
    while (holeIndex > topIndex && comp(*(first + parent), value))
    {
        *(first + holeIndex) = *(first + parent);
        holeIndex = parent;
        parent = (holeIndex - 1) / 2;
    }
    *(first + holeIndex) = value;
}

template <class RandomIter, class Compared, class Distance>
void push_heap_d(RandomIter first, RandomIter last, Distance*, Compared comp)
{
  orange_stl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1), comp);
}

template <class RandomIter, class Compared>
void push_heap(RandomIter first, RandomIter last, Compared comp)
{
    orange_stl::push_heap_d(first, last, distance_type(first), comp);
}

/*****************************************************************************************/
// pop_heap
// 该函数接受两个迭代器，表示 heap 容器的首尾，将 heap 的根节点取出放到容器尾部，调整 heap
/*****************************************************************************************/

/* 
    stl源码中的实现是先将first的值取出并置于末元素的位置上，
    而末元素的值保存在__value中，然后再调用__adjust_heap，在
    __adjust_heap中，对__topIndex以下的元素进行调整到以__topIndex
    为起点的位置上，最后会得到本身是叶子节点的洞结点，并且洞结点前面
    的结点是满足max heap条件的，故可对[__topIndex , __holeIndex]
    执行__push_heap操作，插入的值为__value，故最终__value会存入正确的位置上。 */
template <class RandomIter, class T, class Distance>
void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value)
{
    // 先进行下溯(percolate down)过程
    auto topIndex = holeIndex;
    auto rchild = 2 * holeIndex + 2;
    while (rchild < len)
    {
        if (*(first + rchild) < *(first + rchild - 1))
          --rchild;
        *(first + holeIndex) = *(first + rchild);
        holeIndex = rchild;
        rchild = 2 * (rchild + 1);
    }
    if (rchild == len)
    {  
        // 如果没有右子节点
        *(first + holeIndex) = *(first + (rchild - 1));
        holeIndex = rchild - 1;
    }
    // 再执行一次上溯(percolate up)过程
    orange_stl::push_heap_aux(first, holeIndex, topIndex, value);
}

template <class RandomIter, class T, class Distance>
void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance*)
{
    // 先将首值调至尾节点，然后调整[first, last - 1)使之重新成为一个 max-heap
    *result = *first;
    orange_stl::adjust_heap(first, static_cast<Distance>(0), last - first, value);
}

template <class RandomIter>
void pop_heap(RandomIter first, RandomIter last)
{
    orange_stl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first));
}

// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class T, class Distance, class Compared>
void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value, Compared comp)
{
    // 先进行下溯(percolate down)过程
    auto topIndex = holeIndex;
    auto rchild = 2 * holeIndex + 2;
    while (rchild < len)
    {
        if (comp(*(first + rchild), *(first + rchild - 1)))  --rchild;
        *(first + holeIndex) = *(first + rchild);
        holeIndex = rchild;
        rchild = 2 * (rchild + 1);
    }
    if (rchild == len)
    {
        *(first + holeIndex) = *(first + (rchild - 1));
        holeIndex = rchild - 1;
    }
    // 再执行一次上溯(percolate up)过程
    orange_stl::push_heap_aux(first, holeIndex, topIndex, value, comp);
}

template <class RandomIter, class T, class Distance, class Compared>
void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance*, Compared comp)
{
    *result = *first;  // 先将尾指设置成首值，即尾指为欲求结果
    orange_stl::adjust_heap(first, static_cast<Distance>(0), last - first, value, comp);
}

template <class RandomIter, class Compared>
void pop_heap(RandomIter first, RandomIter last, Compared comp)
{
  orange_stl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first), comp);
}

/*****************************************************************************************/
// sort_heap
// 该函数接受两个迭代器，表示 heap 容器的首尾，不断执行 pop_heap 操作，直到首尾最多相差1
/*****************************************************************************************/
template <class RandomIter>
void sort_heap(RandomIter first, RandomIter last)
{
    // 每执行一次 pop_heap，最大的元素都被放到尾部，直到容器最多只有一个元素，完成排序
    while (last - first > 1)
    {
        orange_stl::pop_heap(first, last--);
    }
}

// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class Compared>
void sort_heap(RandomIter first, RandomIter last, Compared comp)
{
    while (last - first > 1)
    {
        orange_stl::pop_heap(first, last--, comp);
    }
}

/*****************************************************************************************/
// make_heap
// 该函数接受两个迭代器，表示 heap 容器的首尾，把容器内的数据变为一个 heap
/*****************************************************************************************/
template <class RandomIter, class Distance>
void make_heap_aux(RandomIter first, RandomIter last, Distance*)
{
    if (last - first < 2)
      return;
    auto len = last - first;
    auto holeIndex = (len - 2) / 2;
    while (true)
    {
        // 重排以 holeIndex 为首的子树
        orange_stl::adjust_heap(first, holeIndex, len, *(first + holeIndex));
        if (holeIndex == 0)
          return;
        holeIndex--;
    }
}

template <class RandomIter>
void make_heap(RandomIter first, RandomIter last)
{
    orange_stl::make_heap_aux(first, last, distance_type(first));;
}

// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class Distance, class Compared>
void make_heap_aux(RandomIter first, RandomIter last, Distance*, Compared comp)
{
    if (last - first < 2)
        return;
    auto len = last - first;
    auto holeIndex = (len - 2) / 2;
    while (true)
    {
        // 重排以 holeIndex 为首的子树
        orange_stl::adjust_heap(first, holeIndex, len, *(first + holeIndex), comp);
        if (holeIndex == 0)
          return;
        holeIndex--;
    }
}

template <class RandomIter, class Compared>
void make_heap(RandomIter first, RandomIter last, Compared comp)
{
    orange_stl::make_heap_aux(first, last, distance_type(first), comp);
}

} // namespace orange_stl
#endif // !__ORANGE_STL_HEAP_ALGO_H__

