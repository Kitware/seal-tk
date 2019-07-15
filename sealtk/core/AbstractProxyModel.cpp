/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/AbstractProxyModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/types/timestamp.h>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

// ----------------------------------------------------------------------------
bool AbstractProxyModel::lessThan(
  QVariant const& left, QVariant const& right, int role) const
{
  switch (role)
  {
    // String comparisons
    case core::NameRole:
    case core::UniqueIdentityRole:
      return QString::localeAwareCompare(left.toString(),
                                         right.toString());

    // Boolean comparisons
    case core::VisibilityRole:
    case core::UserVisibilityRole:
      return left.toBool() < right.toBool();

    // Integer comparisons
    // TODO classification
    case core::ItemTypeRole:
      return left.toInt() < right.toInt();

    // Floating-point comparisons
    // TODO confidence
    //   return left.toDouble() < right.toDouble();

    // Timestamp comparisons
    case core::StartTimeRole:
    case core::EndTimeRole:
    {
      auto const lt = left.value<kv::timestamp::time_t>();
      auto const rt = right.value<kv::timestamp::time_t>();
      return lt < rt;
    }

    default:
      return false;
  }
}

} // namespace core

} // namespace sealtk
