/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_core_AbstractItemModel_hpp
#define sealtk_core_AbstractItemModel_hpp

#include <sealtk/core/Export.h>

#include <QAbstractItemModel>

namespace sealtk
{

namespace core
{

/// Abstract implementation of an item model.
///
/// This class provides a base class for implementing generic item data models.
/// It provides common implementations of functions that are not likely to
/// differ across different models, such as index creation and "column count".
/// (The latter is useful because generic data models use role rather than
/// column for field discrimination. Accordingly, the "column count" is
/// determined by the representation which ultimately consumes the model,
/// rather than the model itself.)
///
/// The default implementations of #index and #parent are suitable for "flat"
/// models. Models that provide tree-structured data should override these.
class SEALTK_CORE_EXPORT AbstractItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  using QAbstractItemModel::QAbstractItemModel;

  // Reimplemented from QAbstractItemModel
  int columnCount(QModelIndex const& parent = {}) const override;
  QModelIndex index(
    int row, int column, QModelIndex const& parent = {}) const override;
  QModelIndex parent(QModelIndex const& child) const override;

  QVariant data(QModelIndex const& index, int role) const override;

protected:
  static constexpr auto IndexIsValid = CheckIndexOption::IndexIsValid;
  static constexpr auto ParentIsInvalid = CheckIndexOption::ParentIsInvalid;
};

} // namespace core

} // namespace sealtk

#endif
