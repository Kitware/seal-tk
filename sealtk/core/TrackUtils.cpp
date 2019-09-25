/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/TrackUtils.hpp>

#include <vital/range/indirect.h>

#include <qtStlUtil.h>

#include <QVariantHash>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
kv::detected_object_type_sptr classificationToDetectedObjectType(
  QVariantHash const& in)
{
  if (in.isEmpty())
  {
    return nullptr;
  }

  auto out = std::make_shared<kv::detected_object_type>();
  for (auto const& c : in | kvr::indirect)
  {
    out->set_score(stdString(c.key()), c.value().toDouble());
  }
  return out;
}

} // namespace core

} // namespace sealtk
