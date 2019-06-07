/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/AbstractItemRepresentation.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/DateUtils.hpp>

#include <QHash>
#include <QPalette>
#include <QUuid>

namespace kv = kwiver::vital;

namespace sealtk
{

namespace gui
{

// ============================================================================
class AbstractItemRepresentationPrivate
{
public:
  QVector<int> columnRoles;

  bool sortNeeded = false;
  ItemVisibilityMode visibilityMode = OmitHidden;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(AbstractItemRepresentation)

// ----------------------------------------------------------------------------
AbstractItemRepresentation::AbstractItemRepresentation(QObject* parent)
  : QSortFilterProxyModel{parent}, d_ptr{new AbstractItemRepresentationPrivate}
{
  // Our filtering (and likely that of our subclasses) is dependent on the
  // logical data model's data; therefore, we need to re-filter and/or re-sort
  // when the underlying data changes, and so we enable doing so by default
  this->setDynamicSortFilter(true);
}

// ----------------------------------------------------------------------------
AbstractItemRepresentation::~AbstractItemRepresentation()
{
}

// ----------------------------------------------------------------------------
int AbstractItemRepresentation::roleForColumn(int column) const
{
  QTE_D();
  return d->columnRoles.value(column, -1);
}

// ----------------------------------------------------------------------------
void AbstractItemRepresentation::setColumnRoles(
  std::initializer_list<int> const& roles)
{
  QTE_D();
  d->columnRoles = roles;
}

// ----------------------------------------------------------------------------
ItemVisibilityMode AbstractItemRepresentation::itemVisibilityMode() const
{
  QTE_D();
  return d->visibilityMode;
}

// ----------------------------------------------------------------------------
void AbstractItemRepresentation::setItemVisibilityMode(ItemVisibilityMode mode)
{
  QTE_D();

  if (d->visibilityMode != mode)
  {
    d->visibilityMode = mode;
    this->invalidateFilter();
  }
}

// ----------------------------------------------------------------------------
int AbstractItemRepresentation::columnCount(QModelIndex const& parent) const
{
  QTE_D();

  Q_UNUSED(parent);
  return d->columnRoles.size();
}

// ----------------------------------------------------------------------------
QVariant AbstractItemRepresentation::data(
  QModelIndex const& index, int role) const
{
  if (index.isValid() && this->sourceModel())
  {
    switch (role)
    {
      case Qt::DisplayRole:
      case Qt::DecorationRole:
      case Qt::TextAlignmentRole:
      case Qt::ToolTipRole:
      {
        QTE_D();
        auto const c = index.column();
        if (c < d->columnRoles.size())
        {
          return this->data(this->mapToSource(index), role,
                            d->columnRoles[c]);
        }
        break;
      }

      case Qt::ForegroundRole:
        if (!this->sourceData(index, core::VisibilityRole).toBool())
        {
          // Gray out hidden items
          return QPalette().brush(QPalette::Disabled, QPalette::WindowText);
        }
        break;

      default:
        if (role >= core::ItemTypeRole && role < core::UserRole)
        {
          // Requests for logical data roles are passed through
          return this->sourceData(index, role);
        }
        break;
    }
  }

  return QSortFilterProxyModel::data(index, role);
}

// ----------------------------------------------------------------------------
QVariant AbstractItemRepresentation::data(
  QModelIndex const& sourceIndex, int presentationRole, int dataRole) const
{
  if (!sourceIndex.isValid() || !this->sourceModel())
  {
    return {};
  }

  auto* const sm = this->sourceModel();

  switch (presentationRole)
  {
    case Qt::ToolTipRole:
      switch (dataRole)
      {
        case core::StartTimeRole:
        case core::EndTimeRole:
        {
          auto const& dt = core::vitalTimeToQDateTime(
            sm->data(sourceIndex, dataRole).value<kv::timestamp::time_t>());
          auto const& ds = core::dateString(dt);
          auto const& ts = core::timeString(dt);
          return QStringLiteral("%1 %2").arg(ds, ts);
        }

        // TODO classification, confidence

        default:
          break;
      }

      // If tool tip was not handled by above, fall through and use display
      // text as tool tip
      Q_FALLTHROUGH();

    case Qt::DisplayRole:
      switch (dataRole)
      {
        case core::ItemTypeRole:
          break; // TODO

        case core::NameRole:
          return sm->data(sourceIndex, dataRole);

        case core::UniqueIdentityRole:
        {
          auto const& uuid = sm->data(sourceIndex, dataRole).value<QUuid>();
          return (uuid.isNull() ? QStringLiteral("(null)") : uuid.toString());
        }

        case core::StartTimeRole:
        case core::EndTimeRole:
        {
          return core::timeString(
            sm->data(sourceIndex, dataRole).value<kv::timestamp::time_t>());
        }

        // TODO classification, confidence

        default:
          break;
      }
      break;

    case Qt::DecorationRole:
      // TODO
      break;

    case Qt::TextAlignmentRole:
      // TODO
      break;

    default:
      break;
  }

  return {};
}

// ----------------------------------------------------------------------------
QVariant AbstractItemRepresentation::headerData(
  int section, Qt::Orientation orientation, int role) const
{
  QTE_D();

  if (orientation == Qt::Horizontal &&
      section >= 0 && section < d->columnRoles.size())
  {
    switch (role)
    {
      case Qt::DisplayRole:
        switch (d->columnRoles[section])
        {
          case core::ItemTypeRole:        return QStringLiteral("Type");
          case core::NameRole:            return QStringLiteral("Name");
          case core::LogicalIdentityRole: return QStringLiteral("ID");
          case core::UniqueIdentityRole:  return QStringLiteral("UUID");
          case core::StartTimeRole:       return QStringLiteral("Start Time");
          case core::EndTimeRole:         return QStringLiteral("End Time");
          default: return {};
        }
        Q_UNREACHABLE();
        break;

      case Qt::DecorationRole:
        return {};

      default:
        break;
    }
  }

  return QSortFilterProxyModel::headerData(section, orientation, role);
}

// ----------------------------------------------------------------------------
bool AbstractItemRepresentation::lessThan(
  QModelIndex const& left, QModelIndex const& right) const
{
  QTE_D();

  auto const c = left.column();
  Q_ASSERT(c == right.column());
  Q_ASSERT(c >= 0);

  if (c < d->columnRoles.size())
  {
    auto const dataRole = d->columnRoles[c];
    return this->lessThan(left, right, dataRole);
  }

  return QSortFilterProxyModel::lessThan(left, right);
}

// ----------------------------------------------------------------------------
bool AbstractItemRepresentation::lessThan(
  QModelIndex const& left, QModelIndex const& right, int role) const
{
  if (!this->sourceModel())
  {
    return false;
  }

  Q_ASSERT(left.column() == right.column());

  auto* const sm = this->sourceModel();
  auto const& leftData = sm->data(left, role);
  auto const& rightData = sm->data(right, role);

  switch (role)
  {
    // String comparisons
    case core::NameRole:
    case core::UniqueIdentityRole:
      return QString::localeAwareCompare(leftData.toString(),
                                         rightData.toString());

    // Boolean comparisons
    case core::VisibilityRole:
    case core::UserVisibilityRole:
      return leftData.toBool() < rightData.toBool();

    // Integer comparisons
    // TODO classification
    case core::ItemTypeRole:
      return leftData.toInt() < rightData.toInt();

    // Floating-point comparisons
    // TODO confidence
    //   return leftData.toDouble() < rightData.toDouble();

    // Timestamp comparisons
    case core::StartTimeRole:
    case core::EndTimeRole:
    {
      auto const lt = leftData.value<kv::timestamp::time_t>();
      auto const rt = rightData.value<kv::timestamp::time_t>();
      return lt < rt;
    }

    default:
      return false;
  }
}

// ----------------------------------------------------------------------------
bool AbstractItemRepresentation::filterAcceptsRow(
  int sourceRow, QModelIndex const& sourceParent) const
{
  QTE_D();

  auto* const sm = this->sourceModel();
  if (sm && d->visibilityMode == OmitHidden)
  {
    auto const& si = sm->index(sourceRow, 0, sourceParent);

    if (!sm->data(si, core::VisibilityRole).toBool())
    {
      return false;
    }
  }

  return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

// ----------------------------------------------------------------------------
bool AbstractItemRepresentation::filterAcceptsColumn(
  int sourceColumn, QModelIndex const& sourceParent) const
{
  if (this->columnCount(this->mapFromSource(sourceParent)) < sourceColumn)
  {
    // The underlying model is expected to only "really" have one column, since
    // data roles are used to get data that is mapped to display columns by the
    // proxy model, but to claim to have a large number of columns because
    // QSortFilterProxyModel will only show a maximum of the source model's
    // columnCount() columns. Therefore, by default we ignore columns whose
    // index is greater than our column count, which should be equal to the
    // number of columns that we actually map.
    return false;
  }

  return QSortFilterProxyModel::filterAcceptsColumn(
    sourceColumn, sourceParent);
}

// ----------------------------------------------------------------------------
void AbstractItemRepresentation::sort(int column, Qt::SortOrder order)
{
  QTE_D();

  QSortFilterProxyModel::sort(column, order);

  // Determine, if we are sorting (column >= 0), if QSortFilterProxyModel
  // figured out what column to sort; if not, we may need to force a sort later
  d->sortNeeded =
    column >= 0 && !this->mapToSource(this->index(0, column, {})).isValid();
}

// ----------------------------------------------------------------------------
void AbstractItemRepresentation::updateSort()
{
  QTE_D();

  // Our data has changed; if we previously failed to sort...
  if (d->sortNeeded)
  {
    // ...then attempt to do so now (if dynamic sorting is enabled)
    if (this->dynamicSortFilter())
    {
      // NOTE: Must clear dynamic sort filter first or sort() will decline to
      //       actually do anything; this ends up sorting twice, but there
      //       doesn't seem to be any way around that
      this->setDynamicSortFilter(false);
      this->sort(this->sortColumn(), this->sortOrder());
      this->setDynamicSortFilter(true);
    }
  }
}

} // namespace gui

} // namespace sealtk
