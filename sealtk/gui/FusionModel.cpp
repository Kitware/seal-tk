/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/FusionModel.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/types/timestamp.h>

#include <vital/range/indirect.h>
#include <vital/range/iota.h>

#include <qtGet.h>

#include <QMultiHash>
#include <QSet>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace gui
{

namespace // anonymous
{

// ============================================================================
struct RowData
{
  qint64 id;
  QMultiHash<QAbstractItemModel*, int> rows;
};

// ----------------------------------------------------------------------------
struct MinTime
{
  using data_t = kv::timestamp::time_t;

  inline static data_t fuse(data_t a, data_t b)
  {
    return std::min(a, b);
  }
};

// ----------------------------------------------------------------------------
struct MaxTime
{
  using data_t = kv::timestamp::time_t;

  inline static data_t fuse(data_t a, data_t b)
  {
    return std::max(a, b);
  }
};

// ----------------------------------------------------------------------------
struct BooleanOr
{
  using data_t = bool;

  inline static data_t fuse(data_t a, data_t b)
  {
    return a || b;
  }
};

} // namespace <anonymous>

// ============================================================================
class FusionModelPrivate
{
public:
  FusionModelPrivate(FusionModel* q) : q_ptr{q} {}

  void addModelData(QAbstractItemModel* model);
  void removeModelData(QAbstractItemModel* model);

  void addModelData(QAbstractItemModel* model, int firstRow, int rowAfterLast);
  void shiftModelRows(QAbstractItemModel* model, int firstRow, int rowOffset);

  template <typename Fusor>

  static QVariant fuseData(RowData const& rowData, int role);
  static bool setData(RowData const& rowData, QVariant const& value, int role);

  QSet<QAbstractItemModel*> models;
  QHash<qint64, int> items;
  QVector<RowData> data;

private:
  QTE_DECLARE_PUBLIC_PTR(FusionModel);
  QTE_DECLARE_PUBLIC(FusionModel);
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(FusionModel)

// ----------------------------------------------------------------------------
FusionModel::FusionModel(QObject* parent)
  : AbstractItemModel{parent}, d_ptr{new FusionModelPrivate{this}}
{
}

// ----------------------------------------------------------------------------
FusionModel::~FusionModel()
{
}

// ----------------------------------------------------------------------------
int FusionModel::rowCount(QModelIndex const& parent) const
{
  QTE_D();

  if (parent.isValid())
  {
    return 0;
  }

  return d->data.size();
}

// ----------------------------------------------------------------------------
QVariant FusionModel::data(QModelIndex const& index, int role) const
{
  if (this->checkIndex(index, IndexIsValid | ParentIsInvalid))
  {
    QTE_D();

    auto const& r = d->data[index.row()];
    switch (role)
    {
      case core::NameRole:
      case core::LogicalIdentityRole:
        return r.id;

      case core::StartTimeRole:
        return d->fuseData<MinTime>(r, role);

      case core::EndTimeRole:
        return d->fuseData<MaxTime>(r, role);

      case core::UserVisibilityRole:
        return d->fuseData<BooleanOr>(r, role);

      default:
        break;
    }
  }

  return this->AbstractItemModel::data(index, role);
}

// ----------------------------------------------------------------------------
bool FusionModel::setData(
  QModelIndex const& index, QVariant const& value, int role)
{
  if (this->checkIndex(index, IndexIsValid | ParentIsInvalid))
  {
    QTE_D();

    auto const& r = d->data[index.row()];
    switch (role)
    {
      case core::UserVisibilityRole:
        return d->setData(r, value, role);

      default:
        break;
    }
  }

  return this->AbstractItemModel::setData(index, value, role);
}

// ----------------------------------------------------------------------------
void FusionModel::addModel(QAbstractItemModel* model)
{
  QTE_D();

  if (!d->models.contains(model))
  {
    this->beginResetModel();
    d->models.insert(model);
    d->addModelData(model);
    this->endResetModel();

    connect(model, &QObject::destroyed, this,
            [model, this]{ this->removeModel(model); });
    connect(model, &QAbstractItemModel::rowsInserted,
            [model, this](QModelIndex const& parent, int first, int last){
              if (!parent.isValid())
              {
                QTE_D();
                d->shiftModelRows(model, first, 1 + last - first);
                d->addModelData(model, first, last + 1);
              }
            });
    // TODO handle rows removed, moved
  }
}

// ----------------------------------------------------------------------------
void FusionModel::removeModel(QAbstractItemModel* model)
{
  QTE_D();

  if (d->models.contains(model))
  {
    this->beginResetModel();
    d->models.remove(model);
    d->removeModelData(model);
    this->endResetModel();
  }
}

// ----------------------------------------------------------------------------
void FusionModelPrivate::addModelData(QAbstractItemModel* model)
{
  this->addModelData(model, 0, model->rowCount());
}

// ----------------------------------------------------------------------------
void FusionModelPrivate::addModelData(
  QAbstractItemModel* model, int firstRow, int rowAfterLast)
{
  auto const oldCount = this->data.count();
  QHash<qint64, RowData> newData;
  QSet<int> modifiedRows;

  // Examine rows of model
  for (auto sourceRow = firstRow; sourceRow < rowAfterLast; ++sourceRow)
  {
    auto const& index = model->index(sourceRow, 0);
    auto const iid =
      model->data(index, core::LogicalIdentityRole).toLongLong();

    // Find our row for this item (if any)
    auto const localRow = this->items.value(iid, -1);
    if (localRow < 0)
    {
      // Item is new, and...
      if (auto* const newRow = qtGet(newData, iid))
      {
        // ...we are already adding a row for the new item
        newRow->rows.insert(model, sourceRow);
      }
      else
      {
        // ...this is the first time we've seen the item
        newData.insert(iid, {iid, {{model, sourceRow}}});
      }
    }
    else
    {
      // Update source rows of existing item
      auto& r = this->data[localRow];
      r.rows.insert(model, sourceRow);
      modifiedRows.insert(localRow);
    }
  }

  QTE_Q();

  if (!newData.isEmpty())
  {
    auto const newCount = oldCount + newData.count();
    q->beginInsertRows({}, oldCount, newCount - 1);

    this->data.reserve(newCount);
    for (auto i : newData | kvr::indirect)
    {
      this->items.insert(i.key(), this->data.count());
      this->data.append(std::move(i.value()));
    }

    q->endInsertRows();
  }

  if (!modifiedRows.isEmpty())
  {
    q->emitDataChanged({}, modifiedRows.toList());
  }
}

// ----------------------------------------------------------------------------
void FusionModelPrivate::shiftModelRows(
  QAbstractItemModel* model, int firstRow, int rowOffset)
{
  for (auto& rd : this->data)
  {
    for (auto&& i : rd.rows | kvr::indirect)
    {
      if (i.key() == model && i.value() >= firstRow)
      {
        i.value() += rowOffset;
      }
    }
  }
}

// ----------------------------------------------------------------------------
void FusionModelPrivate::removeModelData(QAbstractItemModel* model)
{
  auto i = this->data.begin();
  auto rows = this->data.size();
  auto row = decltype(rows){0};

  while (row < rows)
  {
    // Remove model rows
    i->rows.remove(model);

    // Check if item still has any associated source rows
    if (!i->rows.isEmpty())
    {
      // It does; move on to the next
      ++i;
      ++row;
    }
    else
    {
      // Item has no more source rows; remove it
      this->items.remove(i->id);

      if (row == --rows)
      {
        this->data.erase(i);
        break; // This was the last item, so we must be done
      }
      else
      {
        // Replace with last item and update map
        *i = this->data.takeLast();
        this->items[i->id] = row;
      }
    }
  }
}

// ----------------------------------------------------------------------------
template <typename Fusor>
QVariant FusionModelPrivate::fuseData(RowData const& rowData, int role)
{
  using data_t = typename Fusor::data_t;

  auto first = true;
  auto result = data_t{};

  for (auto const& sourceRow : rowData.rows | kvr::indirect)
  {
    auto* const model = sourceRow.key();
    auto const& index = model->index(sourceRow.value(), 0);
    auto const data = model->data(index, role).value<data_t>();

    if (first)
    {
      result = data;
      first = false;
    }
    else
    {
      result = Fusor::fuse(result, data);
    }
  }

  return QVariant::fromValue(result);
}

// ----------------------------------------------------------------------------
bool FusionModelPrivate::setData(
  RowData const& rowData, QVariant const& value, int role)
{
  auto result = false;

  for (auto const& sourceRow : rowData.rows | kvr::indirect)
  {
    auto* const model = sourceRow.key();
    auto const& index = model->index(sourceRow.value(), 0);

    result = model->setData(index, value, role) || result;
  }

  return result;
}

} // namespace gui

} // namespace sealtk
