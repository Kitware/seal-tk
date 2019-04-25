/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_util_unique_hpp
#define sealtk_util_unique_hpp

#include <memory>

namespace sealtk
{

#if __cplusplus < 201402L

// ----------------------------------------------------------------------------
template <typename T, typename... Args>
inline typename std::unique_ptr<T> make_unique(Args&&... args)
{
  return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

#else

using std::make_unique;

#endif

} // namespace sealtk

#endif
