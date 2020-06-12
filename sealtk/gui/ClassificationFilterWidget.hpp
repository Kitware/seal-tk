/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_ClassificationFilterWidget_hpp
#define sealtk_gui_ClassificationFilterWidget_hpp

#include <sealtk/gui/Export.h>

#include <QWidget>

#include <qtGlobal.h>

namespace sealtk
{

namespace core
{

class ClassificationFilterModel;

} // namespace core

namespace gui
{

class ClassificationFilterWidgetPrivate;

class SEALTK_GUI_EXPORT ClassificationFilterWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ClassificationFilterWidget(QWidget* parent = nullptr);
  ~ClassificationFilterWidget();

  QList<QString> types() const;

  double value(QString const& type) const;

signals:
  void valueChanged(QString const& type, double value);

public slots:
  void addType(QString const& type);
  void setValue(QString const& type, double value);

protected:
  QTE_DECLARE_PRIVATE_RPTR(ClassificationFilterWidget)

private:
  QTE_DECLARE_PRIVATE(ClassificationFilterWidget)
};

} // namespace gui

} // namespace sealtk

#endif
