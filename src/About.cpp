/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "About.hpp"
#include "ui_About.h"

#include "Config.h"

namespace sealtk
{

//=============================================================================
class AboutPrivate
{
public:
  Ui::About ui;
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(About)

//-----------------------------------------------------------------------------
About::About(QWidget* parent)
  : QDialog(parent),
    d_ptr{new AboutPrivate}
{
  QTE_D();
  d->ui.setupUi(this);

  d->ui.labelCopyright->setText(QStringLiteral(
    SEALTK_TITLE " " SEALTK_VERSION "\n\n"
    SEALTK_DESCRIPTION "\n\nCopyright \u00A9 2018 Kitware, Inc."));
}

//-----------------------------------------------------------------------------
About::~About()
{
}

}
