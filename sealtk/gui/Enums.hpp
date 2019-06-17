/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifndef sealtk_gui_Enums_hpp
#define sealtk_gui_Enums_hpp

#include <QObject>

namespace sealtk
{

namespace gui
{

Q_NAMESPACE

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
