/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/FilterWidget.hpp>
#include "ui_FilterWidget.h"

#include <qtScopedValueChange.h>

#include <QVariant>

namespace sealtk
{

namespace gui
{

// ============================================================================
class FilterWidgetPrivate
{
public:
  void emitValue(FilterWidget* q, double value);

  Ui::FilterWidget ui;

  FilterWidget::Mode mode = FilterWidget::LowPass;
  int role = -1;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(FilterWidget)

// ----------------------------------------------------------------------------
FilterWidget::FilterWidget(QWidget* parent)
  : QWidget{parent}, d_ptr{new FilterWidgetPrivate}
{
  QTE_D();
  d->ui.setupUi(this);
  d->ui.checkBox->hide();

  connect(d->ui.checkBox, &QAbstractButton::toggled, this,
          [this, d](bool checked){
            if (checked)
            {
              d->emitValue(this, d->ui.slider->value());
            }
            else
            {
              emit this->valueChanged(this->value());
              emit this->filterMinimumChanged(d->role, +qInf());
              emit this->filterMaximumChanged(d->role, -qInf());
              emit this->filterChanged(d->role, +qInf(), -qInf());
            }
          });
  connect(d->ui.slider, &qtDoubleSlider::valueChanged, this,
          [this, d](double value){
            if (d->ui.checkBox->isChecked())
            {
              d->emitValue(this, value);
            }
          });
}

// ----------------------------------------------------------------------------
FilterWidget::~FilterWidget()
{
}

// ----------------------------------------------------------------------------
void FilterWidget::setFilter(int role, Mode mode)
{
  QTE_D();
  d->role = role;
  d->mode = mode;
}

// ----------------------------------------------------------------------------
void FilterWidget::setMinimum(double minimum)
{
  QTE_D();

  with_expr(qtScopedBlockSignals{d->ui.slider})
  {
    d->ui.slider->setMinimum(minimum);
    d->ui.spinBox->setMinimum(minimum);
  }
}

// ----------------------------------------------------------------------------
void FilterWidget::setMaximum(double maximum)
{
  QTE_D();

  with_expr(qtScopedBlockSignals{d->ui.slider})
  {
    d->ui.slider->setMaximum(maximum);
    d->ui.spinBox->setMaximum(maximum);
  }
}

// ----------------------------------------------------------------------------
void FilterWidget::setRange(double minimum, double maximum)
{
  QTE_D();

  with_expr(qtScopedBlockSignals{d->ui.slider})
  {
    d->ui.slider->setRange(minimum, maximum);
    d->ui.spinBox->setRange(minimum, maximum);
  }
}

// ----------------------------------------------------------------------------
double FilterWidget::value() const
{
  QTE_D();

  if (!d->ui.checkBox->isChecked())
  {
    if (d->mode == FilterWidget::HighPass)
    {
      return -qInf();
    }
    else
    {
      return +qInf();
    }
  }
  else
  {
    return d->ui.slider->value();
  }
}

// ----------------------------------------------------------------------------
void FilterWidget::setValue(double value)
{
  QTE_D();
  d->ui.slider->setValue(value);
}

// ----------------------------------------------------------------------------
bool FilterWidget::isCheckable() const
{
  QTE_D();
  return d->ui.checkBox->isVisible();
}

// ----------------------------------------------------------------------------
void FilterWidget::setCheckable(bool checkable)
{
  QTE_D();

  d->ui.label->setVisible(!checkable);
  d->ui.checkBox->setVisible(checkable);
}

// ----------------------------------------------------------------------------
void FilterWidget::setLabel(QString const& text)
{
  QTE_D();
  d->ui.label->setText(text);
  d->ui.checkBox->setText(text);
}

// ----------------------------------------------------------------------------
void FilterWidgetPrivate::emitValue(FilterWidget* q, double value)
{
  emit q->valueChanged(value);
  if (this->role >= 0)
  {
    if (this->mode == FilterWidget::HighPass)
    {
      emit q->filterMaximumChanged(this->role, value);
      emit q->filterChanged(this->role, {}, value);
    }
    else
    {
      emit q->filterMinimumChanged(this->role, value);
      emit q->filterChanged(this->role, value, {});
    }
  }
}

} // namespace gui

} // namespace sealtk
