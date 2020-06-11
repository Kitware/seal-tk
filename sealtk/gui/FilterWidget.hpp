/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_FilterWidget_hpp
#define sealtk_gui_FilterWidget_hpp

#include <sealtk/gui/Export.h>

#include <QWidget>

#include <qtGlobal.h>

namespace sealtk
{

namespace gui
{

class FilterWidgetPrivate;

class SEALTK_GUI_EXPORT FilterWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
  Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)

public:
  explicit FilterWidget(QWidget* parent = nullptr);
  ~FilterWidget();

  enum Mode { LowPass, HighPass };
  void setFilter(int role, Mode mode = LowPass);

  double value() const;
  bool isCheckable() const;

signals:
  void valueChanged(double value);
  void filterChanged(int role, QVariant low, QVariant high);
  void filterMinimumChanged(int role, QVariant low);
  void filterMaximumChanged(int role, QVariant high);

public slots:
  void setMinimum(double minimum);
  void setMaximum(double maximum);
  void setRange(double minimum, double maximum);
  void setValue(double value);
  void setCheckable(bool checkable);

  void setLabel(QString const& text);

protected:
  QTE_DECLARE_PRIVATE_RPTR(FilterWidget)

private:
  QTE_DECLARE_PRIVATE(FilterWidget)
};

} // namespace gui

} // namespace sealtk

#endif
