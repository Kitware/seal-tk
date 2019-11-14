/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/TrackTypeDelegate.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/types/detected_object_type.h>

#include <vital/range/iota.h>

#include <qtStlUtil.h>

#include <QComboBox>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace noaa
{

namespace gui
{

// ----------------------------------------------------------------------------
TrackTypeDelegate::TrackTypeDelegate(QObject* parent)
  : QStyledItemDelegate{parent}
{
}

// ----------------------------------------------------------------------------
TrackTypeDelegate::~TrackTypeDelegate()
{
}

// ----------------------------------------------------------------------------
QWidget* TrackTypeDelegate::createEditor(
  QWidget* parent, QStyleOptionViewItem const& option,
  QModelIndex const& index) const
{
  QComboBox* box = new QComboBox{parent};
  box->setEditable(true);
  box->setFocusPolicy(Qt::StrongFocus);
  box->setFrame(false);

  for (auto const& type : kv::detected_object_type::all_class_names())
  {
    box->addItem(qtString(type));
  }

  return box;
}

// ----------------------------------------------------------------------------
void TrackTypeDelegate::setEditorData(
  QWidget* editor, QModelIndex const& index) const
{
  if (auto* const model = index.model())
  {
    auto const& data = model->data(index, core::ClassificationTypeRole);
    auto const& type = data.toString();

    auto* const box = qobject_cast<QComboBox*>(editor);

    int i = -1;
    for (auto const j : kvr::iota(box->count()))
    {
      if (box->itemText(j) == type)
      {
        i = j;
        break;
      }
    }

    box->setCurrentIndex(i);
  }
}

// ----------------------------------------------------------------------------
void TrackTypeDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, QModelIndex const& index) const
{
  auto* const box = qobject_cast<QComboBox*>(editor);
  auto const& newType = box->currentText();

  auto newClassification = QVariantHash{{newType, 1.0}};
  model->setData(index, newClassification, core::ClassificationRole);
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
