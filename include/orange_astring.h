#ifndef __ORANGE_STL_ASTRING_H__
#define __ORANGE_STL_ASTRING_H__

// 定义了 string, wstring, u16string, u32string 类型

#include "orange_basic_string.h"

namespace orange_stl
{

using string    = orange_stl::basic_string<char>;
using wstring   = orange_stl::basic_string<wchar_t>;
using u16string = orange_stl::basic_string<char16_t>;
using u32string = orange_stl::basic_string<char32_t>;

}
#endif // !__ORANGE_STL_ASTRING_H__