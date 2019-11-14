/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_gui_TrackTypeDelegate_hpp
#define sealtk_noaa_gui_TrackTypeDelegate_hpp

#include <sealtk/noaa/gui/Export.h>

#include <QStyledItemDelegate>

namespace sealtk
{

namespace noaa
{

namespace gui
{

class SEALTK_NOAA_GUI_EXPORT TrackTypeDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit TrackTypeDelegate(QObject* parent = nullptr);
  ~TrackTypeDelegate() override;

  QWidget* createEditor(
    QWidget* parent, QStyleOptionViewItem const& option,
    QModelIndex const& index) const override;
  void setEditorData(
    QWidget* editor, QModelIndex const& index) const override;
  void setModelData(
    QWidget* editor, QAbstractItemModel* model,
    QModelIndex const& index) const override;
};

} // namespace gui

} // namespace noaa

} // namespace sealtk

#endif
