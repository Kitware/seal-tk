/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#include "Panel.hpp"

namespace sealtk
{

//=============================================================================
class PanelPrivate
{
};

//-----------------------------------------------------------------------------
QTE_IMPLEMENT_D_FUNC(Panel)

//-----------------------------------------------------------------------------
Panel::Panel(QWidget* parent)
  : QMainWindow(parent),
    d_ptr{new PanelPrivate}
{
  auto flags = this->windowFlags();
  flags &= ~Qt::Window;
  this->setWindowFlags(flags);
}

//-----------------------------------------------------------------------------
Panel::~Panel()
{
}

//-----------------------------------------------------------------------------
void Panel::init(Window* window)
{
}

}
