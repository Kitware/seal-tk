/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/NotesDelegate.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <vital/range/iota.h>

#include <QAbstractProxyModel>
#include <QCollator>
#include <QComboBox>
#include <QSet>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace sealtk
{

namespace noaa
{

namespace gui
{

// ----------------------------------------------------------------------------
NotesDelegate::NotesDelegate(QObject* parent)
  : QStyledItemDelegate{parent}
{
}

// ----------------------------------------------------------------------------
NotesDelegate::~NotesDelegate()
{
}

// ----------------------------------------------------------------------------
QWidget* NotesDelegate::createEditor(
  QWidget* parent, QStyleOptionViewItem const& option,
  QModelIndex const& index) const
{
  QComboBox* box = new QComboBox{parent};
  box->setEditable(true);
  box->setFocusPolicy(Qt::StrongFocus);
  box->setFrame(false);

  auto allNotes = QSet<QString>{};
  if (auto* const model = index.model())
  {
    auto const& parent = model->parent(index);
    for (auto r : kvr::iota(model->rowCount(parent)))
    {
      auto const& i = model->index(r, 0, parent);
      auto const& notes = model->data(i, core::NotesRole).toStringList();
      allNotes.insert(notes.join(QStringLiteral("; ")));
    }
  }

  auto collator = QCollator{};
  auto sortedNotes = allNotes.toList();
  std::sort(sortedNotes.begin(), sortedNotes.end(), collator);
  for (auto const& n : sortedNotes)
  {
    box->addItem(n);
  }

  return box;
}

// ----------------------------------------------------------------------------
void NotesDelegate::setEditorData(
  QWidget* editor, QModelIndex const& index) const
{
  if (auto* const model = index.model())
  {
    auto const& notes = model->data(index, core::NotesRole).toStringList();

    auto* const box = qobject_cast<QComboBox*>(editor);
    box->setCurrentText(notes.join(QStringLiteral("; ")));
  }
}

// ----------------------------------------------------------------------------
void NotesDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, QModelIndex const& index) const
{
  auto* const box = qobject_cast<QComboBox*>(editor);

  auto const newNotes = QStringList{box->currentText()};
  model->setData(index, newNotes, core::NotesRole);
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
