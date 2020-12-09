#ifndef __ORANGE_TYPE_TRAITS_H__
#define __ORANGE_TYPE_TRAITS_H__

//使用标准头提取类型信息
#include <type_traits>

namespace orange_stl
{
    template <class T, T v>
    struct m_integral_constant
    {
        static constexpr T value = v;
    };

    template <bool b>
    using m_bool_constant = m_integral_constant<bool, b>;

    typedef m_bool_constant<true>  m_true_type;
    typedef m_bool_constant<false> m_false_type;


    template <class T1, class T2>
    struct pair;

    template <class T>
    struct is_pair : orange_stl ::m_false_type {};

    template <class T1, class T2>
    struct is_pair<orange_stl::pair<T1, T2>> : orange_stl::m_true_type {};
}


#endif // !__ORANGE_TYPE_TRAITS_H__
