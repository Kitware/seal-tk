/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/TrackScoreDelegate.hpp>

#include <sealtk/core/DataModelTypes.hpp>

#include <QDoubleSpinBox>

namespace sealtk
{

namespace noaa
{

namespace gui
{

// ----------------------------------------------------------------------------
TrackScoreDelegate::TrackScoreDelegate(QObject* parent)
  : qtDoubleSpinBoxDelegate{parent}
{
  this->setRange(0.0, 1.0);
  this->setPrecision(5);
}

// ----------------------------------------------------------------------------
TrackScoreDelegate::~TrackScoreDelegate()
{
}

// ----------------------------------------------------------------------------
void TrackScoreDelegate::setEditorData(
  QWidget* editor, QModelIndex const& index) const
{
  if (auto* const model = index.model())
  {
    auto const& data = model->data(index, core::ClassificationScoreRole);
    auto* const spin = qobject_cast<QDoubleSpinBox*>(editor);
    spin->setValue(data.toDouble());
  }
}

// ----------------------------------------------------------------------------
void TrackScoreDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, QModelIndex const& index) const
{
  QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(editor);
  spin->interpretText();

  auto const& typeData = model->data(index, core::ClassificationTypeRole);
  auto const& newScore = spin->value();

  auto newClassification = QVariantHash{{typeData.toString(), newScore}};
  model->setData(index, newClassification, core::ClassificationRole);
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
