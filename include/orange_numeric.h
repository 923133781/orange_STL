#ifndef __ORANGE_STL_NUMERIC_H__
#define __ORANGE_STL_NUMERIC_H__

#include "orange_iterator.h"

namespace orange_stl
{
// accumulate
// 以初值init对每个元素进行累加
template <class InputIterator, class T>
T accumulate(InputIterator first, InputIterator last, T init)
{
    for(; first!=last; ++first)
    {
        init += *first;
    }
    return init;
}
// 以初值为init对每个元素进行二元操作
template <class InputIter, class T, class BinaryOp>
T accumulate(InputIter first, InputIter last, T init, BinaryOp binary_op)
{
    for(; first!=last; ++first)
    {
        init = binary_op(init, *first);
    }
    return init;
}

/*
adjacent_difference
1. 计算相邻元素的差值，结果保存到result起始的区间上
2. 自定义相邻元素的二元操作
*/
//1
template <class InputIter, class OutputIter>
OutputIter adjacent_difference(InputIter first, InputIter last, OutputIter result)
{
    if(first==last) return result;
    *result=*first;
    auto value=*first;
    while(++first != last)
    {
        auto tmp=*first;
        *++result = tmp-value;
        value=tmp;
    }
    return ++result;
}
//2
template <class InputIter, class OutputIter, class BinaryOp>
OutputIter adjacent_difference(InputIter first, InputIter last, OutputIter result, BinaryOp binary_op)
{
    if(first==last) return result;
    *result = *first;
    auto value=*first;
    while(++first!=last)
    {
        auto tmp=*first;
        *++result=binary_op(tmp, value);
        value=tmp;
    }
    return ++result;
}

/*
inner_product
1. 以init为初值，计算两个区间的内积
2. 自定义operator+和operator*
*/
//1
template <class InputIter1, class InputIter2, class T>
T inner_product(InputIter1 first1, InputIter1 last1, InputIter2 first2, T init)
{
    for(; first1!=last1; ++first1, ++first2)
    {
        init = init+(*first1*first2);
    }
    return init;
}
//2
template <class InputIter1, class InputIter2, class T, class BinaryOp1, class BinaryOp2>
T inner_product(InputIter1 first1, InputIter1 last1, InputIter2 first2, T init, BinaryOp1 binary_op1, BinaryOp2 binary_op2)
{
    for(; first1!=last1; ++first1, ++first2)
    {
        init = binary_op1(init, binary_op2(*first1, *first2));
    }
    return init;
}


/*
iota
填充[first, last)，以value为初值开始递增
*/
template <class ForwardIter, class T>
void iota(ForwardIter first, ForwardIter last, T value)
{
    while(first != last)
    {
        *first++=value;
        ++value;
    }
}


/*
partial_sum
1. 计算局部累计求和，结果保存到result起始区间上
2. 进行局部自定义二元操作
*/
//1
template <class InputIter, class OutputIter>
OutputIter partial_sum(InputIter first, InputIter last, OutputIter result)
{
    if(first==last) return result;
    *result=*first;
    auto value=*first;
    while(++first!=last)
    {
        value=value+*first;
        *++result=value;
    }
    return ++result;
}
//2
template <class InputIter, class OutputIter, class BinaryOp>
OutputIter partial_sum(InputIter first, InputIter last, OutputIter result, BinaryOp binary_op)
{
    if(first==last) return result;
    *result=*first;
    auto value=*first;
    while(++first!=last)
    {
        value=binary_op(value, *first);
        *++result=value;
    }
    return ++result;
}


}   // end orange_stl

#endif