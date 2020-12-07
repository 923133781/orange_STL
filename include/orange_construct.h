#ifndef __ORANGE_CONSTRUCT_H__
#define __ORANGE_CONSTRUCT_H__

#include "orange_type_traits.h"
#include "orange_iterator.h"

namespace orange_stl
{

// construct 构造对象

template <class Ty>
void construct(Ty* ptr)
{
  ::new ((void*)ptr) Ty();        //placement new 一个对象创建在已经分配好的内存地址上
}

template <class Ty1, class Ty2>
void construct(Ty1* ptr, const Ty2& value)
{
  ::new ((void*)ptr) Ty1(value);  //placement new 一个对象创建在已经分配好的内存地址上
}

template <class Ty, class... Args>
void construct(Ty* ptr, Args&&... args)
{
  ::new ((void*)ptr) Ty(orange_stl::forward<Args>(args)...);
}

// destroy 将对象析构

template <class Ty>
void destroy_one(Ty*, std::true_type) {}

template <class Ty>
void destroy_one(Ty* pointer, std::false_type)
{
  if (pointer != nullptr)
  {
    pointer->~Ty();
  }
}

// 第三个参数是判别有无默认的构造函数
template <class ForwardIter>
void destroy_cat(ForwardIter , ForwardIter , std::true_type) {}

template <class ForwardIter>
void destroy_cat(ForwardIter first, ForwardIter last, std::false_type)
{
  for (; first != last; ++first)
    destroy(&*first);
}

template <class Ty>
void destroy(Ty* pointer)
{
  destroy_one(pointer, std::is_trivially_destructible<Ty>{});
}

template <class ForwardIter>
void destroy(ForwardIter first, ForwardIter last)
{
  destroy_cat(first, last, std::is_trivially_destructible<
              typename iterator_traits<ForwardIter>::value_type>{});
}

} // namespace orange_stl


#endif // !__ORANGE_CONSTRUCT_H__