/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/IdentityTransform.hpp>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
kv::transform_2d_sptr IdentityTransform::clone() const
{
  return std::make_shared<IdentityTransform>();
}

// ----------------------------------------------------------------------------
kv::transform_2d_sptr IdentityTransform::inverse_() const
{
  return std::make_shared<IdentityTransform>();
}

} // namespace core

} // namespace sealtk
