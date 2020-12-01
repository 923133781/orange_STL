#ifndef __ORANGE_UTIL_H__
#define __ORANGE_UTIL_H__

#include <cstddef>

#include "orange_type_traits.h"

namespace orange_stl
{
//--------------------------------------------------------------//
//--------------------move--------------------------------------//

template <class T>
typename std::remove_reference<T>::type&& move(T&& arg) noexcept
{
  return static_cast<typename std::remove_reference<T>::type&&>(arg);
}
/*
std::remove_reference为C++0x标准库中的元函数，其功能为去除类型中的引用。

以下语法形式将把表达式 t 转换为T类型的右值（准确的说是无名右值引用，是右值的一种）
static_cast<T&&>(t)

无名的右值引用是右值,具名的右值引用是左值
*/
//--------------------------------------------------------------//



//--------------------------------------------------------------//
//-----------------------forward--------------------------------// 
template <class T>
T&& forward(typename std::remove_reference<T>::type& arg) noexcept
{
  return static_cast<T&&>(arg);
}

template <class T>
T&& forward(typename std::remove_reference<T>::type&& arg) noexcept
{
  static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
  return static_cast<T&&>(arg);
}
//---------------------------------------------------------------//


//---------------------------------------------------------------//
//-------------------------swap----------------------------------//
template <class Tp>
void swap(Tp& lhs, Tp& rhs)
{
  //使用move将左值变为右值引用，可以减少拷贝的过程
  auto tmp(orange_stl::move(lhs));
  lhs = orange_stl::move(rhs);
  rhs = orange_stl::move(tmp);
}

template <class ForwardIter1, class ForwardIter2>
ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2)
{
  for (; first1 != last1; ++first1, (void) ++first2)
    orange_stl::swap(*first1, *first2);
  return first2;
}

template <class Tp, size_t N>
void swap(Tp(&a)[N], Tp(&b)[N])
{
  orange_stl::swap_range(a, a + N, b);
}
//------------------------------------------------------------------//



// -----------------------------------------------------------------//
//---------------------------- pair---------------------------------//

// 结构体模板 : pair
// 两个模板参数分别表示两个数据的类型
// 用 first 和 second 来分别取出第一个数据和第二个数据
template <class Ty1, class Ty2>
struct pair
{
  typedef Ty1    first_type;
  typedef Ty2    second_type;

  first_type first;    // 保存第一个数据
  second_type second;  // 保存第二个数据

  // default constructiable
  template <class Other1 = Ty1, class Other2 = Ty2,
    typename = typename std::enable_if<
    std::is_default_constructible<Other1>::value &&
    std::is_default_constructible<Other2>::value, void>::type>
    constexpr pair()
    : first(), second()
  {
  }

  // implicit constructiable for this type
  template <class U1 = Ty1, class U2 = Ty2,
    typename std::enable_if<
    std::is_copy_constructible<U1>::value &&
    std::is_copy_constructible<U2>::value &&
    std::is_convertible<const U1&, Ty1>::value &&
    std::is_convertible<const U2&, Ty2>::value, int>::type = 0>
    constexpr pair(const Ty1& a, const Ty2& b)
    : first(a), second(b)
  {
  }

  // explicit constructible for this type
  template <class U1 = Ty1, class U2 = Ty2,
    typename std::enable_if<
    std::is_copy_constructible<U1>::value &&
    std::is_copy_constructible<U2>::value &&
    (!std::is_convertible<const U1&, Ty1>::value ||
     !std::is_convertible<const U2&, Ty2>::value), int>::type = 0>
    explicit constexpr pair(const Ty1& a, const Ty2& b)
    : first(a), second(b)
  {
  }

  pair(const pair& rhs) = default;
  pair(pair&& rhs) = default;

  // implicit constructiable for other type
  template <class Other1, class Other2,
    typename std::enable_if<
    std::is_constructible<Ty1, Other1>::value &&
    std::is_constructible<Ty2, Other2>::value &&
    std::is_convertible<Other1&&, Ty1>::value &&
    std::is_convertible<Other2&&, Ty2>::value, int>::type = 0>
    constexpr pair(Other1&& a, Other2&& b)
    : first(orange_stl::forward<Other1>(a)),
    second(orange_stl::forward<Other2>(b))
  {
  }

