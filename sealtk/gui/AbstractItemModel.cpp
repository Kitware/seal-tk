/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/AbstractItemModel.hpp>

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

// ----------------------------------------------------------------------------
int AbstractItemModel::columnCount(QModelIndex const& parent) const
{
  Q_UNUSED(parent);

  // NOTE:
  //
  // If the model is filtered prior to being fed to a representation (which is
  // often the case), QSortFilterProxyModel needs to reserve a QVector<int> of
  // this size, or the final representation will end up truncated to this
  // number of columns. Therefore, we need to return a value that is "not too
  // large", but likely to be at least as large as the maximum number of
  // columns that any representation will have.
  return 64;
}

// ----------------------------------------------------------------------------
QModelIndex AbstractItemModel::index(
  int row, int column, QModelIndex const& parent) const
{
  return (parent.isValid() ? QModelIndex{} : this->createIndex(row, column));
}

// ----------------------------------------------------------------------------
QModelIndex AbstractItemModel::parent(QModelIndex const& child) const
{
  Q_UNUSED(child);
  return {};
}

// ----------------------------------------------------------------------------
QVariant AbstractItemModel::data(QModelIndex const& index, int role) const
{
  if (role == core::VisibilityRole)
  {
    // VisibilityRole is meant to be defined by filters based on whatever
    // filtering criteria they are using. Models notionally should not provide
    // this role directly. However, this will break if a model is used without
    // a filter, so by default, map VisibilityRole to UserVisibilityRole.
    return this->data(index, core::UserVisibilityRole);
  }

  return {};
}

} // namespace gui

} // namespace sealtk
