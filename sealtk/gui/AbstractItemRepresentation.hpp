/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_AbstractItemRepresentation_hpp
#define sealtk_gui_AbstractItemRepresentation_hpp

#include <sealtk/gui/Export.h>

#include <sealtk/core/AbstractProxyModel.hpp>

#include <qtGlobal.h>

namespace sealtk
{

namespace gui
{

QTE_BEGIN_META_NAMESPACE(SEALTK_GUI_EXPORT, representation_enums)

/// Item visibility mode.
///
/// This enumeration specifies how "hidden" items will be manipulated by the
/// representation.
enum ItemVisibilityMode
{
  /// "Hidden" items are not shown.
  OmitHidden,
  /// "Hidden" items are shown in an alternate ("grayed out") color.
  ShadowHidden,
};

QTE_ENUM_NS(ItemVisibilityMode)

QTE_END_META_NAMESPACE()

class AbstractItemRepresentationPrivate;

/// Abstract implementation of an item representation.
///
/// This class provides a base class for implementing item representations from
/// a generic data model. It provides some common functionality for translating
/// low-level data types into data suitable for presentation, as well as common
/// handling for handling and manipulating item visibility states.
class SEALTK_GUI_EXPORT AbstractItemRepresentation
  : public sealtk::core::AbstractProxyModel
{
  Q_OBJECT

  /// This property specifies the visibility mode for "hidden" items.
  ///
  /// This property controls the presentation of items which are "hidden"
  /// (that is, the item data for VisibilityRole is \c false).
  ///
  /// \sa ItemVisibilityMode, itemVisibilityMode(), setItemVisibilityMode()
  Q_PROPERTY(ItemVisibilityMode itemVisibilityMode
             READ itemVisibilityMode
             WRITE setItemVisibilityMode)

public:
  explicit AbstractItemRepresentation(QObject* parent = nullptr);
  ~AbstractItemRepresentation() override;

  /// Get the visibility mode for "hidden" items.
  /// \sa ItemVisibilityMode, itemVisibilityMode, setItemVisibilityMode()
  virtual ItemVisibilityMode itemVisibilityMode() const;

  /// Get data role mapping for column.
  ///
  /// This returns the data role mapping (i.e. the logical data role) for the
  /// specified column, or -1 if no mapping has been set.
  ///
  /// This is not used internally, but may be used by e.g. views in order to
  /// take appropriate actions when an item index is activated or edited. As
  /// such, subclasses should override this method if necessary to ensure that
  /// a suitable logical role is always returned whenever possible.
  virtual int roleForColumn(int column) const;

  // Reimplemented from QAbstractItemModel
  int columnCount(QModelIndex const& parent = {}) const override;
  QVariant data(QModelIndex const& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  // Reimplemented from QSortFilterProxyModel
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

public slots:
  /// Set the visibility mode for "hidden" items.
  /// \sa ItemVisibilityMode, itemVisibilityMode, itemVisibilityMode()
  virtual void setItemVisibilityMode(ItemVisibilityMode);

protected slots:
  /// Update sorting when data changes.
  ///
  /// This slot checks if the model has not been sorted, and if so, sorts it.
  /// This is done to work around a bug in QSortFilterProxyModel where items
  /// are not sorted after a data change if all items were initially hidden.
  ///
  /// This slot is called automatically when the proxy model's data changes
  /// (note: \em not the source model's data). Users should not normally need
  /// to call this slot themselves.
  void updateSort();

protected:
  QTE_DECLARE_PRIVATE_RPTR(AbstractItemRepresentation)

  /// Set data role mappings for columns.
  ///
  /// This sets a data role mapping for all columns, which also sets the
  /// representation's column count. Calls to #data(QModelIndex const&, int)
  /// and #lessThan(QModelIndex const&, QModelIndex const&) will return or use
  /// the data for the specified logical data role.
  virtual void setColumnRoles(std::initializer_list<int> const& roles);

  /// Return representation for specified index and data role.
  ///
  /// This method returns presentation-ready data for the specified
  /// presentation role and data role.
  ///
  /// Supported presentation roles are:
  /// \li Qt::DisplayRole
  /// \li Qt::DecorationRole
  /// \li Qt::TextAlignmentRole
  /// \li Qt::ToolTipRole
  virtual QVariant data(
    QModelIndex const& sourceIndex, int presentationRole, int dataRole) const;

  /// Obtain data from source model.
  ///
  /// This is a convenience function to obtain data from the source model,
  /// using an index from this model.
  inline QVariant sourceData(QModelIndex const& index, int role) const
  {
    auto* const sm = this->sourceModel();
    auto const& si = this->mapToSource(index);
    Q_ASSERT(sm);

    return sm->data(si, role);
  }

  using AbstractProxyModel::lessThan;

  /// Compare data for two indices.
  ///
  /// This method performs a comparison of the \p role data of two proxy
  /// indices. If \p role is not a supported data role, the result is \c false.
  ///
  /// \return \c true if the left index's data is less than the right index's
  ///         data; otherwise \c false.
  virtual bool lessThan(
    QModelIndex const& left, QModelIndex const& right, int role) const;

  // Reimplemented from QSortFilterProxyModel
  bool lessThan(
    QModelIndex const& left, QModelIndex const& right) const override;

  // Reimplemented from QSortFilterProxyModel
  bool filterAcceptsRow(
    int sourceRow, QModelIndex const& sourceParent) const override;
  bool filterAcceptsColumn(
    int sourceColumn, QModelIndex const& sourceParent) const override;

private:
  QTE_DECLARE_PRIVATE(AbstractItemRepresentation)
};

} // namespace gui

} // namespace sealtk

#endif
