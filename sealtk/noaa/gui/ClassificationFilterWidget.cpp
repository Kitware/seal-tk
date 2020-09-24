/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/ClassificationFilterWidget.hpp>

#include <vital/types/class_map.h>

#include <qtStlUtil.h>

namespace kv = kwiver::vital;

using super = sealtk::gui::ClassificationFilterWidget;

namespace sealtk
{

namespace noaa
{

namespace gui
{

// ============================================================================
class ClassificationFilterWidgetPrivate : public kwiver::vital::context
{
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(ClassificationFilterWidget)

// ----------------------------------------------------------------------------
ClassificationFilterWidget::ClassificationFilterWidget(QWidget* parent)
  : super{parent}, d_ptr{new ClassificationFilterWidgetPrivate}
{
  QTE_D();

  kv::class_map::class_name_added.connect(
    d,
    [this](std::string const& stdTypeName){
      auto qtTypeName = qtString(stdTypeName);
      QMetaObject::invokeMethod(
        this, [qtTypeName, this]{ this->addType(qtTypeName); });
    });

  for (auto const& typeName : kv::class_map::all_class_names())
  {
    this->addType(qtString(typeName));
  }
}

// ----------------------------------------------------------------------------
ClassificationFilterWidget::~ClassificationFilterWidget()
{
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
