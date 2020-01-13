/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_IdentityTransform_hpp
#define sealtk_core_IdentityTransform_hpp

#include <sealtk/core/Export.h>

#include <vital/types/transform_2d.h>

namespace sealtk
{

namespace core
{

class SEALTK_CORE_EXPORT IdentityTransform : public kwiver::vital::transform_2d
{
public:
  kwiver::vital::transform_2d_sptr clone() const override;

  kwiver::vital::vector_2d map(
    kwiver::vital::vector_2d const& p) const override
  { return p; }

protected:
  kwiver::vital::transform_2d_sptr inverse_() const override;
};

} // namespace core

} // namespace sealtk

#endif
