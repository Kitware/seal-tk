/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/gui/ClassificationFilterWidget.hpp>
#include <sealtk/gui/FilterWidget.hpp>

#include <sealtk/core/ClassificationFilterModel.hpp>
#include <sealtk/core/DataModelTypes.hpp>

#include <QVBoxLayout>

namespace sealtk
{

namespace gui
{

// ============================================================================
class ClassificationFilterWidgetPrivate
{
public:
  QHash<QString, FilterWidget*> widgets;
  QVBoxLayout layout;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(ClassificationFilterWidget)

// ----------------------------------------------------------------------------
ClassificationFilterWidget::ClassificationFilterWidget(QWidget* parent)
  : QWidget{parent}, d_ptr{new ClassificationFilterWidgetPrivate}
{
  QTE_D();

  this->setLayout(&d->layout);
  d->layout.setSpacing(0);
  d->layout.addStretch();
}

// ----------------------------------------------------------------------------
ClassificationFilterWidget::~ClassificationFilterWidget()
{
}

// ----------------------------------------------------------------------------
void ClassificationFilterWidget::addType(QString const& typeName)
{
  QTE_D();

  if (d->widgets.contains(typeName))
  {
    return;
  }

  auto widget = new FilterWidget{this};
  widget->setLabel(typeName);
  widget->setCheckable(true);
  widget->setRange(0.0, 1.0);
  widget->setFilter(core::ClassificationScoreRole);

  connect(widget, &FilterWidget::valueChanged, this,
          [typeName, this, d](double value){
            emit this->valueChanged(typeName, value);
          });

  emit this->valueChanged(typeName, widget->value());

  // TODO make label sizes consistent

  d->layout.insertWidget(d->layout.count() - 1, widget);
  d->widgets.insert(typeName, widget);
}

// ----------------------------------------------------------------------------
QList<QString> ClassificationFilterWidget::types() const
{
  QTE_D();
  return d->widgets.keys();
}

// ----------------------------------------------------------------------------
double ClassificationFilterWidget::value(QString const& typeName) const
{
  QTE_D();

  auto* const widget = d->widgets.value(typeName);
  return (widget ? widget->value() : qQNaN());
}

// ----------------------------------------------------------------------------
void ClassificationFilterWidget::setValue(
  QString const& typeName, double value)
{
  QTE_D();

  if (auto* const widget = d->widgets.value(typeName))
  {
    widget->setValue(value);
  }
}

} // namespace gui

} // namespace sealtk
