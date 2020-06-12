/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_noaa_gui_ClassificationFilterWidget_hpp
#define sealtk_noaa_gui_ClassificationFilterWidget_hpp

#include <sealtk/noaa/gui/Export.h>

#include <sealtk/gui/ClassificationFilterWidget.hpp>

namespace sealtk
{

namespace noaa
{

namespace gui
{

class ClassificationFilterWidgetPrivate;

class SEALTK_NOAA_GUI_EXPORT ClassificationFilterWidget
  : public sealtk::gui::ClassificationFilterWidget
{
  Q_OBJECT

public:
  explicit ClassificationFilterWidget(QWidget* parent = nullptr);
  ~ClassificationFilterWidget();

protected:
  QTE_DECLARE_PRIVATE_RPTR(ClassificationFilterWidget)

private:
  QTE_DECLARE_PRIVATE(ClassificationFilterWidget)
};

} // namespace gui

} // namespace noaa

} // namespace sealtk

#endif