  // explicit constructiable for other type
  template <class Other1, class Other2,
    typename std::enable_if<
    std::is_constructible<Ty1, Other1>::value &&
    std::is_constructible<Ty2, Other2>::value &&
    (!std::is_convertible<Other1, Ty1>::value ||
     !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
    explicit constexpr pair(Other1&& a, Other2&& b)
    : first(orange_stl::forward<Other1>(a)),
    second(orange_stl::forward<Other2>(b))
  {
  }

  // implicit constructiable for other pair
  template <class Other1, class Other2,
    typename std::enable_if<
    std::is_constructible<Ty1, const Other1&>::value &&
    std::is_constructible<Ty2, const Other2&>::value &&
    std::is_convertible<const Other1&, Ty1>::value &&
    std::is_convertible<const Other2&, Ty2>::value, int>::type = 0>
    constexpr pair(const pair<Other1, Other2>& other)
    : first(other.first),
    second(other.second)
  {
  }

  // explicit constructiable for other pair
  template <class Other1, class Other2,
    typename std::enable_if<
    std::is_constructible<Ty1, const Other1&>::value &&
    std::is_constructible<Ty2, const Other2&>::value &&
    (!std::is_convertible<const Other1&, Ty1>::value ||
     !std::is_convertible<const Other2&, Ty2>::value), int>::type = 0>
    explicit constexpr pair(const pair<Other1, Other2>& other)
    : first(other.first),
    second(other.second)
  {
  }

  // implicit constructiable for other pair
  template <class Other1, class Other2,
    typename std::enable_if<
    std::is_constructible<Ty1, Other1>::value &&
    std::is_constructible<Ty2, Other2>::value &&
    std::is_convertible<Other1, Ty1>::value &&
    std::is_convertible<Other2, Ty2>::value, int>::type = 0>
    constexpr pair(pair<Other1, Other2>&& other)
    : first(orange_stl::forward<Other1>(other.first)),
    second(orange_stl::forward<Other2>(other.second))
  {
  }

  // explicit constructiable for other pair
  template <class Other1, class Other2,
    typename std::enable_if<
    std::is_constructible<Ty1, Other1>::value &&
    std::is_constructible<Ty2, Other2>::value &&
    (!std::is_convertible<Other1, Ty1>::value ||
     !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
    explicit constexpr pair(pair<Other1, Other2>&& other)
    : first(orange_stl::forward<Other1>(other.first)),
    second(orange_stl::forward<Other2>(other.second))
  {
  }

  // copy assign for this pair
  pair& operator=(const pair& rhs)
  {
    if (this != &rhs)
    {
      first = rhs.first;
      second = rhs.second;
    }
    return *this;
  }

  // move assign for this pair
  pair& operator=(pair&& rhs)
  {
    if (this != &rhs)
    {
      first = orange_stl::move(rhs.first);
      second = orange_stl::move(rhs.second);
    }
    return *this;
  }

  // copy assign for other pair
  template <class Other1, class Other2>
  pair& operator=(const pair<Other1, Other2>& other)
  {
    first = other.first;
    second = other.second;
    return *this;
  }

  // move assign for other pair
  template <class Other1, class Other2>
  pair& operator=(pair<Other1, Other2>&& other)
  {
    first = orange_stl::forward<Other1>(other.first);
    second = orange_stl::forward<Other2>(other.second);
    return *this;
  }

  ~pair() = default;

  void swap(pair& other)
  {
    if (this != &other)
    {
      orange_stl::swap(first, other.first);
      orange_stl::swap(second, other.second);
    }
  }

};
//---------------------end pair--------------------------//

/* ----------------------------------------------------- */
// ------------------重载比较操作符 --------------------- //
template <class Ty1, class Ty2>
bool operator==(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
  return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class Ty1, class Ty2>
bool operator<(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
  return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
}

template <class Ty1, class Ty2>
bool operator!=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
  return !(lhs == rhs);
}

template <class Ty1, class Ty2>
bool operator>(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
  return rhs < lhs;
}

template <class Ty1, class Ty2>
bool operator<=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
  return !(rhs < lhs);
}

template <class Ty1, class Ty2>
bool operator>=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
{
  return !(lhs < rhs);
}

// 重载 orange_stl 的 swap
template <class Ty1, class Ty2>
void swap(pair<Ty1, Ty2>& lhs, pair<Ty1, Ty2>& rhs)
{
  lhs.swap(rhs);
}

// 全局函数，让两个数据成为一个 pair
template <class Ty1, class Ty2>
pair<Ty1, Ty2> make_pair(Ty1&& first, Ty2&& second)
{
  return pair<Ty1, Ty2>(orange_stl::forward<Ty1>(first), orange_stl::forward<Ty2>(second));
}

}

#endif // !__ORANGE_UTIL_H__