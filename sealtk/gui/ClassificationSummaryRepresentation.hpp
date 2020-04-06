/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_ClassificationSummaryRepresentation_hpp
#define sealtk_gui_ClassificationSummaryRepresentation_hpp

#include <sealtk/gui/Export.h>

#include <qtGlobal.h>

#include <QAbstractItemModel>

namespace sealtk
{

namespace gui
{

class ClassificationSummaryRepresentationPrivate;

/// Representation which summarizes classifications of all items.
///
/// This class provides an item model representation which collects and
/// summarizes item classifications.
class SEALTK_GUI_EXPORT ClassificationSummaryRepresentation
  : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit ClassificationSummaryRepresentation(QObject* parent = nullptr);
  ~ClassificationSummaryRepresentation() override;

  // Reimplemented from QAbstractItemModel
  int rowCount(QModelIndex const& parent = {}) const override;
  int columnCount(QModelIndex const& parent = {}) const override;

  QModelIndex parent(QModelIndex const& child) const override;
  QModelIndex index(int row, int column,
                    QModelIndex const& parent = {}) const override;

  QVariant data(QModelIndex const& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  QAbstractItemModel* sourceModel() const;

public slots:
  void setSourceModel(QAbstractItemModel*);

protected:
  QTE_DECLARE_PRIVATE_RPTR(ClassificationSummaryRepresentation)

private:
  QTE_DECLARE_PRIVATE(ClassificationSummaryRepresentation)
};

} // namespace gui

} // namespace sealtk

#endif
