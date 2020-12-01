#ifndef __ORANGE_EXCEPTDEF_H__
#define __ORANGE_EXCEPTDEF_H__

#include <stdexcept>

#include <cassert>

namespace orange_stl
{

#define ORANGE_STL_DEBUG(expr) \
  assert(expr)  

#define THROW_LENGTH_ERROR_IF(expr, what) \
  if ((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what) \
  if ((expr)) throw std::out_of_range(what)

#define THROW_RUNTIME_ERROR_IF(expr, what) \
  if ((expr)) throw std::runtime_error(what)

} // namepsace orange_stl

#endif // !__ORANGE_EXCEPTDEF_H__

