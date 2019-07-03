/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_UnsharedPointer_hpp
#define sealtk_core_UnsharedPointer_hpp

#include <memory>

namespace sealtk
{

namespace core
{

// ============================================================================
template <typename T>
class UnsharedPointer : public std::shared_ptr<T>
{
  using super = std::shared_ptr<T>;

public:
  UnsharedPointer() = default;
  UnsharedPointer(UnsharedPointer&& other) = default;
  UnsharedPointer(UnsharedPointer const& other);

  UnsharedPointer(super&& other) : super{other} {}

  ~UnsharedPointer() = default;

  UnsharedPointer& operator=(UnsharedPointer&& other) = default;
  UnsharedPointer& operator=(UnsharedPointer const& other);

protected:
  struct CopyHelper
  {
    static std::shared_ptr<T> clone(std::shared_ptr<T> const& other);
  };
};

// ----------------------------------------------------------------------------
template <typename T>
UnsharedPointer<T>::UnsharedPointer(UnsharedPointer<T> const& other)
  : std::shared_ptr<T>{CopyHelper::clone(other)}
{
}

// ----------------------------------------------------------------------------
template <typename T>
UnsharedPointer<T>& UnsharedPointer<T>::operator=(
  UnsharedPointer<T> const& other)
{
  *this = CopyHelper::clone(other);
  return *this;
}

// ----------------------------------------------------------------------------
template <typename T>
std::shared_ptr<T> UnsharedPointer<T>::CopyHelper::clone(
  std::shared_ptr<T> const& other)
{
  return other->clone();
}

} // namespace core

} // namespace sealtk

#endif
