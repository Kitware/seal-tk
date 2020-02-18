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

  connect(d->ui.slider, &qtDoubleSlider::valueChanged,
          [this, d](double value){
            emit this->valueChanged(value);
            if (d->role >= 0)
            {
              if (d->mode == HighPass)
              {
                emit this->filterMaximumChanged(d->role, value);
                emit this->filterChanged(d->role, {}, value);
              }
              else
              {
                emit this->filterMinimumChanged(d->role, value);
                emit this->filterChanged(d->role, value, {});
              }
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
void FilterWidget::setValue(double value)
{
  QTE_D();
  d->ui.slider->setValue(value);
}

// ----------------------------------------------------------------------------
void FilterWidget::setLabel(QString const& text)
{
  QTE_D();
  d->ui.label->setText(text);
}

} // namespace gui

} // namespace sealtk
