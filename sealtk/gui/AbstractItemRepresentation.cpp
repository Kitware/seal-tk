/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/AbstractItemRepresentation.hpp>

#include <sealtk/core/DataModelTypes.hpp>
#include <sealtk/core/DateUtils.hpp>

#include <vital/range/indirect.h>

#include <QHash>
#include <QPalette>
#include <QUuid>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace gui
{

using sealtk::core::AbstractProxyModel;

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
  : AbstractProxyModel{parent}, d_ptr{new AbstractItemRepresentationPrivate}
{
  // Our filtering (and likely that of our subclasses) is dependent on the
  // logical data model's data; therefore, we need to re-filter and/or re-sort
  // when the underlying data changes, and so we enable doing so by default
  this->setDynamicSortFilter(true);
  this->setFilterRole(core::VisibilityRole);
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

  return this->AbstractProxyModel::data(index, role);
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

        case core::ClassificationTypeRole:
        case core::ClassificationScoreRole:
        {
          static auto const tableTemplate =
            QStringLiteral("<table>%1</table>");
          static auto const rowTemplate =
            QStringLiteral("<tr>"
                           "<td>%1</td>"
                           "<td>&nbsp;</td>"
                           "<td style=\"align: right;\">%2</td>"
                           "</tr>");

          auto const& c = sm->data(sourceIndex, core::ClassificationRole);

          auto rows = QString{};
          for (auto const& i : c.toHash() | kvr::indirect)
          {
            auto const s = QString::number(i.value().toDouble());
            rows += rowTemplate.arg(i.key(), s);
          }

          return tableTemplate.arg(rows);
        }

        // TODO confidence

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
        case core::ClassificationTypeRole:
        case core::ClassificationScoreRole:
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

        // TODO confidence

        default:
          break;
      }
      break;

    case Qt::DecorationRole:
      // TODO
      break;

    case Qt::TextAlignmentRole:
      switch (dataRole)
      {
        case core::ClassificationScoreRole:
          return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);

        default:
          break;
      }

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
#define QSL QStringLiteral
          case core::ItemTypeRole:            return QSL("Type");
          case core::NameRole:                return QSL("Name");
          case core::LogicalIdentityRole:     return QSL("ID");
          case core::UniqueIdentityRole:      return QSL("UUID");
          case core::StartTimeRole:           return QSL("Start Time");
          case core::EndTimeRole:             return QSL("End Time");
          case core::ClassificationTypeRole:  return QSL("Type");
          case core::ClassificationScoreRole: return QSL("Score");
          default: return {};
#undef QSL
        }
        Q_UNREACHABLE();
        break;

      case Qt::DecorationRole:
        return {};

      default:
        break;
    }
  }

  return this->AbstractProxyModel::headerData(section, orientation, role);
}

// ----------------------------------------------------------------------------
void AbstractItemRepresentation::sort(int column, Qt::SortOrder order)
{
  this->setSortRole(this->roleForColumn(column));
  this->QSortFilterProxyModel::sort(column, order);
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

  return this->AbstractProxyModel::lessThan(left, right);
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
  return this->lessThan(sm->data(left, role), sm->data(right, role), role);
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

  return this->AbstractProxyModel::filterAcceptsRow(sourceRow, sourceParent);
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

  return this->AbstractProxyModel::filterAcceptsColumn(
    sourceColumn, sourceParent);
}

} // namespace gui

} // namespace sealtk
