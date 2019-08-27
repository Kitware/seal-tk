/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_Enums_hpp
#define sealtk_gui_Enums_hpp

#include <sealtk/gui/Export.h>

#include <QObject>

namespace sealtk
{

namespace gui
{

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
Q_NAMESPACE_EXPORT(SEALTK_GUI_EXPORT)
#else
Q_NAMESPACE
# ifndef Q_OS_WINDOWS
  // Ugh...
  extern SEALTK_GUI_EXPORT const QMetaObject staticMetaObject;
# endif
#endif

/// Item visibility mode.
///
/// This enumeration specifies how "hidden" items will be manipulated by the
/// representation.
enum ItemVisibilityMode
{
  /// "Hidden" items are not shown.
  OmitHidden,
  /// "Hidden" items are shown in an alternate ("grayed out") color.
  ShadowHidden,
};

Q_ENUM_NS(ItemVisibilityMode)

enum class ContrastMode
{
  Manual,
  Percentile,
};

Q_ENUM_NS(ContrastMode)

} // namespace gui

} // namespace sealtk

#endif
