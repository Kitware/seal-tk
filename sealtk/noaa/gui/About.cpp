/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include <sealtk/noaa/gui/About.hpp>
#include "ui_About.h"

#include <sealtk/core/Version.h>

namespace sealtk
{

namespace noaa
{

namespace gui
{

// ============================================================================
class AboutPrivate
{
public:
  Ui::About ui;
};

// ----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(About)

// ----------------------------------------------------------------------------
About::About(QWidget* parent)
  : QDialog{parent},
    d_ptr{new AboutPrivate}
{
  QTE_D();
  d->ui.setupUi(this);

  d->ui.labelCopyright->setText(
    QStringLiteral(
      u"SEAL-TK " SEALTK_VERSION "\n\n"
      u"SEAL-TK, the Stereoscopic Examination of Aquatic Life Toolkit\n\n"
      u"Copyright \u00A9 2018-2019 Kitware, Inc."));
}

// ----------------------------------------------------------------------------
About::~About()
{
}

} // namespace gui

} // namespace noaa

} // namespace sealtk
