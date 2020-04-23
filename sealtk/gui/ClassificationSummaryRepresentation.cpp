/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/ClassificationSummaryRepresentation.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/range/indirect.h>
#include <vital/range/iota.h>

#include <QHash>
#include <QVector>

namespace sc = sealtk::core;

namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace gui
{

namespace // anonymous
{

// ============================================================================
enum Column : int
{
  TypeColumn,
  CountColumn,
  ColumnCount
};

// ============================================================================
struct Record
{
  QString type;
  int count;
};

}

// ============================================================================
class ClassificationSummaryRepresentationPrivate
{
public:
  void recompute(ClassificationSummaryRepresentation* q);

  QAbstractItemModel* sourceModel = nullptr;
  QVector<Record> data;
  QHash<QString, int> map;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(ClassificationSummaryRepresentation)

// ----------------------------------------------------------------------------
ClassificationSummaryRepresentation::ClassificationSummaryRepresentation(
  QObject* parent)
  : QAbstractItemModel{parent},
    d_ptr{new ClassificationSummaryRepresentationPrivate}
{
}

// ----------------------------------------------------------------------------
ClassificationSummaryRepresentation::~ClassificationSummaryRepresentation()
{
}

// ----------------------------------------------------------------------------
QAbstractItemModel* ClassificationSummaryRepresentation::sourceModel() const
{
  QTE_D();
  return d->sourceModel;
}

// ----------------------------------------------------------------------------
void ClassificationSummaryRepresentation::setSourceModel(
  QAbstractItemModel* sourceModel)
{
  QTE_D();

  if (d->sourceModel != sourceModel)
  {
    d->sourceModel = sourceModel;
    d->recompute(this);

    if (sourceModel)
    {
      auto recompute = [this, d]{ d->recompute(this); };

      connect(sourceModel, &QAbstractItemModel::rowsInserted, this, recompute);
      connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, recompute);
      connect(sourceModel, &QAbstractItemModel::dataChanged, this, recompute);

      connect(sourceModel, &QObject::destroyed,
              this, [this]{ this->setSourceModel(nullptr); });
    }
  }
}

// ----------------------------------------------------------------------------
int ClassificationSummaryRepresentation::rowCount(
  QModelIndex const& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  QTE_D();
  return d->data.size();
}

// ----------------------------------------------------------------------------
int ClassificationSummaryRepresentation::columnCount(
  QModelIndex const& parent) const
{
  Q_UNUSED(parent);
  return ColumnCount;
}

// ----------------------------------------------------------------------------
QModelIndex ClassificationSummaryRepresentation::parent(
  QModelIndex const&) const
{
  return {};
}

// ----------------------------------------------------------------------------
QModelIndex ClassificationSummaryRepresentation::index(
  int row, int column, QModelIndex const& parent) const
{
  if (parent.isValid())
  {
    return {};
  }
  return this->createIndex(row, column);
}

// ----------------------------------------------------------------------------
QVariant ClassificationSummaryRepresentation::data(
  QModelIndex const& index, int role) const
{
  if (index.isValid() && this->sourceModel() && role == Qt::DisplayRole)
  {
    QTE_D();

    auto const& record = d->data[index.row()];

    switch (index.column())
    {
      case TypeColumn:  return record.type;
      case CountColumn: return record.count;
    }
  }

  return {};
}

// ----------------------------------------------------------------------------
QVariant ClassificationSummaryRepresentation::headerData(
  int section, Qt::Orientation orientation, int role) const
{
  QTE_D();

  if (orientation == Qt::Horizontal && section >= 0 && section < ColumnCount)
  {
    switch (role)
    {
      case Qt::DisplayRole:
        switch (section)
        {
#define QSL QStringLiteral
          case Column::TypeColumn:  return QSL("Type");
          case Column::CountColumn: return QSL("Count");
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

  return this->QAbstractItemModel::headerData(section, orientation, role);
}

// ----------------------------------------------------------------------------
void ClassificationSummaryRepresentationPrivate::recompute(
  ClassificationSummaryRepresentation* q)
{
  // Extract information from underlying model
  auto newData = QHash<QString, Record>{};

  if (this->sourceModel)
  {
    for (auto const row : kvr::iota(this->sourceModel->rowCount()))
    {
      auto const& index = this->sourceModel->index(row, 0);

      auto const& vd = this->sourceModel->data(index, sc::VisibilityRole);
      if (!vd.toBool())
      {
        continue;
      }

      auto const& cd = this->sourceModel->data(index, sc::ClassificationRole);
      for (auto const& c : cd.toHash() | kvr::indirect)
      {
        if (c.value().toDouble() > 0.0)
        {
          auto const& type = c.key();
          auto& newRecord = newData[type];
          if (newRecord.type.isEmpty())
          {
            newRecord.type = type;
          }

          ++newRecord.count;
        }
      }
    }
  }

  // First, identify and remove rows with zero counts
  if (!this->data.isEmpty())
  {
    auto start = decltype(this->data.size()){0};
    auto count = decltype(start){0};

    auto shuffle = [&]{
      // Adjust map
      for (auto& index : this->map)
      {
        if (index > start)
        {
          index -= count;
        }
      }

      // Remove rows from internal data
      q->beginRemoveRows({}, start, start + count - 1);
      this->data.remove(start, count);
      q->endRemoveRows();

      // Update/reset counters
      start += count;
      count = 0;
    };

    while (start + count < this->data.size())
    {
      auto const& type = this->data[start + count].type;
      if (newData.contains(type))
      {
        if (count)
        {
          shuffle();
        }
        else
        {
          ++start;
        }
      }
      else
      {
        this->map.remove(type);
        ++count;
      }
    }
    if (count)
    {
      shuffle();
    }
  }

  // Merge in updates and determine what rows have changed
  auto newRecords = QVector<Record>{};
  auto firstRow = this->data.size();
  auto lastRow = decltype(firstRow){-1};

  for (auto const& newRecord : newData)
  {
    auto const i = this->map.value(newRecord.type, -1);
    if (i < 0)
    {
      newRecords.append(newRecord);
    }
    else
    {
      auto& oldRecord = this->data[i];
      if (oldRecord.count != newRecord.count)
      {
        oldRecord.count = newRecord.count;
        firstRow = std::min(firstRow, i);
        lastRow = std::max(lastRow, i);
      }
    }
  }

  if (lastRow >= firstRow)
  {
    // Conservatively assume that everything between the first and last
    // modified rows has changed; this shouldn't hurt our users too much, and
    // figuring out how to emit more fine-grained notifications is a pain, on
    // top of which the extra signal emissions may well outweigh any benefit
    emit q->dataChanged(q->index(firstRow, CountColumn),
                        q->index(lastRow, CountColumn));
  }

  // Add new rows
  if (!newRecords.isEmpty())
  {
    auto const start = this->data.size();
    q->beginInsertRows({}, start, start + newRecords.size() - 1);
    this->data.append(newRecords);
    q->endInsertRows();

    // Update map
    for (auto const i : kvr::iota(newRecords.size()))
    {
      this->map.insert(newRecords[i].type, start + i);
    }
  }
}

} // namespace gui

} // namespace sealtk
