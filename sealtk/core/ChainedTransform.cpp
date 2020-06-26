/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/ChainedTransform.hpp>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
ChainedTransform::ChainedTransform(
  std::initializer_list<kv::transform_2d_sptr> in)
{
  this->transforms = in;
}

// ----------------------------------------------------------------------------
kv::transform_2d_sptr ChainedTransform::clone() const
{
  auto copy = std::make_shared<ChainedTransform>();

  for (auto const& xf : this->transforms)
  {
    copy->transforms.emplace_back(std::move(xf->clone()));
  }

  return copy;
}

// ----------------------------------------------------------------------------
kv::transform_2d_sptr ChainedTransform::inverse_() const
{
  throw std::logic_error("not implemented");
}

// ----------------------------------------------------------------------------
kv::vector_2d ChainedTransform::map(kv::vector_2d const& p) const
{
  auto out = p;

  for (auto const& xf : this->transforms)
  {
    out = xf->map(out);
  }

  return out;
}

} // namespace core

} // namespace sealtk
