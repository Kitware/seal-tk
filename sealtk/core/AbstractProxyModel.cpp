/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/core/AbstractProxyModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/types/timestamp.h>

#include <QUuid>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace core
{

namespace // anonymous
{

// ----------------------------------------------------------------------------
bool localeAwareLessThan(QString const& left, QString const& right)
{
  return QString::localeAwareCompare(left, right) < 0;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
bool AbstractProxyModel::isValidData(QVariant const& data, int role)
{
  switch (role)
  {
    case core::UniqueIdentityRole:
      return data.canConvert<QUuid>();

    case core::NameRole:
      switch (data.type())
      {
      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::LongLong:
      case QVariant::ULongLong:
        return true;
      default:
        return data.canConvert<QString>();
      }

    // String comparisons
    case core::ClassificationTypeRole:
      return data.canConvert<QString>();

    // Boolean comparisons
    case core::VisibilityRole:
    case core::UserVisibilityRole:
      return data.canConvert<bool>();

    // Integer comparisons
    // TODO classification
    case core::ItemTypeRole:
      return data.canConvert<int>();

    // Floating-point comparisons
    // TODO confidence
    case core::ClassificationScoreRole:
      return data.canConvert<double>();

    case core::StartTimeRole:
    case core::EndTimeRole:
      return data.canConvert<kv::timestamp::time_t>();

    default:
      return false;
  }
}

// ----------------------------------------------------------------------------
bool AbstractProxyModel::lessThan(
  QVariant const& left, QVariant const& right, int role) const
{
  switch (role)
  {
    case core::UniqueIdentityRole:
      return left.toUuid() < right.toUuid();

    case core::NameRole:
      switch (left.type())
      {
        case QVariant::Int:
        case QVariant::LongLong:
          return left.toLongLong() < right.toLongLong();
        case QVariant::UInt:
        case QVariant::ULongLong:
          return left.toULongLong() < right.toULongLong();
        default:
          return localeAwareLessThan(left.toString(), right.toString());
      }

    // String comparisons
    case core::ClassificationTypeRole:
      return localeAwareLessThan(left.toString(), right.toString());

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
    case core::ClassificationScoreRole:
      return left.toDouble() < right.toDouble();

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

// ----------------------------------------------------------------------------
void AbstractProxyModel::invalidateVisibility()
{
  if (auto const rows = this->rowCount())
  {
    auto const& first = this->index(0, 0);
    auto const& last = this->index(rows, 0);

    emit this->dataChanged(first, last, {VisibilityRole});
  }
}

} // namespace core

} // namespace sealtk
